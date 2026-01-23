#pragma once
#include <unordered_map>
#include <queue>
#include "../GameData/bag.h"
#include "../GameData/role.h"
#include "../GameData/m_object.h"
#include "../GameData/skill.h"
#include "../GameData/team.h"
#include "../Common/utils.h"
#include "../GameData/gamecall.h"
#include "../luajit/src/lua.hpp"
#include "../Common/shareMemoryCli.h"
#include "LuaBridge/LuaBridge.h"


// 声明外部对象 
extern shareMemoryCli shareCli; 
extern int shareindex; 
extern role r; 
extern role ys; 
extern m_object m_obj; 
extern skill m_skill; 
extern bag r_bag; 
extern bag ys_bag;
extern bag pet_bag;
extern gamecall mfun; 
extern team m_team; 


// 存储地图名称和转换关系的映射 需要的 结构体 和 声明
struct Position {
	int x, y;
};
using TransitionMap = std::unordered_map<std::string, std::vector<Position>>;
using MapNames = std::unordered_map<std::string, std::string>;
using Transitions = std::unordered_map<std::string, TransitionMap>;
extern MapNames map_names; //地图名称映射
extern Transitions transitions; //地图转换


//存储NPC信息的 需要的 结构体 和 声明
struct NPC {
	std::string mapName;
	std::string npcName;
	int x;
	int y;
	bool operator==(const NPC& other) const {
		return mapName == other.mapName && npcName == other.npcName && x == other.x && y == other.y;
	}
};
// 用于 std::unordered_map 的哈希函数
struct NPCHash {
	std::size_t operator()(const NPC& npc) const {
		std::size_t h1 = std::hash<std::string>{}(npc.mapName);
		std::size_t h2 = std::hash<std::string>{}(npc.npcName);
		std::size_t h3 = std::hash<int>{}(npc.x);
		std::size_t h4 = std::hash<int>{}(npc.y);
		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

class lua_interface
{
public:
	lua_interface() : L(luaL_newstate(), &lua_close)
	{
		if (L) {
			luaL_openlibs(L.get());
		} 
	}
	~lua_interface() = default;

	static int lua_print(lua_State* L);
	void registerClasses(lua_State* L); //注册类到 Lua 中
	lua_State* getLuaState() const { return L.get(); }
	// 禁用拷贝构造函数和赋值操作符
	lua_interface(const lua_interface&) = delete;
	lua_interface& operator=(const lua_interface&) = delete;
private:
	std::unique_ptr<lua_State, decltype(&lua_close)> L;


public:
	//地图信息
	void load_and_store_map_data(lua_State* L, const std::string& file_path);
	std::vector<std::pair<std::string, std::vector<Position>>> find_path_with_positions(const std::string& start_name, const std::string& end_name);	
	bool find_path(const std::string& start_name, const std::string& end_name, std::vector<std::string>& path);//获取不带坐标的路径

	// NPC信息存储
	void  saveNPCsToLua(const std::vector<NPC>& npcs, const std::string& filePath);
	std::map<std::string, std::unordered_map<NPC, bool, NPCHash>> existingNPCs;

public:
	//游戏功能封装
	bool presskey(int vkcode);
	void applySJLP();//提交赏金令牌
	void getGoodsProcessIndex() { r_bag.getGoodsProcessIndex(); } //回城之后 获取物品处理方式的索引 写入index_vec_store .index_vec_sell_jewel...
	int getStoreGoodsNumber() { return r_bag.index_vec_store.size(); }//获取待存物品数量
	int getSellJewelryNumber() { return r_bag.index_vec_sell_jewel.size(); }//获取待卖首饰数量
	int getSellClothesNumber() { return r_bag.index_vec_sell_cloth.size(); }//获取待卖衣服数量
	int getSellWeaponNumber() { return r_bag.index_vec_sell_weapon.size(); }//获取待卖武器数量
	//const DWORD getSellMedicineNumber();//不用计算卖药品数量  要买药 到时候再卖
	// 新增无参版本的方法
	bool storeGoods() { return storeGoods(r_bag.index_vec_store); } //保存物品
	bool sellMedicine() { return sellMedicine(r_bag.index_vec_sell_medci); }//卖药
	bool sellClothes() { return sellClothes(r_bag.index_vec_sell_cloth); }//卖衣服
	bool sellJewelry() { return sellJewelry(r_bag.index_vec_sell_jewel); }//卖首饰
	bool sellWeapon() { return sellWeapon(r_bag.index_vec_sell_weapon); }//卖武器
	bool buyMedicine(const std::string& med_name, BYTE num);//买药
	int getCurrentHP() { return 100 * *r.m_roleproperty.Object.HP / *r.m_roleproperty.Object.HP_MAX; }//获取当前血量百分比
	int getCurrentMP() { return 100 * *r.m_roleproperty.Object.MP / *r.m_roleproperty.Object.MP_MAX; }// 获取当前蓝量百分比
	int getCurrentX() { return *r.m_roleproperty.Object.X; }//获取当前坐标X
	int getCurrentY() { return *r.m_roleproperty.Object.Y; }// 获取当前坐标Y
	std::string getCurrentMapName() { return r.m_roleproperty.p_Current_Map; }//获取当前地图名称
	int getBagSpaceMax(){return *r.m_roleproperty.Bag_Size;} //获取背包最大格子数量
	int getBagSpace() { return r_bag.getBagSpace(); }//获取背包剩余格子数量
	int getBagWeightRemain() { return *r.m_roleproperty.BAG_W; }//获取背包已使用负重
	int getBagWeightMax() { return *r.m_roleproperty.BAG_W_MAX; }//获取背包最大负重
	int getBagGoodsNumber(std::string& name) { return r_bag.caclGoodsNumber(name); }//获取背包物品数量
	int getBagHpMedcine() { return r_bag.caclHpGoodsNumer(); }//获取背包红药数量
	int getBagMpMedcine() { return r_bag.caclMpGoodsNumer(); }//获取背包蓝药数量
	int getGoldNumber(){return *r.m_roleproperty.p_Role_Gold;}//获取金币数量
	int getYbNumber() { return *r.m_roleproperty.p_Role_YB; }//获取元宝数量
	int getBindGoldNumber() { return *r.m_roleproperty.p_Role_GoldBind; }//获取绑定金币数量
	int getBindYbNumber() { return (int)*r.m_roleproperty.p_Role_YB_Bind; }//获取绑定元宝数量
	bool useBagGoods(const std::string& goodsName) { return mfun.useGoods(r_bag.getGoodsIndex(goodsName)); }//使用背包物品
	bool useSkillToMyself(std::string skillname) { return mfun.useSkillTo(m_skill.getSkillId(skillname.c_str()), getCurrentX(), getCurrentY(), 0); }//对自己使用技能
	bool useSkillToTarget(std::string skillname, int x, int y,DWORD tId) { return mfun.useSkillTo(m_skill.getSkillId(skillname.c_str()), x, y, tId); }//对目标使用技能
	bool smallExit() { return mfun.small_exit(); }//小退
	bool immdiaRebirth() { return mfun.immdia_rebirth(); }//立即复活
	bool runTo(int x, int y){ return mfun.Run_or_Step_To(x, y,2);}//跑到目标点 一次2格
    bool walkTo(int x, int y) { return mfun.Run_or_Step_To(x, y, 1); }//走到目标点 一次1格
    bool gotoMapXY(int x, int y) { return mfun.CurrentMapMove(x, y); }//寻路到地图坐标
	float getDistance(int x, int y) { return mfun.caclDistance(getCurrentX(), getCurrentY(), x, y); }	//计算与坐标(x,y)的距离 ,直线距离 无法计算实际距离
	bool interactNPC(const std::string& npcName, const std::string& command);//对话NPC 并选择一个命令
	bool chooseNpcCommand(const std::string& command) { return mfun.ChooseCmd(command.c_str()); }//选择NPC对话
	void startAttack();//开始战斗
	void endAttack(); //结束战斗
	bool  openOrCloseYS() { return mfun.release_Promenade(); }//释放/关闭元神

private:
	//买卖存仓辅助函数
	DWORD getEviroNPCIdByName(std::string npcName);//通过名字获取NPCID
	bool interactWithNPC(const std::string& npcName, const std::string& command, const std::function<bool(int)>& action);
	bool sellItems(const std::vector<DWORD>& bagIndexSell, int npcId);
	bool storeGoods(const std::vector<DWORD>& bagIndexStore = r_bag.index_vec_store);//保存物品
	bool sellClothes(const std::vector<DWORD>& bagIndexCloSell = r_bag.index_vec_sell_cloth);//卖衣服
	bool sellJewelry(const std::vector<DWORD>& bagIndexJeSell = r_bag.index_vec_sell_jewel);//卖首饰
	bool sellWeapon(const std::vector<DWORD>& bagIndexWpSell = r_bag.index_vec_sell_weapon);//卖武器
	bool sellMedicine(const std::vector<DWORD>& bagIndexMedSell = r_bag.index_vec_sell_medci);//卖药
};

