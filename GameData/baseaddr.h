#pragma once
#include <windows.h>
/*
基地址/偏移
*/

#define RoleBase 0x135fb30 //人物主体基址
#define PROMENADE_Base 0x135fb34 //元神基址
#define Envi_Offset 0x412D68 //NPC,怪物遍历偏移
#define Ground_Offset 0x412354 //地面物品遍历偏移


/*call 参数*/
#define CALL_ECX  0x135fb10 //所有call的ecx来源
#define CALL_ECX_1 0x4E072C4 //寻路call的ecx
#define CALL_ECX_2 0x0135FB08//按键CALL的ecx

#define CALL_EDX 0x110D9D4//对话call的edx

#define ATTACK_SLEEP 1210 //攻击间隔毫秒

#define ID_LONGIN 0x8C00CFE5 //登录控件ID

/*call*/
#define CALL_LOGIN   0x689B90   //登录call
#define CALL_PRESSKEY 0x0062C750 //按键call
#define CALL_USESKILL 0x733640  //技能call地址
#define CALL_CURRENT_MAPMOVE 0x007AF860 //当前地图寻路call地址
#define CALL_USEGOODS 0x00718A10 //使用背包物品call
#define CALL_OPENNPC 0x7BC280   //打开NPC
#define CALL_CHOOSE_CMD 0x6AC2D0//选择对话命令
#define CALL_PICKUP_GOODS 0x6C9A30  //拾取地面物品
#define CALL_STEP_TO_MAP 0x7331D0  //走路到指定坐标，每次走一格，需要连续调用
#define CALL_RUN_TO_MAP 0x733270    //跑步到指定坐标，每次走2格，需要连续调用，只能走到2格以内
#define CALL_TEAM_OPEN_CLOSE 0x76F2B0  //队伍开关
#define CALL_MAKE_TEAM 0x76F7B0    //发起组队请求
#define CALL_ALLOW_TEAM 0x7358C0    //同意组队请求
#define CALL_RELEASE_PROMENADE 0x754F80 //释放元神
#define CALL_START_END_ATTACK 0x5D2E60 //开关自动挂机
#define CALL_OPEN_BAG_REC 0x6AC2D0 //获取角色对象
#define CALL_RCV_GOODS 0x68F490 //回收物品
#define CALL_SUBMIT_INPUTBOX 0x006E41E0 //提交输入框内容call
#define CALL_SMALL_EXIT 0x68E250//小退call
#define CALL_IMMDIA_REBIRTH 0x00E306D0 //立即复活call
#define CALL_GET_BUY_GOODS_ID 0x00BAD830 //获取买物品的物品ID
#define CALL_BUY_GOODS 0x0068F000 //获取买物品的物品ID
#define CALL_SELL_GOODS 0x68EF10 //卖物品 卖完小退
#define CALL_STORE_GOODS 0x68EE10 //存物品 存完小退
#define CALL_TAKE_PUT_ROLE_BAG_GOODS 0x00543310 //鼠标 拿起/放下 本体包裹物品
#define CALL_TAKE_PUT_YS_BAG_GOODS 0x00546AD0 //鼠标 拿起/放下 元神包裹物品
#define CALL_TAKE_PUT_PAT_BAG_GOODS 0x00548580 //鼠标 拿起/放下 灵兽包裹物品

#define P_CURRENT_NPC_DLG 0x110d9d4 //当前对话NPC ID指针 ,跟选择对话call的EDX指针一样
/*NPC ID*/
#define NPC_ZJ_RCV 0xcc716a0 //中州 物资回收使者
#define NPC_ZJ_CZHK 0xcc74300 //中州 充值回馈
#define NPC_ZJ_FBHW 0x d411d60 //中州 副本护卫
#define NPC_ZJ_ZHP 0xcc3da30 //中州 杂货铺掌柜
#define NPC_ZJ_LB 0xcd141e0 //中州 老兵
#define NPC_ZJ_RCRW 0xcc6d410 //中州 日常任务仙子 
#define NPC_ZJ_ZBKJ 0xcc70070 //中州 珍宝空间 
#define NPC_ZJ_YXSZ 0xcc58300 //中州 游戏使者
#define NPC_ZJ_RANDOM 0xcc9e0e0 //中州 随机传送门   
#define NPC_ZJ_YDZG 0xcc36b40 //中州 药店掌柜 ID: cc36b40
#define NPC_ZJ_BJCSM 0xcc75930 //中州 便捷传送门 ID: cc75930
#define NPC_ZJ_BS 0xcc6bde0 //中州 镖师 ID: cc6bde0
#define NPC_ZJ_TJ 0xcc328b0 //中州 铁匠 ID: cc328b0
#define NPC_ZJ_CK 0xcc3add0 //中州 仓库保管员 ID: cc3add0
#define NPC_ZJ_SJLR 0xcc4faa0 //中州 赏金猎人 ID: cc4faa0

#define NPC_ZJ_HG_MRMFFL 0xcc60820 //中州皇宫 每日免费福利 ID: cc60820
#define NPC_ZJ_HG_TSL 0xcc5c590//中州皇宫 太史令 ID: cc5c590
#define NPC_ZJ_HG_QZLB 0xcc5dbc0 //中州皇宫 钱庄老板 ID: cc5dbc0
#define NPC_ZJ_HG_OZZ 0xcc5af60 //中州皇宫 欧治子 ID: cc5af60
#define NPC_ZJ_HG_JZL 0xcc5f1f0 //中州皇宫 金子陵 ID: cc5f1f0
#define NPC_ZJ_HG_GW 0xccabec0 //中州皇宫 国王 ID: ccabec0

#define NPC_XY_LB 0xcceef40 //西域奇境 老兵 ID: cceef40

#define NPC_LY_HD_MJCSM 0xcde4550 //海岛 魔镜传送门 ID: cde4550
#define NPC_LY_HD_LYCSM 0xcde71b0 //海岛 炼狱传送门 ID: cde71b0
#define NPC_LY_BHPY_FHCSM 0xcd98fb0 //滨海平原 返回海岛传送门 ID: cd98fb0
#define NPC_LY_BHPY_LYCSM 0xcd8ad90 //滨海平原 炼狱传送门 传送门 ID: cd8ad90

#define NPC_LY_ML_FHCSM 0xcd97980 //密林 返回海岛传送门 ID: cd97980
#define NPC_LY_ML_LYCSM 0xcd89760 //密林 炼狱传送门 ID: cd89760










