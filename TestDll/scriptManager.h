#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <regex>
#include <string>
#include "lua_interface.h"

class scriptManager {
private:

	// 条件表达式解析相关
	struct Condition {
		enum class Operator { LT, LE, GT, GE, EQ, NE };
		enum class LogicalOp { AND, OR };

		// 基础条件：变量比较
		Condition(const std::string& var, Operator op, int value)
			: type(Type::BASIC), var(var), op(op), value(value) {
		}

		// 复合条件
		Condition(LogicalOp logOp, std::shared_ptr<Condition> left, std::shared_ptr<Condition> right)
			: type(Type::COMPOUND), logOp(logOp), left(left), right(right) {
		}

		bool evaluate(scriptManager* mgr) const {
			if (type == Type::BASIC) {
				return evaluateBasic(mgr);
			}
			else {
				return evaluateCompound(mgr);
			}
		}
	private:
		enum class Type { BASIC, COMPOUND };
		Type type;

		// BASIC
		std::string var;
		Operator op;
		int value;

		// COMPOUND
		LogicalOp logOp;
		std::shared_ptr<Condition> left;
		std::shared_ptr<Condition> right;

		bool evaluateBasic(scriptManager* mgr) const {
			int actual = mgr->get_cached_value(var);

			switch (op) {
			case Operator::LT: return actual < value;
			case Operator::LE: return actual <= value;
			case Operator::GT: return actual > value;
			case Operator::GE: return actual >= value;
			case Operator::EQ: return actual == value;
			case Operator::NE: return actual != value;
			default: return false;
			}
		}

		bool evaluateCompound(scriptManager* mgr) const {
			bool l = left->evaluate(mgr);
			bool r = right->evaluate(mgr);

			return logOp == LogicalOp::AND ? (l && r) : (l || r);
		}
	};

	// 触发器定义
	struct Trigger {
		std::shared_ptr<Condition> condition;
		std::string action;
	};
	std::vector<Trigger> triggers;
	std::mutex triggerMutex;
private:
	// 数值型变量（需要缓存）
	std::shared_mutex varMutex;
	std::unordered_map<std::string, std::function<int()>> varMap;

	// 字符串型变量（不缓存，返回std::string）
	std::shared_mutex stringVarMutex;
	std::unordered_map<std::string, std::function<std::string()>> stringVarMap;

	// 缓存表（仅数值型变量）
	std::shared_mutex cacheMutex;
	std::unordered_map<std::string, std::pair<int, int64_t>> varCache; // <value, timestamp>

	lua_State* L;
	lua_interface* game;
	static const char* REGISTRY_KEY;
	const int CACHE_TTL = 100; // 100ms缓存有效期

	std::mutex luaMutex;  // Lua 状态操作互斥锁

	std::atomic<bool> running{ false };  // 使用原子操作
	std::thread monitorThread;         // 单独记录监控线程
	// 启动触发器监控线程
	//void start_trigger_monitor() {
	//	running = true;
	//	monitorThread = std::thread([this]() {
	//		while (running) {
	//			check_triggers();
	//			std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//		}
	//		});
	//}
	// 解析条件字符串
	std::shared_ptr<Condition> parse_condition(const std::string& condStr) {
		// 预处理：移除所有空格
		std::string str = std::regex_replace(condStr, std::regex("\\s+"), "");

		// 分割逻辑运算符（递归处理 AND/OR）
		size_t pos = str.find("and");
		if (pos != std::string::npos) {
			auto left = parse_condition(str.substr(0, pos));
			auto right = parse_condition(str.substr(pos + 3));
			return std::make_shared<Condition>(Condition::LogicalOp::AND, left, right);
		}

		pos = str.find("or");
		if (pos != std::string::npos) {
			auto left = parse_condition(str.substr(0, pos));
			auto right = parse_condition(str.substr(pos + 2));
			return std::make_shared<Condition>(Condition::LogicalOp::OR, left, right);
		}

		// 解析基础条件
		// 修改正则表达式：允许变量名包含中文字符
		std::regex pattern(R"(([^<>=!]+)(<=|>=|==|!=|<|>)(\d+))");
		std::smatch match;

		if (std::regex_match(str, match, pattern)) {
			std::string var = match[1];
			std::string opStr = match[2];
			int value = std::stoi(match[3]);

			Condition::Operator op = Condition::Operator::EQ;
			if (opStr == "<") op = Condition::Operator::LT;
			else if (opStr == "<=") op = Condition::Operator::LE;
			else if (opStr == ">") op = Condition::Operator::GT;
			else if (opStr == ">=") op = Condition::Operator::GE;
			else if (opStr == "==") op = Condition::Operator::EQ;
			else if (opStr == "!=") op = Condition::Operator::NE;

			return std::make_shared<Condition>(var, op, value);
		}

		throw std::runtime_error("无法解析条件: " + condStr);
	}

	// 定期检查所有触发器
	void check_triggers() {
		// 先获取 luaMutex，再获取 triggerMutex
		std::lock_guard<std::mutex> trigger_lock(triggerMutex);
		for (const auto& trigger : triggers) {
			if (trigger.condition->evaluate(this)) {
				execute_action(trigger.action);
			}
		}
	}


public:
	scriptManager(lua_interface* gameInterface)
		: game(gameInterface) {
		initLuaState();
		game->registerClasses(L);
		auto_register_game_vars();
		//start_trigger_monitor();
		
		luabridge::getGlobalNamespace(L)
			.beginClass<scriptManager>("scriptManager")
			.addFunction("add_trigger", &scriptManager::add_trigger)
			.addFunction("execute_action", &scriptManager::execute_action)
			.addFunction("reload_script", &scriptManager::reload_script)
			.addFunction("stop_all_triggers", &scriptManager::stop_all_triggers)
			.addFunction("request_stop",&scriptManager::request_stop)
			.addFunction("clear_triggers",&scriptManager::clear_triggers)
			.endClass();
		// 将game接口暴露到Lua全局
		luabridge::setGlobal(L, game, "game");

		// 暴露管理器自身到 Lua
		luabridge::setGlobal(L, this, "scriptMgr");

	}

	~scriptManager() {
		stop_all_triggers();  // 先停止线程
		{
			std::lock_guard<std::mutex> lock(triggerMutex);
			triggers.clear();
		}
		lua_close(L);  // 最后关闭 Lua 状态
	}

	// 条件解析方法
	void add_trigger(const std::string& condition, const std::string& action) {
		auto cond = parse_condition(condition);

		std::lock_guard<std::mutex> lock(triggerMutex);
		triggers.push_back({ cond, action });
	}

	void scriptManager::remove_trigger(const std::string& action) {
		std::lock_guard<std::mutex> lock(triggerMutex);
		triggers.erase(std::remove_if(triggers.begin(), triggers.end(),
			[&action](const Trigger& t) { return t.action == action; }),
			triggers.end());
	}

	// 定期检查所有触发器
	void clear_triggers() {
		//std::lock_guard<std::mutex> lock(triggerMutex);
		triggers.clear();
	}
	// 注册数值型变量（带缓存）
	void register_cache_var(const std::string& name, std::function<int()> getter) {
		std::unique_lock<std::shared_mutex> lock(varMutex);
		varMap[name] = getter;
		std::unique_lock<std::shared_mutex> cacheLock(cacheMutex);
		varCache[name] = { getter(), 0 };
	}

	// 注册字符串型变量（不缓存，使用std::string）
	void register_string_var(const std::string& name, std::function<std::string()> getter) {
		std::unique_lock<std::shared_mutex> lock(stringVarMutex);
		stringVarMap[name] = getter;
	}

	void auto_register_game_vars() {
		// 注册数值型变量
		register_cache_var("血量", [this]() { return game->getCurrentHP(); });
		register_cache_var("蓝量", [this]() { return game->getCurrentMP(); });
		register_cache_var("红药数量", [this]() { return game->getBagHpMedcine(); });
		register_cache_var("蓝药数量", [this]() { return game->getBagMpMedcine(); });
		register_cache_var("坐标X", [this]() { return game->getCurrentX(); });
		register_cache_var("坐标Y", [this]() { return game->getCurrentY(); });
		register_cache_var("剩余格子", [this]() { return game->getBagSpace(); });
		register_cache_var("背包格子", [this]() { return game->getBagSpaceMax(); });
		register_cache_var("背包负重", [this]() { return game->getBagWeightRemain(); });
		register_cache_var("背包总负重", [this]() { return game->getBagWeightMax(); });
		register_cache_var("金币", [this]() { return game->getGoldNumber(); });
		register_cache_var("元宝", [this]() { return game->getYbNumber(); });
		register_cache_var("绑定金币", [this]() { return game->getBindGoldNumber(); });
		register_cache_var("绑定元宝", [this]() { return game->getBindYbNumber(); });

		// 注册字符串型变量（使用std::string）
		register_string_var("当前地图", [this]() { return game->getCurrentMapName(); });
	}


 // 执行 Lua 动作（标签或函数）
	void execute_action(const std::string& action) {
		// 初始化当前线程的 Lua 状态
		if (!L) {
			// 执行 Lua 代码
			std::lock_guard<std::mutex> lock(luaMutex);
			lua_getglobal(L, action.c_str());
			if (lua_isfunction(L, -1)) {
				if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
					std::cerr << "执行错误: " << lua_tostring(L, -1) << std::endl;
					lua_pop(L, 1);
				}
			}
			else {
				lua_pop(L, 1);
				// 尝试直接跳转（需要预编译跳转逻辑）
				std::string code = "goto " + action;
				if (luaL_loadbuffer(L, code.c_str(), code.size(), "jumptag") ||
					lua_pcall(L, 0, 0, 0)) {
					std::cerr << "无效标签: " << action << std::endl;
					lua_pop(L, 1);
				}
			}
		}
	}

	//允许运行时重新加载脚本而不中断已有触发器：
	void reload_script(const std::string& filepath) {
		std::lock_guard<std::mutex> lock(luaMutex);
		stop_all_triggers();
		luaL_dofile(L, filepath.c_str());
		start(filepath);
	}

	static int lua_index_handler(lua_State* L) {
		lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_KEY);
		scriptManager* self = static_cast<scriptManager*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		const char* varName = lua_tostring(L, 2);

		// 1. 检查字符串变量（实时获取）
		{
			std::shared_lock<std::shared_mutex> lock(self->stringVarMutex);
			auto it = self->stringVarMap.find(varName);
			if (it != self->stringVarMap.end()) {
				std::string value = it->second(); // 调用getter获取最新值
				lua_pushstring(L, value.c_str());
				return 1;
			}
		}

		// 2. 检查数值变量（带缓存）
		{
			std::shared_lock<std::shared_mutex> lock(self->varMutex);
			auto it = self->varMap.find(varName);
			if (it != self->varMap.end()) {
				int value = self->get_cached_value(varName);
				lua_pushinteger(L, value);
				return 1;
			}
		}

		lua_pushnil(L);
		return 1;
	}

	int get_cached_value(const std::string& varName) {
		auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		// 第一次检查（共享读锁）
		{
			std::shared_lock<std::shared_mutex> read_lock(cacheMutex);
			auto it = varCache.find(varName);
			if (it != varCache.end() && (now - it->second.second) <= CACHE_TTL) {
				return it->second.first;
			}
		}

		// 获取独占写锁并更新缓存
		std::unique_lock<std::shared_mutex> write_lock(cacheMutex);
		auto& entry = varCache[varName];
		if ((now - entry.second) > CACHE_TTL) {
			entry.first = varMap[varName]();
			entry.second = now;
		}
		return entry.first;
	}

	void initLuaState() {
		L = luaL_newstate();
		luaL_openlibs(L);

		// 存储指针到注册表
		lua_pushlightuserdata(L, this);
		lua_setfield(L, LUA_REGISTRYINDEX, REGISTRY_KEY);

		// 设置全局表的元表
		lua_newtable(L); // 创建元表
		lua_pushcfunction(L, &scriptManager::lua_index_handler);
		lua_setfield(L, -2, "__index");
		lua_setmetatable(L, LUA_GLOBALSINDEX);
		// 替换 Lua 的 print 函数
		lua_pushcfunction(L, &lua_interface::lua_print);
		lua_setglobal(L, "print");
		// 在 initLuaState 中添加
		lua_pushcfunction(L, [](lua_State* L) {
			int ms = luaL_checkinteger(L, 1);
			std::this_thread::sleep_for(std::chrono::milliseconds(ms));
			//lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_KEY);
			//auto self= static_cast<scriptManager*>(lua_touserdata(L, lua_upvalueindex(1)));
			//self->check_triggers();//再sleep加入检查触发器
			return 0;
			});
		lua_setglobal(L, "sleep");
	}

	void start(const std::string& filepath) {  

		if (!tools::getInstance()->fileIsexist(filepath)) {
			std::cout << "[错误] 文件不存在: " << filepath << std::endl;
			return;
		}

		running = true;
		std::lock_guard<std::mutex> lock(luaMutex);

		if (luaL_loadfile(L, filepath.c_str()) != LUA_OK) {
			std::cerr << "[错误] 脚本加载失败: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
			return;
		}
		if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
			std::cerr << "[错误] 脚本执行错误: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}

	void request_stop() {
		std::thread([this]() {
			stop_all_triggers();
			}).detach();
	}
	void stop_all_triggers() {
		running = false;
		if (monitorThread.joinable()) {
			monitorThread.join();
		}
		// 清理其他线程（如果有）

	}


	private:   
		// 静态成员变量，使用 thread_local 修饰
	     static thread_local lua_State* threadLua;

		// 静态初始化函数
		void initializeLuaState() {
			threadLua = luaL_newstate();
			luaL_openlibs(threadLua);
			// 其他初始化操作
			cloneMainStateToThread();
		}


		// 从主状态复制全局环境到当前线程的 Lua 状态
		void cloneMainStateToThread() {
			std::lock_guard<std::mutex> lock(luaMutex);
			lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY_KEY);
			copyTable(L, threadLua); // 递归复制表内容
			lua_pop(L, 1); // 弹出主状态全局表
			lua_newtable(threadLua); // 创建元表
			lua_pushcfunction(threadLua, &scriptManager::lua_index_handler);
			lua_setfield(threadLua, -2, "__index");
			lua_setmetatable(threadLua, LUA_GLOBALSINDEX);
			// 替换 Lua 的 print 函数
			lua_pushcfunction(threadLua, &lua_interface::lua_print);
			lua_setglobal(threadLua, "print");
			// 在 initLuaState 中添加
			lua_pushcfunction(threadLua, [](lua_State* L) {
				int ms = luaL_checkinteger(threadLua, 1);
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				return 0;
				});
			lua_setglobal(threadLua, "sleep");

			game->registerClasses(threadLua);
			auto_register_game_vars();

			luabridge::getGlobalNamespace(threadLua)
				.beginClass<scriptManager>("scriptManager")
				.addFunction("add_trigger", &scriptManager::add_trigger)
				.addFunction("execute_action", &scriptManager::execute_action)
				.addFunction("reload_script", &scriptManager::reload_script)
				.addFunction("stop_all_triggers", &scriptManager::stop_all_triggers)
				.addFunction("request_stop", &scriptManager::request_stop)
				.addFunction("clear_triggers", &scriptManager::clear_triggers)
				.endClass();
			luabridge::setGlobal(threadLua, game, "game");
			luabridge::setGlobal(threadLua, this, "scriptMgr");

		}

		// 递归复制表（深拷贝）
		static void copyTable(lua_State* src, lua_State* dest) {
			lua_pushnil(src);
			while (lua_next(src, -2) != 0) {
				// 复制键
				lua_pushvalue(src, -2);
				copyValue(src, dest);

				// 复制值
				copyValue(src, dest);

				// 设置到目标表
				lua_rawset(dest, -3);

				lua_pop(src, 1);
			}
		}

		// 复制 Lua 值（支持基础类型和表）
		static void copyValue(lua_State* src, lua_State* dest) {
			int type = lua_type(src, -1);
			switch (type) {
			case LUA_TNIL:
				lua_pushnil(dest);
				break;
			case LUA_TBOOLEAN:
				lua_pushboolean(dest, lua_toboolean(src, -1));
				break;
			case LUA_TNUMBER:
				lua_pushnumber(dest, lua_tonumber(src, -1));
				break;
			case LUA_TSTRING: {
				size_t len;
				const char* str = lua_tolstring(src, -1, &len);
				lua_pushlstring(dest, str, len);
				break;
			}
			case LUA_TTABLE:
				lua_newtable(dest);
				copyTable(src, dest); // 递归复制子表
				break;
			default:
				// 其他类型（如函数、userdata）需要特殊处理
				lua_pushnil(dest);
				break;
			}
		}
};
thread_local lua_State* scriptManager::threadLua = nullptr;
const char* scriptManager::REGISTRY_KEY = "SCRIPT_MANAGER_PTR";