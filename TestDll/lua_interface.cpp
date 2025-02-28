#include "stdafx.h"
#include "lua_interface.h"
#include "TestDlg.h"


void lua_interface::registerClasses(lua_State* L)
{
	using namespace luabridge;

	// 类导出到lua
	getGlobalNamespace(L)
		.beginClass<bag>("bag") //bag类
		.addConstructor<void(*)()>()
		.addFunction("init", &bag::init)
		.addFunction("ifHasPoison", &bag::ifHasPoison)
		.endClass()

		.beginClass<tools>("tools")//tools类
		.addStaticFunction("getInstance", &tools::getInstance)
		//.addFunction("message", &tools::message)
		.addFunction("improveProcPriv", &tools::improveProcPriv)
		//.addFunction("findPattern", &tools::findPattern)
		.addFunction("getModuleInfo", &tools::getModuleInfo)
		.addFunction("log2file", &tools::log2file)
		.addFunction("write2file", &tools::write2file)
		.addFunction("write2file_c", &tools::write2file_c)
		//.addFunction("read", &tools::read)
		//.addFunction("write", &tools::write)
		//.addFunction("byte2hex", &tools::byte2hex)
		//.addFunction("lower2upper", &tools::lower2upper)
		//.addFunction("hexstr2upper", &tools::hexstr2upper)
		//.addFunction("hex2byte", &tools::hex2byte)
		//.addFunction("bytes2hexstr", &tools::bytes2hexstr)
		//.addFunction("hexstr2bytes", &tools::hexstr2bytes)
		.addFunction("fileIsexist", &tools::fileIsexist)
		.addFunction("ReadTxt", &tools::ReadTxt)
		.addFunction("splitString", &tools::splitString)
		.addFunction("GetCurrDir", &tools::GetCurrDir)
		.addFunction("getParentPath", &tools::getParentPath)
		.addFunction("char2wchar", &tools::char2wchar)
		.addFunction("wchar2char", &tools::wchar2char)
		.addFunction("ReleaseResource", &tools::ReleaseResource)
		.addFunction("eipinjectDll", &tools::eipinjectDll)
		.addStaticFunction("exeload", &tools::exeload)
		.endClass()

		.beginClass<gamecall>("gamecall") //gamecall类
		.addConstructor<void(*)()>()
		.addFunction("OpendNPC", &gamecall::OpendNPC)
		.addFunction("ChooseCmd", &gamecall::ChooseCmd)
		.addFunction("pickupGoods", &gamecall::pickupGoods)
		.addFunction("team_open_close", &gamecall::team_open_close)
		.addFunction("maketeam", &gamecall::maketeam)
		.addFunction("allowteam", &gamecall::allowteam)
		.addFunction("OpenRecovry", &gamecall::OpenRecovry)
		.addFunction("RecovryGoods", &gamecall::RecovryGoods)
		.addFunction("RecovryGoods_To_Exp", &gamecall::RecovryGoods_To_Exp)
		.addFunction("RecovryGoods_To_Gold", &gamecall::RecovryGoods_To_Gold)
		//.addFunction("SubmitInputbox", &gamecall::SubmitInputbox)
		.endClass()
		
	

		.beginClass<lua_interface>("LuaInterface") // 注册本类方法到 Lua
		.addConstructor<void(*)(void)>()
		.addFunction("模拟按键", &lua_interface::presskey)
		.addFunction("交赏金", &lua_interface::applySJLP)
		.addFunction("买药", &lua_interface::buyMedicine)
		.addFunction("回城整理背包", &lua_interface::getGoodsProcessIndex) //回城调用 获取物品处理方式写入index_vec_store .. 之后才能调用下面的待存待卖物品
		.addFunction("待存物品数量", &lua_interface::getStoreGoodsNumber)
		.addFunction("待卖首饰数量", &lua_interface::getSellJewelryNumber)
		.addFunction("待卖衣服数量", &lua_interface::getSellClothesNumber)
		.addFunction("待卖武器数量", &lua_interface::getSellWeaponNumber)
		.addFunction("存仓库", static_cast<bool(lua_interface::*)()>(&lua_interface::storeGoods))
		.addFunction("卖衣服", static_cast<bool(lua_interface::*)()>(&lua_interface::sellClothes))
		.addFunction("卖首饰", static_cast<bool(lua_interface::*)()>(&lua_interface::sellJewelry))
		.addFunction("卖武器", static_cast<bool(lua_interface::*)()>(&lua_interface::sellWeapon))
		.addFunction("卖药", static_cast<bool(lua_interface::*)()>(&lua_interface::sellMedicine))
		.addFunction("当前坐标X", &lua_interface::getCurrentX)
		.addFunction("当前坐标Y", &lua_interface::getCurrentY)
		.addFunction("当前地图名", &lua_interface::getCurrentMapName)
		.addFunction("背包剩余格子", &lua_interface::getBagSpace)
		.addFunction("最大背包格子", &lua_interface::getBagSpaceMax)
		.addFunction("当前背包负重", &lua_interface::getBagWeightRemain)
		.addFunction("最大背包负重", &lua_interface::getBagWeightMax)
		.addFunction("计算物品数量", &lua_interface::getBagGoodsNumber)
		.addFunction("当前血量", &lua_interface::getCurrentHP)
		.addFunction("当前蓝量", &lua_interface::getCurrentMP)
		.addFunction("红药数量", &lua_interface::getBagHpMedcine)
		.addFunction("蓝药数量", &lua_interface::getBagMpMedcine)
		.addFunction("当前金币", &lua_interface::getGoldNumber)
		.addFunction("当前元宝", &lua_interface::getYbNumber)
		.addFunction("当前绑定金币", &lua_interface::getBindGoldNumber)
		.addFunction("当前绑定元宝", &lua_interface::getBindYbNumber)
		.addFunction("使用物品", &lua_interface::useBagGoods)
		.addFunction("对自己使用技能", &lua_interface::useSkillToMyself)
		.addFunction("对目标使用技能", &lua_interface::useSkillToTarget)
		.addFunction("小退", &lua_interface::smallExit)
		.addFunction("立即复活", &lua_interface::immdiaRebirth)
		.addFunction("跑到目标点", &lua_interface::runTo)
		.addFunction("走到目标点", &lua_interface::walkTo)
		.addFunction("寻路到", &lua_interface::gotoMapXY)
		.addFunction("开始战斗", &lua_interface::startAttack)
		.addFunction("停止战斗", &lua_interface::endAttack)
		.addFunction("计算距离", &lua_interface::getDistance)
		.addFunction("对话NPC选择命令", &lua_interface::interactNPC) //交互NPC并选择一个命令
		.addFunction("选择命令", &lua_interface::chooseNpcCommand)
		.endClass();
}


//重定向lua的print函数
int lua_interface::lua_print(lua_State* L) {
	int nargs = lua_gettop(L); // 获取参数数量
	for (int i = 1; i <= nargs; ++i) {
		if (lua_isstring(L, i)) {
			std::cout << lua_tostring(L, i) << "\t"; // 输出到C++的std::cout
		}
	}
	std::cout << std::endl;
	return 0;
}

//封装presskey
bool lua_interface::presskey(int vkcode)
{
	return mfun.presskey(::GetCurrentProcessId(),vkcode);
}


// 加载并解析的lua map文件 包含地图编号与名字的映射关系 以及地图之间的路径关系
void lua_interface::load_and_store_map_data(lua_State* L, const std::string& file_path)
{
	// 加载 Lua 文件
	int result = luaL_loadfile(L, file_path.c_str());
	if (result != 0) {
		std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // 移除错误消息
		return;
	}

	// 执行 Lua 文件
	result = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (result != 0) {
		std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // 移除错误消息
		return;
	}

	// 确保返回值是一个表
	if (!lua_istable(L, -1)) {
		std::cerr << "预期返回一个表" << std::endl;
		return;
	}

	// 获取全局表
	lua_pushnil(L); // 第一个键
	while (lua_next(L, -2) != 0) {
		// 'key' 是在栈顶 (-1)，'value' 是在栈顶的下一个位置 (-2)
		if (lua_type(L, -2) == LUA_TSTRING) {
			const char* key = lua_tostring(L, -2);

			if (std::string(key) == "map_names") {
				if (!lua_istable(L, -1)) {
					std::cerr << "map_names 不是表类型" << std::endl;
					lua_pop(L, 1); // 移除 'value'
					continue;
				}
				lua_pushnil(L); // 第一个键
				while (lua_next(L, -2) != 0) {
					if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING) {
						std::cerr << "map_names 键或值不是字符串类型" << std::endl;
						lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
						continue;
					}
					const char* map_id = lua_tostring(L, -2);
					const char* map_name = lua_tostring(L, -1);
					map_names[map_id] = map_name;
					lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
				}
			}
			else if (std::string(key) == "transitions") {
				if (!lua_istable(L, -1)) {
					std::cerr << "transitions 不是表类型" << std::endl;
					lua_pop(L, 1); // 移除 'value'
					continue;
				}
				lua_pushnil(L); // 第一个键
				while (lua_next(L, -2) != 0) {
					if (lua_type(L, -2) != LUA_TSTRING || !lua_istable(L, -1)) {
						std::cerr << "transitions 键不是字符串类型或值不是表类型" << std::endl;
						lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
						continue;
					}
					const char* from_map = lua_tostring(L, -2);
					TransitionMap transition_map;

					lua_pushnil(L); // 第一个键
					while (lua_next(L, -2) != 0) {
						if (lua_type(L, -2) != LUA_TSTRING || !lua_istable(L, -1)) {
							std::cerr << "transitions 内部键不是字符串类型或值不是表类型" << std::endl;
							lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
							continue;
						}
						const char* to_map = lua_tostring(L, -2);
						std::vector<Position> positions;

						lua_pushnil(L); // 第一个键
						while (lua_next(L, -2) != 0) {
							if (!lua_istable(L, -1)) {
								std::cerr << "positions 值不是表类型" << std::endl;
								lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
								continue;
							}
							Position pos;
							lua_pushnil(L); // 第一个键
							while (lua_next(L, -2) != 0) {
								if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TNUMBER) {
									std::cerr << "positions 键或值不是数字类型" << std::endl;
									lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
									continue;
								}
								const char* coord_key = lua_tostring(L, -2);
								if (std::string(coord_key) == "x") {
									pos.x = static_cast<int>(lua_tointeger(L, -1));
								}
								else if (std::string(coord_key) == "y") {
									pos.y = static_cast<int>(lua_tointeger(L, -1));
								}
								lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
							}

							positions.push_back(pos);

							lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
						}

						transition_map[to_map] = positions;
						lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
					}
					transitions[from_map] = transition_map;

					// 如果没有找到任何 'to_map'，输出提示信息
					if (transition_map.empty()) {
						std::cout << "Warning: No transitions found for From map: " << from_map << std::endl;
					}
					lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
				}
			}
		}
		lua_pop(L, 1); // 移除 'value'，保留 'key' 以便继续迭代
	}
}

//找到跨图路径
bool lua_interface::find_path(const std::string& start_name, const std::string& end_name, std::vector<std::string>& path) {
	std::unordered_map<std::string, std::string> came_from;
	std::queue<std::string> queue;

	// 根据地图名称找到对应的ID
	std::string start_id, end_id;
	for (const auto& pair : map_names) {
		if (pair.second == start_name) {
			start_id = pair.first;
		}
		if (pair.second == end_name) {
			end_id = pair.first;
		}
	}

	if (start_id.empty() || end_id.empty()) {
		std::cerr << "未找到指定的地图名称" << std::endl;
		return false;
	}

	queue.push(start_id);
	came_from[start_id] = "";

	while (!queue.empty()) {
		std::string current = queue.front();
		queue.pop();

		if (current == end_id) {
			// 回溯路径
			std::string node = end_id;
			while (!node.empty()) {
				path.insert(path.begin(), node);
				node = came_from[node];
			}
			return true;
		}

		auto it = transitions.find(current);
		if (it != transitions.end()) {
			for (const auto& transition : it->second) {
				const std::string& to_map = transition.first;
				if (came_from.find(to_map) == came_from.end()) {
					queue.push(to_map);
					came_from[to_map] = current;
				}
			}
		}
	}

	return false;
}

//找到跨图路径和过图坐标
std::vector<std::pair<std::string, std::vector<Position>>> lua_interface::find_path_with_positions(const std::string& start_name, const std::string& end_name)
{
	std::vector<std::pair<std::string, std::vector<Position>>> path_with_positions; // 存储路径和过图坐标
	std::vector<std::string> path;// 存储路径
	if (!find_path(start_name, end_name, path))
	{
		std::cerr << "Failed to find path from " << start_name << " to " << end_name << std::endl;
		return path_with_positions;
	}
	for (size_t i = 0; i < path.size() - 1; ++i) {
		const std::string& from_map = path[i];
		const std::string& to_map = path[i + 1];

		auto it_from = transitions.find(from_map);
		if (it_from != transitions.end()) {
			auto it_to = it_from->second.find(to_map);
			if (it_to != it_from->second.end()) {
				path_with_positions.push_back({ to_map, it_to->second });
			}
			else {
				std::cerr << "未找到从 " << from_map << " 到 " << to_map << " 的过渡点" << std::endl;
			}
		}
		else {
			std::cerr << "未找到地图 " << from_map << " 的过渡信息" << std::endl;
		}
	}

	return path_with_positions;
}

//通过名字获取玩家周围NPCID
DWORD lua_interface::getEviroNPCIdByName(std::string npcName)
{
	if (!r.Get_Envionment(m_obj.p_pets, m_obj.p_npcs, m_obj.p_monster, m_obj.p_players))
	{
		std::cout << "遍历周围错误!!!" << std::endl;
		return -1;
	}

	for (size_t i = 0; i < m_obj.p_npcs.size(); i++)
	{
		if (npcName.compare((char*)(m_obj.p_npcs[i]+0x10)) == 0) return *(DWORD*)(m_obj.p_npcs[i] + 8);  // 返回匹配的NPC的ID
	}
		return -1;
}

// 通用 NPC 交互函数
bool lua_interface::interactWithNPC(const std::string& npcName, const std::string& command, const std::function<bool(int)>& action) {
	auto npcId = getEviroNPCIdByName(npcName);
	if (npcId == -1) {
		std::cerr << "无法找到 NPC: " << npcName << std::endl;
		return false;
	}

	if (!mfun.OpendNPC(npcId)) {
		std::cerr << "打开 NPC 对话框失败: " << npcName << std::endl;
		return false;
	}

	Sleep(100);

	if (!mfun.ChooseCmd(command.c_str())) {
		std::cerr << "选择命令失败: " << command << std::endl;
		return false;
	}

	Sleep(100);

	return action(npcId);
}

// 通用卖物品函数
bool lua_interface::sellItems(const std::vector<DWORD>& bagIndexSell, int npcId) {
	for (auto index : bagIndexSell) {
		const auto& item = r_bag.m_bag[index];
		if (!mfun.sellGoods(std::string(item.pName), *item.ID, npcId)) {
			std::cerr << "出售物品失败: " << item.pName << std::endl;
			return false;
		}
		memset((DWORD*)item.Name_Length, 0, 0x688); // 清空相关字段
		Sleep(50);
	}
	return true;
}

/*封装游戏函数------------------------------------------------------------------------------------------------*/

//提交赏金令牌
void lua_interface::applySJLP()
{
	int num = r_bag.caclGoodsNumber("赏金令牌");
	std::cout <<"赏金令牌的数量为：" << num << std::endl;
	if (num > 0)
	{
		auto npcid = getEviroNPCIdByName("赏金猎人");
		if (npcid == -1)return;
		mfun.OpendNPC(npcid);
		for (auto i = 0; i < num; ++i)
		{
			Sleep(50);
			mfun.ChooseCmd("@givepai");
			Sleep(50);
			mfun.ChooseCmd("@main");
		}
	}
}

//买药  参数一：名字  参数二：数量 返回bool
bool lua_interface::buyMedicine(const std::string& medName, BYTE num) {
	return interactWithNPC("药店掌柜", "@bindbuy", [&](int npcId) {
		if (*r.m_roleproperty.p_Role_GoldBind > 100000) { // 使用绑定金币
			if (!mfun.buyGoods(medName, npcId, 1, num)) {
				std::cerr << "购买药品失败: " << medName << std::endl;
				return false;
			}
			return true;
		}
		else if (*r.m_roleproperty.p_Role_Gold > 100000) { // 使用金币
			if (!mfun.buyGoods(medName, npcId, 0, num)) {
				std::cerr << "购买药品失败: " << medName << std::endl;
				return false;
			}
			return true;
		}
		else {
			std::cerr << "金币和绑定金币不足，购买失败: " << medName << std::endl;
			return false;
		}
		});
}

//保存物品 参数:待存物品的背包索引容器
bool lua_interface::storeGoods(const std::vector<DWORD>& bagIndexStore) {
	return interactWithNPC("仓库保管员", "@storage", [&](int npcId) {
		for (auto index : bagIndexStore) {
			const auto& item = r_bag.m_bag[index];
			if (!mfun.storeGoods(std::string(item.pName), *item.ID, npcId)) {
				std::cerr << "保存物品失败: " << item.pName << std::endl;
				return false;
			}
			memset((DWORD*)item.Name_Length, 0, 0x688); // 清空相关字段
			Sleep(100);
		}
		return true;
		});
}

//卖药 参数:待卖物品的背包索引容器
bool lua_interface::sellMedicine(const std::vector<DWORD>& bagIndexMedSell) {
	return interactWithNPC("药店掌柜", "@sell", [&](int npcId) {
		for (auto index : bagIndexMedSell) {
			const auto& item = r_bag.m_bag[index];
			if (!mfun.sellGoods(std::string(item.pName), *item.ID, npcId)) {
				std::cerr << "出售药品失败: " << item.pName << std::endl;
				return false;
			}
			memset((DWORD*)item.Name_Length, 0, 0x688); // 清空相关字段
			Sleep(50);
		}

		// 处理剩余药品
		for (const auto& kv : r_bag.m_bag) {
			if (kv.howProcess == 4 && kv.goods_type == 4 && kv.remainNumbers < (DWORD)r_bag.caclGoodsNumber(kv.pName)) {
				if (!mfun.sellGoods(std::string(kv.pName), *kv.ID, npcId)) {
					std::cerr << "出售剩余药品失败: " << kv.pName << std::endl;
					return false;
				}
				memset((DWORD*)kv.Name_Length, 0, 0x688); // 清空相关字段
				Sleep(50);
			}
		}
		return true;
		});
}

//卖衣服 待卖物品的背包索引容器
bool lua_interface::sellClothes(const std::vector<DWORD>& bagIndexCloSell) {
	return interactWithNPC("服装店掌柜", "@sell", [&](int npcId) {
		return sellItems(bagIndexCloSell, npcId);
		});
}

//卖首饰 待卖物品的背包索引容器
bool lua_interface::sellJewelry(const std::vector<DWORD>& bagIndexJeSell) {
	return interactWithNPC("首饰店掌柜", "@sell", [&](int npcId) {
		return sellItems(bagIndexJeSell, npcId);
		});
}

//卖武器 待卖物品的背包索引容器
bool lua_interface::sellWeapon(const std::vector<DWORD>& bagIndexWpSell) {
	return interactWithNPC("铁匠", "@sell", [&](int npcId) {
		return sellItems(bagIndexWpSell, npcId);
		});
}

// 通用 NPC 交互函数
bool lua_interface::interactNPC(const std::string& npcName, const std::string& command) {
	auto npcId = getEviroNPCIdByName(npcName);
	if (npcId == -1) {
		std::cerr << "无法找到 NPC: " << npcName << std::endl;
		return false;
	}

	if (!mfun.OpendNPC(npcId)) {
		std::cerr << "打开 NPC 对话框失败: " << npcName << std::endl;
		return false;
	}

	Sleep(800);

	if (!mfun.ChooseCmd(command.c_str())) {
		std::cerr << "选择命令失败: " << command << std::endl;
		return false;
	}
	Sleep(500);
	return true;
}

//解析storeANDsell.ini 解析需要存仓和卖出的物品名字列表
std::vector<std::string>  bag::StoreVec, bag::SellWeaponVec, bag::SellClothesVec, bag::SellJewelryVec;//分别存储 存仓物品 卖武器 衣服 首饰 名字
std::map<std::string, DWORD> bag::SellMedicineVec;//存储 卖药品 的名字 和剩余数量
bool bag::initGoodsProcWayList()
{
	static bool initialized = false;
	if (!initialized) {
		std::string cfgpath = std::string(shareCli.m_pSMAllData->currDir) + "cfg\\" + r.m_roleproperty.Object.pName + "\\" + "storeANDsell.ini";
		auto data = tools::getInstance()->parseIniFile(cfgpath);
		if (data.empty())std::cerr << "OpenFile error！！" << std::endl;
		for (const auto& section : data)
		{
			if (section.first == std::string("仓库"))
			{
				for (const auto& kv : section.second)
				{
					StoreVec.push_back(kv.first);
					}
			}
			else if (section.first == std::string("衣服"))
			{
				for (const auto& kv : section.second)
				{
					SellClothesVec.push_back(kv.first);
				}
			}
			else if (section.first == std::string("首饰"))
			{
				for (const auto& kv : section.second)
				{
					SellJewelryVec.push_back(kv.first);
					}
			}
			else if (section.first == std::string("武器"))
			{
				for (const auto& kv : section.second)
				{
					SellWeaponVec.push_back(kv.first);

				}
			}
			else if (section.first == std::string("药品"))
			{
				for (const auto& kv : section.second) {
					try {
						DWORD value = std::stoul(kv.second);
						SellMedicineVec[kv.first] = value;
					}
					catch (const std::invalid_argument& e) {
						std::cerr << "Invalid argument: " << e.what() << std::endl;
					}
					catch (const std::out_of_range& e) {
						std::cerr << "Out of range: " << e.what() << std::endl;
					}
				}
			}

		}
		initialized = true;
		return true;
	}
	else return false;
}

//追加存储NPC 信息到Lua文件中
void lua_interface::saveNPCsToLua(const std::vector<NPC>& npcs, const std::string& filePath) {
	std::ofstream outputFile(filePath, std::ios::app); // 使用追加模式
	if (outputFile.is_open()) {
		std::map<std::string, std::unordered_map<NPC, bool, NPCHash>> mapNPCs;

		// 将 NPC 按地图分组并去重
		for (const auto& npc : npcs) {
			mapNPCs[npc.mapName][npc] = true;
		}

		// 检查文件是否为空，如果为空则写入表头
		outputFile.seekp(0, std::ios::end);
		bool isEmpty = (outputFile.tellp() == 0);
		if (isEmpty) {
			outputFile << "npcs = {\n";
		}

		for (const auto& mapEntry : mapNPCs) {
			const std::string& mapName = mapEntry.first;
			const std::unordered_map<NPC, bool, NPCHash>& mapNPCList = mapEntry.second;

			// 检查该地图是否已经存在
			outputFile.seekp(0, std::ios::end);
			bool mapExists = false;
			std::string line;
			std::ifstream checkFile(filePath);
			if (checkFile.is_open()) {
				while (std::getline(checkFile, line)) {
					if (line.find("[\"" + mapName + "\"]") != std::string::npos) {
						mapExists = true;
						break;
					}
				}
				checkFile.close();
			}

			if (!mapExists) {
				outputFile << "    [\"" << mapName << "\"] = {\n";
			}

			for (const auto& npcEntry : mapNPCList) {
				const NPC& npc = npcEntry.first;

				// 检查该 NPC 是否已经存在
				if (existingNPCs[mapName].find(npc) == existingNPCs[mapName].end()) {
					outputFile << "        [\"" << npc.npcName << "\"] = { x = " << npc.x << ", y = " << npc.y << " },\n";
					existingNPCs[mapName][npc] = true;
				}
			}

			if (!mapExists) {
				outputFile << "    },\n";
			}
		}

		// 如果文件为空，写入表尾
		if (isEmpty) {
			outputFile << "}\n";
		}

		outputFile.close();
		std::cout << "NPCs saved to " << filePath << std::endl;
	}
	else {
		std::cerr << "Unable to open file " << filePath << std::endl;
	}
}