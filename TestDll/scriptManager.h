#pragma once
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <regex>
#include <string>
#include "lua_interface.h"

class scriptManager {
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

	lua_interface* game;
	static const char* REGISTRY_KEY;
	const int CACHE_TTL = 100; // 100ms缓存有效期

	std::mutex luaMutex;  // Lua 状态操作互斥锁
	std::atomic<bool> running{ false };  // 使用原子操作


public:
	lua_State* L;
	scriptManager(lua_interface* p):game(p){
		initLuaState();
		if (nullptr == game) { std::cout << "初始化ScriptManager失败！" << std::endl; return; }
		game->registerClasses(L);
		auto_register_game_vars();

		luabridge::getGlobalNamespace(L)
			.beginClass<scriptManager>("scriptManager")
			.endClass();
		// 将game接口暴露到Lua全局
		luabridge::setGlobal(L, game, "game");

		// 暴露管理器自身到 Lua
		luabridge::setGlobal(L, this, "scriptMgr");
	}

	~scriptManager() {
		if (L) {
			lua_close(L);
		}
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

};
const char* scriptManager::REGISTRY_KEY = "SCRIPT_MANAGER_PTR";