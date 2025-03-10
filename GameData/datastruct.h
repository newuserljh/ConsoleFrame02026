#pragma once
#include <windows.h>

//周围对象
struct MONSTER_PROPERTY
{
	MONSTER_PROPERTY();
	MONSTER_PROPERTY(DWORD* p);
	~MONSTER_PROPERTY();

	DWORD*  ID;// +8
	char*  pName;// +0x10
	WORD* oType;// +0x68 //对象类型 0为玩家 ，1 为NPC， 2为怪物和宝宝(大刀和弓箭也是2)，0x400为心魔(各大地图守卫)
 	DWORD*  HP;// +0x80
	DWORD*  HP_MAX;// +0x84
	DWORD*  MP;// +0x88
	DWORD*  MP_MAX;// +0x8c
	DWORD*  X;// X;// +0xac 
	DWORD*  Y;// Y;// +0xb0
	BYTE*  IsPosion;// +0x34b  //是否中毒 0没毒，0x40红毒，0x80绿毒，0xc0红绿毒,
	float Distance; //对象和角色的距离
};
//人物属性
struct ROLE_PROPERTY
{
	ROLE_PROPERTY();
	~ROLE_PROPERTY();	
	MONSTER_PROPERTY Object;
	DWORD* VIP_Level; //+0x5cc VIP等级
	DWORD* Job;//  +0xe4 //职业 0/战士  1/法师 2/道士
	DWORD* Level;// +0xec //等级
	DWORD* Sex; //+0xdc 性别 0男 1女
	DWORD* GJL_L;//攻击 0xA5C
	DWORD* GJL_H;//+0xA60
	DWORD* MFL_L;//魔法+0xA64
	DWORD* MFL_H; //+0xA68
	DWORD* DSL_L;//道术+0xA6c
	DWORD* DSL_H;//+0xA70
	DWORD* FY_L;//防御//+0xA7C
	DWORD* FY_H;//+0xA80
	DWORD* MF_L;//防御/+0xA84 
	DWORD* MF_H;//0xA88
	WORD* BAG_W; // 背包重量 +0xAA8
	WORD* BAG_W_MAX; // 背包重量 //+0xAAA
	DWORD* SW; //声望//+0xAD0 
	DWORD* p_Bag_Base; //背包基址，第一个格子指针0x135fb30]+0xC54]+ i * 0x688   (0-0x41)
	DWORD* Bag_Size;//背包大小[0x135fb30  ]+0xc58 
	DWORD* p_LR_Bag_Base;//灵兽背包基址，第一个格子指针[0x135fb30  ]+0xc60  
	DWORD* LR_Bag_Size; //灵兽背包大小  +0xc64 
	DWORD* p_Target_ID; //选中/攻击 目标ID +0x458 
	DWORD* p_Skill_Base;//i为技能栏的顺序从0开始0x135fb30]+0x1358]+ i * 0x88
	DWORD* p_ZB;/*== [[0x135fb30 ]+ 0xc48] + i * 0x688 //身上装备 0衣服，1武器，2勋章，3项链，4头盔
					，5右手镯，6左手镯，7右戒指，8左戒指，9靴子，10腰带，11宝石，12护符，13盾牌，14魂珠,
					15 极品御兽天衣 ，16 紋佩 ，18 神龙背饰 ，19 面具 ，20 仙翼*/
	DWORD* LL; //[0x135fb30] + 0x139C//灵力
	char* p_Current_Map;// [135fb30] + 0x8299C8//所在地图 CHAR*
    DWORD* PatCount; ///宠物数量+0xA24
	DWORD* Team_is_allow;//[0x135fb30  ]+0x1054550 是否允许组队0不允许,1允许,可直接修改
	DWORD* Team_pointer;//+0x1054550+8 队伍链表对象指针 指向表头的指针 

	DWORD* Is_has_Promenade; //+0xAE4 / AE8  0无元神 1有元神; +0x13A0 4有 0 无

	//元神独有属性
	DWORD*  Is_Promenade_Release; // [0x135fb34]+0x3D8        元神是否释放 1是 0否  +0xAA4也是
	DWORD*  Promenade_Mode; // [0x135fb34]+0xA08        元神模式 0战斗 1跟随 2停止 可直接修改值改变模式

	
	DWORD* p_Role_EXP;// = [0x135fb30] + 0xA84//当前经验值 unsigned类型
	DWORD* p_Role_EXP_MAX;// = [0x135fb30] + 0xAA0//当前等级最大经验值 unsigned
	double* p_Role_YB_Bind;// = [0x135fb30] + 0xC08//绑定元宝 Double类型
	DWORD* p_Role_Gold;// = [0x135fb30] + 0xC10//金币
	DWORD* p_Role_GoldBind;// = [0x135fb30] + 0xC14//绑定金币
	DWORD* p_Role_YB;// = [0x135fb30] + 0xBEC//元宝

};



//技能对象
struct SKILL_PROPERTY 
{
	SKILL_PROPERTY();
	~SKILL_PROPERTY();
	DWORD* level;// +8
	DWORD* ID;// + 0xe
	char* pName;//+ 14
};

//物品对象
struct GOODS_PROPERTY
{
	GOODS_PROPERTY();
	~GOODS_PROPERTY();
	BYTE* Name_Length;// + 0
	char* pName;//+ 1
	BYTE*  WD_low;//+ 1a / /物理防御下  /幸运武器
	BYTE* WD_high;// + 1b 物理防御上  /命中武器
	BYTE* MD_low;//+ 1c / 魔防
	BYTE* MD_high;//+ 1D
	BYTE* PA_low;//+ 1e        //攻击力
	BYTE* PA_high;// + 1f
	BYTE* Magic_low;// + 20   //魔法攻击
	BYTE* Magic_high;// + 21
	BYTE* Tao_low;//+ 22   //道术攻击
	BYTE* Tao_high;//+ 23
	BYTE* Need_what;// + 24  佩戴需求 0等级 1攻击 2魔法 3道术
	BYTE* Need_Num;//+ 25  佩戴需求大小 等级 / 魔法。。
	DWORD* ID;//+ 2c
	WORD* Use_Num;//+ 30            耐久 / 使用次数
	WORD* Use_Num_Max;// + 32    最大耐久 / 使用次数

	//自加属性
	DWORD goods_type; //1 武器 2首饰 3衣服 4药品 5其他
	DWORD howProcess; //物品处理方式 5待定（未定义） 1存仓 2卖掉 3立即使用 4卖剩
	DWORD remainNumbers; //卖剩下多少 当howProcess=4时再计算此项

};

//物品对象
struct GROUND_GOODS
{
	GROUND_GOODS();
	GROUND_GOODS(DWORD* p);
	~GROUND_GOODS();
	char* pName;//+ 0x18
	DWORD* X;//+8
	DWORD* Y;//+0xc
	float Distance;//距离角色距离
};

//任务对象
struct TASK_PROPERTY
{
	TASK_PROPERTY();
	~TASK_PROPERTY();
	char* pName;
};

//坐标
struct MapXY
{
	DWORD x;
	DWORD y;
};

//队伍成员对象
struct TEAM_PROPERTY //队伍 链表 （ 第一个链条数据为空 ）链条总数为n+1 n为队伍总人数
{
	TEAM_PROPERTY();
	TEAM_PROPERTY(DWORD* p);
	~TEAM_PROPERTY();
	TEAM_PROPERTY* Next; //下一个链条指针
	TEAM_PROPERTY* Previous;//上一个链条指针  
	char* pName;//队员名字
};
