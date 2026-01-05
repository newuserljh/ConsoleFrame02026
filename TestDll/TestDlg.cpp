// TestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <iomanip>
#include "TestDll.h"
#include "TestDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <mutex>
#include "../GameData/m_object.h"
#include "../GameData/skill.h"
#include "../GameData/bag.h"
#include "../GameData/team.h"
#include "../Common/utils.h"
#include "../GameData/gamecall.h"
#include "../Common/shareMemoryCli.h"
#include "../GameData/config.h"
#pragma comment(lib , "Common.lib")
#pragma comment(lib ,"GameData.lib")
#include "scriptManager.h"
//#include "HookAPI.h"
#include "../Common/HookReg.h"
// CTestDlg 对话框


std::unique_ptr<lua_interface> g_lua; //luainterface智能指针
std::unique_ptr<scriptManager> g_mgr; //scriptManager 脚本管理器智能指针
// 显式初始化智能指针
void init_global_objects() {
	g_lua = std::make_unique<lua_interface>();
	g_mgr = std::make_unique<scriptManager>(g_lua.get());
}

// 显式销毁智能指针
void cleanup_global_objects() {
	g_mgr.reset();
	g_lua.reset();
}

shareMemoryCli shareCli(MORE_OPEN_NUMBER);
int shareindex = -1;
//初始化HOOk
HookReg hook;
HookReg hook_npc_cmd;
role r;//角色
role Promenade; //元神
m_object m_obj;
skill m_skill;
bag r_bag;
gamecall mfun;
team m_team;

std::mutex team_mutex;  // 队伍文件变量互斥 

MapNames map_names; //地图名称映射
Transitions transitions; //地图转换


_declspec(naked) void CallTest()
{
	_asm pushad
	if (hook.EAX != 100)
	{
		tools::getInstance()->message("错误!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		shareCli.m_pSMAllData->m_sm_data[shareindex].server_alive = false;//验证外挂存活
	}
	_asm  popad
	_asm ret
}

extern CTestDlg* pDlg;

IMPLEMENT_DYNAMIC(CTestDlg, CDialogEx)


CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestDlg::IDD, pParent)
	, m_EditLuaPath(_T(""))
{
	
}

CTestDlg::~CTestDlg()
{
	cleanup_global_objects();
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_edit2);
	DDX_Text(pDX, IDC_EDIT_LUA_PATH, m_EditLuaPath);
}


BEGIN_MESSAGE_MAP(CTestDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON8, &CTestDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CTestDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON4, &CTestDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_CHK_TEAM, &CTestDlg::OnBnClickedChkTeam)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_GJ, &CTestDlg::OnBnClickedBtnGj)
	ON_BN_CLICKED(IDC_BTN_RECNPC, &CTestDlg::OnBnClickedBtnRecnpc)
	ON_BN_CLICKED(IDC_BTN_LUATST, &CTestDlg::OnBnClickedBtnLuatst)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_BAGPROC, &CTestDlg::OnBnClickedBtnBagproc)
	ON_BN_CLICKED(IDC_BUTTON6, &CTestDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BTN_CHOOSE_LUA_FILE, &CTestDlg::OnBnClickedBtnChooseLuaFile)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CTestDlg 消息处理程序

/*自动登陆线程*/
void threadLogin()
{
	int i = 0;//尝试登录次数
	DWORD npid = GetCurrentProcessId();
	do
	{
		shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = "登录中...";
		Sleep(5000);
		mfun.presskey(npid, VK_RETURN);
		Sleep(100);
		mfun.loginGame(shareCli.m_pSMAllData->m_sm_data[shareindex].userName.c_str(), shareCli.m_pSMAllData->m_sm_data[shareindex].passWord.c_str());
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		mfun.presskey(npid);
		Sleep(1000);
		i++;
	} while ((!r.init()) && i < 10);
	if (!r.init()) //包括了 角色和角色装备的初始化
	{
		shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = "登陆失败,正在结束程序";
		shareCli.m_pSMAllData->m_sm_data[shareindex].server_alive = false;
	}
	else
	{

		shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = "登陆完成";
		shareCli.m_pSMAllData->m_sm_data[shareindex].roleName = std::string(r.m_roleproperty.Object.pName);

		//登陆成功 初始化
		pDlg->SetTimer(22222, 5000, NULL);	//设置定时器5s 检测角色是否死亡
		//初始化背包  技能 队伍
		m_skill.skillBase = (DWORD)r.m_roleproperty.p_Skill_Base;
		m_skill.init();
		m_team.team_Base = r.m_roleproperty.Team_pointer;
		pDlg->init_team(); //包含了组队的定时器 SetTimer(11111, 30000, NULL)

		std::string usercfgpath = (std::string)shareCli.m_pSMAllData->currDir + "cfg\\" + r.m_roleproperty.Object.pName;
		CreateDirectory(usercfgpath.c_str(), NULL);// 创建 角色名 文件夹（存放角色的配置
		if (tools::getInstance()->fileIsexist(usercfgpath + "\\" + "storeANDsell.ini") == false)
		{
			std::string defaultpath = (std::string)shareCli.m_pSMAllData->currDir + "cfg\\default\\storeANDsell.ini" ;
			tools::getInstance()->copyFile(defaultpath, usercfgpath + "\\storeANDsell.ini");
		}
		bag::initGoodsProcWayList(); // 初始化 存储和出售物品的配置
		r_bag.maxSize = *r.m_roleproperty.Bag_Size;
		r_bag.bagBase = (DWORD)r.m_roleproperty.p_Bag_Base;
		r_bag.init();
		pDlg->SetTimer(99999, 500, NULL);//用于初始智能指针
	}
	return;
}


/*判断辅助存活,判断角色是否活着，线程*/
void threadAlive()
{
	while (true)
	{
		//if ((*r.m_roleproperty.Object.HP_MAX != 0) && (*r.m_roleproperty.Object.HP == 0))
		//{
		//	shareCli.m_pSMAllData->m_sm_data[shareindex].server_alive = false;
		//	shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = std::string("角色死亡，重登");
		//	return;
		//}
		shareCli.m_pSMAllData->m_sm_data[shareindex].rcv_rand = shareCli.m_pSMAllData->m_sm_data[shareindex].send_rand;//验证外挂存活
		shareCli.m_pSMAllData->m_sm_data[shareindex].ndPid=GetCurrentProcessId();//读取游戏进程PID
		Sleep(2000);
	}
}

// 函数：追加文本到EditControl
void AppendText(CEdit &m_edit, CString strAdd)
{
	m_edit.SetSel(m_edit.GetWindowTextLength(), m_edit.GetWindowTextLength());
	m_edit.ReplaceSel(strAdd + L"\n");
}


BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_edit2.SetLimitText(0); // 解除字符限制
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT2);
	static EditStreamBuf editBuf(pEdit);
	std::cout.rdbuf(&editBuf); // 重定向 cout
	// 为 std::cerr 创建缓冲区
	static EditStreamBuf cerrBuf(pEdit);
	std::cerr.rdbuf(&cerrBuf); // 重定向 std::cerr
		//初始化 线程标志
	tflag_attack = true;
	tflag_goto = true;
	tflag_pickup = true;
	tflag_autoavoid = true;
	tflag_processBag = true;

  //初始化共享内存,取得共享内存索引
	if (!shareCli.openShareMemory())
	{
		tools::getInstance()->message("打开共享内存失败\n");
	}
	shareindex = shareCli.getIndexByPID(GetCurrentProcessId());

	//载入地图数据到内存
	//std::string input_file = (std::string)shareCli.m_pSMAllData->currDir + "map\\map_data.lua";
	//m_luaInterface.load_and_store_map_data(L, input_file);

	std::string cfgtpath = (std::string)shareCli.m_pSMAllData->currDir + "cfg";
	CreateDirectory(cfgtpath.c_str(), NULL);// 创建 cfg 文件夹（如果不存在）

	//HOOK连接服务器失败代码
	hook.hookReg(0x5F8B69, 5, CallTest);

	//CloseHandle(::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(threadLogin), NULL, NULL, NULL));

	//登录成功之后设置 启动通讯线程,定时验证存活消息
	//CloseHandle(::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(threadAlive), NULL, NULL, NULL));

			//登陆成功 初始化
	r.init();
	pDlg->SetTimer(22222, 5000, NULL);	//设置定时器5s 检测角色是否死亡
	//初始化背包  技能 队伍
	m_skill.skillBase = (DWORD)r.m_roleproperty.p_Skill_Base;
	m_skill.init();
	m_team.team_Base = r.m_roleproperty.Team_pointer;
	pDlg->init_team(); //包含了组队的定时器 SetTimer(11111, 30000, NULL)

	std::string usercfgpath = (std::string)shareCli.m_pSMAllData->currDir + "cfg\\" + r.m_roleproperty.Object.pName;
	CreateDirectory(usercfgpath.c_str(), NULL);// 创建 角色名 文件夹（存放角色的配置
	if (tools::getInstance()->fileIsexist(usercfgpath + "\\" + "storeANDsell.ini") == false)
	{
		std::string defaultpath = (std::string)shareCli.m_pSMAllData->currDir + "cfg\\default\\storeANDsell.ini";
		tools::getInstance()->copyFile(defaultpath, usercfgpath + "\\storeANDsell.ini");
	}
	bag::initGoodsProcWayList(); // 初始化 存储和出售物品的配置
	r_bag.maxSize = *r.m_roleproperty.Bag_Size;
	r_bag.bagBase = (DWORD)r.m_roleproperty.p_Bag_Base;
	r_bag.init();
	pDlg->SetTimer(99999, 500, NULL);//用于初始智能指针


	//hookAPI(WSARecv, MyWSARecv);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

/*初始化打怪相关设置*/
bool CTestDlg:: initVariable()
{
	i_map = 0;
	s_ID = -1;

	
	/*载入拾取物品*/
	pick_goods_list.clear();
	pick_goods_list = tools::getInstance()->ReadTxt(std::string(shareCli.m_pSMAllData->currDir) + "cfg\\拾取物品.txt");
	if (!pick_goods_list.size())return false;
	/*设置技能*/
	if (!Set_Skill())return false;
	/*载入打怪地图路线*/
	if (!Load_coordinate())return false;
	/*载入攻击怪物列表*/
	attack_monlist.clear();
	attack_monlist = tools::getInstance()->ReadTxt(std::string(shareCli.m_pSMAllData->currDir) + "cfg\\逆魔大殿.txt");
	if (!attack_monlist.size())return false;

	return true;
}


//函数:自动回收物品，将军团物品回收为经验，逆魔物品回收为元宝
void CTestDlg::AutoRecvGoods(void)
{
	/*回收经验*/
	if (r_bag.caclGoodsNumber("幽冥项链") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("幽冥项链")].ID),*r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("阎罗手套") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("阎罗手套")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("光芒手套") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("光芒手套")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("天珠项链") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("天珠项链")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("心灵护腕") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("心灵护腕")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("白金戒指") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("白金戒指")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("生命项链") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("生命项链")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("魔魂手镯") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("魔魂手镯")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("红宝戒指") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("红宝戒指")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("龙戒") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("龙戒")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("记忆项链") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("记忆项链")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("记忆头盔") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("记忆头盔")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("战神(男)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("战神(男)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("战神(女)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("战神(女)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("幽灵战甲(男)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("幽灵战甲(男)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("幽灵战甲(女)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("幽灵战甲(女)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("恶魔长袍(男)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("恶魔长袍(男)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("恶魔长袍(女)") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("恶魔长袍(女)")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("离别钩") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("离别钩")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("方天画戢") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("方天画戢")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("井中月") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("井中月")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("铜锤") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("铜锤")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("无极") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("无极")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("魔法权杖") > 0) {
		mfun.RecovryGoods_To_Exp(*(r_bag.m_bag[r_bag.getGoodsIndex("魔法权杖")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(500);
	}

	/*回收元宝*/
	if (r_bag.caclGoodsNumber("恶魔铃") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("恶魔铃")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("龙鳞手镯") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("龙鳞手镯")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("碧螺戒指") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("碧螺戒指")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("魔龙腰带") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("魔龙腰带")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("魔龙法靴") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("魔龙法靴")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("黑铁头盔") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("黑铁头盔")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("绿色项链") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("绿色项链")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("猛士手镯") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("猛士手镯")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("力量戒指") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("力量戒指")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("雷霆腰带") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("雷霆腰带")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("雷霆战靴") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("雷霆战靴")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("灵魂项链") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("灵魂项链")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("三眼护腕") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("三眼护腕")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("泰坦戒指") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("泰坦戒指")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("风云腰带") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("风云腰带")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
	if (r_bag.caclGoodsNumber("风云道靴") > 0) {
		mfun.RecovryGoods_To_Gold(*(r_bag.m_bag[r_bag.getGoodsIndex("风云道靴")].ID), *r.m_roleproperty.VIP_Level);
		Sleep(200);
	}
}

//函数:回城处理
void CTestDlg::AutoReturnToCity()
{
	// 检查背包中是否有回城卷轴
	if (r_bag.caclGoodsNumber("永久回城神石") > 0)
	{
		// 使用回城卷轴
		mfun.useGoods(r_bag.getGoodsIndex("永久回城神石"));
		Sleep(2000); // 等待回城操作完成

		// 回城后处理包裹
		AutoRecvGoods();

		// 检查背包空间
		if (r_bag.getBagSpace() < 10)
		{
			// 背包空间不足，处理多余物品
			if (r_bag.caclGoodsNumber("强效太阳神水") > REMAIN_TAIYANG)
			{
				mfun.useGoods(r_bag.getGoodsIndex("强效太阳神水"));
				Sleep(100);
			}
			if (r_bag.caclGoodsNumber("治疗药水") > REMAIN_TAIYANG)
			{
				mfun.useGoods(r_bag.getGoodsIndex("治疗药水"));
				Sleep(200);
			}
		}
	}
	else
	{
		// 背包中没有回城卷轴，提示用户
		AfxMessageBox("背包中没有回城卷轴，请补充！");
	}
}

// 遍历人物属性
void CTestDlg::OnBnClickedButton1()
{
	if (!r.init()) return;
	std::cout << r.m_roleproperty.Object.pName << std::endl;
	char* sex = (*r.m_roleproperty.Sex == 0) ? "男" : "女";
	char* job = (*r.m_roleproperty.Job == 0) ? "战士" :(*r.m_roleproperty.Job == 1) ? "法师" : "道士";
	std::cout << "职业:" << job << "      等级:" << *r.m_roleproperty.Level << "    性别:" << sex << std::endl;
	std::cout << "HP:" << *r.m_roleproperty.Object.HP << " / " << *r.m_roleproperty.Object.HP_MAX << std::endl;
	std::cout << "MP:" << *r.m_roleproperty.Object.MP << " / " << *r.m_roleproperty.Object.MP_MAX << std::endl;
	std::cout << "当前地图:" << r.m_roleproperty.p_Current_Map << "  坐标" << *r.m_roleproperty.Object.X << "," << *r.m_roleproperty.Object.Y << std::endl;
	std::cout << "背包大小:" << *r.m_roleproperty.Bag_Size << "  背包负重" << *r.m_roleproperty.BAG_W << " / " << *r.m_roleproperty.BAG_W_MAX << std::endl;
	std::cout << "ID:" << std::hex << *r.m_roleproperty.Object.ID << std::dec << std::endl;
	// 输出装备信息
	for (auto i = 0; i < 21; i++)
	{
		if (!(*r.m_euip[i].ID)) continue;
		std::cout << i << " :" << r.m_euip[i].pName << " 耐久:" << *(r.m_euip[i].Use_Num) << "/" << *(r.m_euip[i].Use_Num_Max) << std::endl;
	}

	// 元神属性
	if (*r.m_roleproperty.Is_has_Promenade == 0) return;
	Promenade.init_promenade();
	if (*Promenade.m_roleproperty.Is_Promenade_Release == 0) mfun.release_Promenade();
	if (!Promenade.init_promenade()) return;
	std::cout << Promenade.m_roleproperty.Object.pName << std::endl;
	sex = (*Promenade.m_roleproperty.Sex == 0) ? "男" : "女";
	job = (*Promenade.m_roleproperty.Job == 0) ? "战士" :(*Promenade.m_roleproperty.Job == 1) ? "法师" : "道士";
	std::cout << "职业:" << job << "      等级:" << *Promenade.m_roleproperty.Level << "    性别:" << sex << std::endl;
	std::cout << "HP:" << *Promenade.m_roleproperty.Object.HP << " / " << *Promenade.m_roleproperty.Object.HP_MAX << std::endl;
	std::cout << "MP:" << *Promenade.m_roleproperty.Object.MP << " / " << *Promenade.m_roleproperty.Object.MP_MAX << std::endl;
	std::cout << "当前地图:" << Promenade.m_roleproperty.p_Current_Map << "  坐标" << *Promenade.m_roleproperty.Object.X << "," << *Promenade.m_roleproperty.Object.Y << std::endl;
	std::cout << "背包大小:" << *Promenade.m_roleproperty.Bag_Size << "  背包负重" << *Promenade.m_roleproperty.BAG_W << " / " << *Promenade.m_roleproperty.BAG_W_MAX << std::endl;
	std::cout << "ID:" << std::hex << *Promenade.m_roleproperty.Object.ID << std::dec << std::endl;
	// 输出元神装备信息
	for (auto i = 0; i < 21; i++)
	{
		if (!(*Promenade.m_euip[i].ID)) continue;
		std::cout << i << " :" << Promenade.m_euip[i].pName << " 耐久:" << *(Promenade.m_euip[i].Use_Num) << "/" << *(Promenade.m_euip[i].Use_Num_Max) << std::endl;
	}

	// 计算并输出元神与主体的距离
	Promenade.m_roleproperty.Object.Distance = mfun.caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y, *Promenade.m_roleproperty.Object.X, *Promenade.m_roleproperty.Object.Y);
	std::cout << "距主体距离:" << Promenade.m_roleproperty.Object.Distance << std::endl;
}
//int
//WSAAPI
//MyWSARecv(
//	_In_ SOCKET s,
//	_In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
//	_In_ DWORD dwBufferCount,
//	_Out_opt_ LPDWORD lpNumberOfBytesRecvd,
//	_Inout_ LPDWORD lpFlags,
//	_Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
//	_In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
//)
//{
//	
//	rcvryAPI(WSARecv, rcvryCode);
//   int ret= WSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
//	int er=WSAGetLastError();
//	if (er != 10035)
//	{
//		tools::getInstance()->message("错误:%d", er);
//	}
//
//	hookAPI(WSARecv, MyWSARecv);
//	return ret;
//}

//  遍历周围对象 地面 怪物NPC

void CTestDlg::OnBnClickedButton2()
{
	r.init();
	if (!r.Get_Envionment(m_obj.p_pets, m_obj.p_npcs, m_obj.p_monster, m_obj.p_players)){
		std::cerr<<"遍历周围错误："<<std::endl;
		return;
	}

	if (!r.Get_Ground(m_obj.p_ground)){
		std::cerr << "遍历地面错误：" << std::endl;
		return;
	}

	m_obj.init();

	auto printEntityInfo = [&](const auto& entities, const std::string& label) {
		std::cout << "\n" << label << "：" << std::endl;
		for (const auto& entity : entities)
		{
			std::cout << "指针: " << std::hex << std::setw(8) << std::setfill('0')
				<< reinterpret_cast<uintptr_t>(entity.pName) - 0x10
				<< "  " << entity.pName
				<< " ID: " << std::hex << *(entity.ID)
				<< ", 坐标(x,y)："
				<< std::dec << *(entity.X) << ", " << *(entity.Y)
				<< "   距离："
				<< std::fixed << std::setprecision(2)
				<< mfun.caclDistance(*(r.m_roleproperty.Object.X), *(r.m_roleproperty.Object.Y),
					*(entity.X), *(entity.Y))
				<< std::endl;
		}
		};

	printEntityInfo(m_obj.m_monster, "怪物");
	printEntityInfo(m_obj.m_players, "玩家");
	printEntityInfo(m_obj.m_pets, "宝宝");
	printEntityInfo(m_obj.m_npcs, "NPC");

	std::cout << "\n地面：" << std::endl;
	for (const auto& ground : m_obj.m_ground)
	{
		std::cout << " " << ground.pName
			<< " : " << *ground.X
			<< "/" << *ground.Y
			<< std::endl;
	}
}

// TODO: 技能遍历
void CTestDlg::OnBnClickedButton3()
{
	r.init();
	m_skill.skillBase = reinterpret_cast<DWORD>(r.m_roleproperty.p_Skill_Base);
	if (!m_skill.init())
	{
		std::cerr << "遍历技能错误!" << std::endl;
		return;
	}

	std::cout << "技能：" << std::endl;
	for (size_t i = 0; i < m_skill.m_skillList.size(); i++)
	{
		std::cout << m_skill.m_skillList[i].pName
			<< "  等级:" << *(m_skill.m_skillList[i].level)
			<< "  ID: " << *(m_skill.m_skillList[i].ID)
			<< std::endl;
	}
}

// TODO: 背包遍历
void CTestDlg::OnBnClickedButton8()
{
	if (!r.init()) return;

	r_bag.maxSize = *r.m_roleproperty.Bag_Size;
	r_bag.bagBase = reinterpret_cast<DWORD>(r.m_roleproperty.p_Bag_Base);

	if (!r_bag.init())
	{
		std::cout << "遍历背包错误：" << std::endl;
		return;
	}

	std::cout << "大小: " << r_bag.maxSize << "   剩余: " << r_bag.getBagSpace() << std::endl;

	for (size_t i = 0; i < r_bag.maxSize; i++)
	{
		if (*(r_bag.m_bag[i].ID))
		{
			std::cout << "第" << i << "格: " << r_bag.m_bag[i].pName
				<< "   ID: " << std::hex << std::setw(8) << std::setfill('0') << *r_bag.m_bag[i].ID
				<< " 处理方式: " << std::dec << r_bag.m_bag[i].howProcess;

			if (r_bag.m_bag[i].howProcess == 4)
			{
				std::cout << " " << r_bag.m_bag[i].remainNumbers << "个";
			}

			std::cout << std::endl;
		}
	}
}

//函数功能:选择打怪目标 参数:攻击怪物列表 返回值：选中怪物对象指针
MONSTER_PROPERTY Choose_Moster( std::vector<std::string>& vec)
{
	MONSTER_PROPERTY ret;
	if(*r.m_roleproperty.p_Target_ID == 0)
	{
		std::vector<MONSTER_PROPERTY> near_atak_mon_list = mfun.sort_aroud_monster(r, vec);
		if (near_atak_mon_list.size() > 0)
		{
			ret = near_atak_mon_list[0];
			*r.m_roleproperty.p_Target_ID = *ret.ID;
		}
		else
		{
			/*边寻路边找怪*/
			if (pDlg->i_map == pDlg->map_xy.size())pDlg->i_map = 0;
			MapXY xy = pDlg->map_xy[pDlg->i_map];
			mfun.CurrentMapMove(xy.x, xy.y);//寻路0.8s
			Sleep(800);
			mfun.CurrentMapMove(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y);//停止
			if (mfun.caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y,xy.x,xy.y)<4)pDlg->i_map++;
		}
	}
	return ret;
}

//函数功能:自动打怪 //参数1:攻击怪物列表 //参数2:使用技能ID// 返回值：true为已经打死怪物，fasle为错误（或者600次没打死怪物）
bool Auto_Attack( std::vector<std::string>& vec, DWORD s_ID)
{
	*r.m_roleproperty.p_Target_ID = 0;
	MONSTER_PROPERTY att_mon = Choose_Moster(vec);
	if (nullptr == att_mon.ID)return false;
	unsigned i = 0;/*记录攻击次数*/
	while( (*att_mon.HP> 0) && (0 != *r.m_roleproperty.p_Target_ID) && (i < 600))
	{
		if (mfun.caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y, *att_mon.X, *att_mon.Y) > 7)
		{
			mfun.CurrentMapMove(*att_mon.X, *att_mon.Y);
			Sleep(500);
		}
		DWORD s_posion = m_skill.getSkillId("施毒术");
		if( (s_posion!=-1))//道士
		{
			if ((r_bag.ifHasPoison()>0)&& (*att_mon.IsPosion < 0x40))//是否中毒 0没毒，0x40红毒，0x80绿毒，0xc0红绿毒,
			{
				mfun.useSkillTo(s_posion, *att_mon.X, *att_mon.Y, *att_mon.ID);
				Sleep(ATTACK_SLEEP);
				mfun.useSkillTo(s_posion, *att_mon.X, *att_mon.Y, *att_mon.ID);
				Sleep(ATTACK_SLEEP);
				/*	mfun.presskey(VK_F1);
					Sleep(ATTACK_SLEEP);*/
			}
		}
		mfun.useSkillTo(s_ID, *att_mon.X, *att_mon.Y, *att_mon.ID);
		i++;
		Sleep(ATTACK_SLEEP);
	}
	return true;
}

//函数功能:设置打怪技能
bool CTestDlg::Set_Skill()
{
	/*技能设置*/
	if (*r.m_roleproperty.Job == 0)/*战士*/
	{
		return false;
	}
    if (*r.m_roleproperty.Job == 1)/*法师*/
	{
		DWORD  s_tmp = m_skill.getSkillId("狂龙紫电");
		if (s_tmp != -1)
		{
			s_ID = s_tmp;
			return true;
		}
		s_tmp = m_skill.getSkillId("雷电术");
		if (s_tmp != -1)
		{
			s_ID = s_tmp;
			return true;
		}
		s_tmp = m_skill.getSkillId("冰箭术");
		if (s_tmp != -1)
		{
			s_ID = s_tmp;
			return true;
		}
		s_tmp = m_skill.getSkillId("小火球");
		if (s_tmp != -1)
		{
			s_ID = s_tmp;
			return true;
		}
		return false;
	}
	if (*r.m_roleproperty.Job == 2)/*道士*/
	{
		return false;
	}
	 return false;

}


//函数功能:载入寻路打怪坐标
bool CTestDlg::Load_coordinate()
{
	map_xy.clear();
	std::vector<std::string> map_coordinate= tools::getInstance()->ReadTxt(std::string(shareCli.m_pSMAllData->currDir) + "cfg\\逆魔大殿1.txt");
	if (!map_coordinate.size())return false;
	for (size_t i = 0; i < map_coordinate.size(); i++)
	{
		MapXY xy;
		xy = mfun.splitXY(map_coordinate[i]);
		if ((xy.x!=-1)&&(xy.y!=-1))map_xy.push_back(xy);
	}
	if (!map_xy.size())return false;
	return true;
}

//函数功能:初始化组队Timer 和 队伍人员
bool CTestDlg::init_team()
{
	m_team.init();
	m_team_check_id = 0;
	pBtn = (CButton*)GetDlgItem(IDC_CHK_TEAM);  //获得组队复选框控件的句柄
	team_list.clear();
	team_list = tools::getInstance()->ReadTxt(std::string(shareCli.m_pSMAllData->currDir )+ "cfg\\队伍人员.txt");
	if (pBtn->GetCheck())
	{
		if (*r.m_roleproperty.Team_is_allow != 1)mfun.team_open_close(1);//允许组队
		m_team_check_id = SetTimer(11111, 30000, NULL);
	}
	return true;
}


/*组队函数：读取组队文件并组队*/
void  CTestDlg::MakeTeam()
{	
	m_team.init();
	if (team_list.size()<2)return;
	if (team_list.size() == m_team.m_team_list.size())return;//组队完毕
	if (strcmp(pDlg->team_list[0].c_str(),r.m_roleproperty.Object.pName)==0)//我是队长
	{
		for (size_t i = 1; i < team_list.size(); i++)
		{
			size_t temp = i;
			for (size_t j = 0; j < m_team.m_team_list.size(); j++)
			{
				if (strcmp(pDlg->team_list[i].c_str(), m_team.m_team_list[j].pName) == 0)
				{
					temp = i + 1;
					break;
				}			
			}
			if (temp==i) mfun.maketeam(team_list[i]);
		}
	}
	else //我是队员
	{
		if (shareCli.m_pSMAllData->team_info == 0)
		{
			pBtn->SetCheck(0);
			OnBnClickedChkTeam();
			return;
		}
		for (size_t j = 0; j < m_team.m_team_list.size(); j++)
		{
			if (strcmp(r.m_roleproperty.Object.pName, m_team.m_team_list[j].pName) == 0)
			{
				return;
			}
		}
	  mfun.allowteam(team_list[0]);//同意组队
	}
}

/*检测角色是否死亡*/
void  CTestDlg::RoleIsDeath(void)
{
	r.init();
	if (*r.m_roleproperty.Object.HP>0)return;
	//保存正在执行的任务到 ./cfg/角色名字.cfg
	std::string cfg_file_path = (std::string)shareCli.m_pSMAllData->currDir + "cfg\\";
	cfg_file_path = cfg_file_path + r.m_roleproperty.Object.pName +".cfg";
	tools::getInstance()->write2file(cfg_file_path, "当前任务", std::ios::out);

	if (tflag_attack) //停止打怪 闪避 背包处理线程
	{
		tflag_attack = false;
		WaitForSingleObject(m_threadAttack, 60000);
		WaitForSingleObject(m_threadBagProcess, 60000);
		if (tflag_autoavoid)
		{
			tflag_autoavoid = false;
			WaitForSingleObject(m_threadAutoAvoid, 60000);
		}
		KillTimer(22222);
	}

	mfun.immdia_rebirth();//立即复活
	Sleep(2000);
	mfun.presskey(GetCurrentProcessId());
	Sleep(200);
	mfun.presskey(GetCurrentProcessId());
	if (*r.m_roleproperty.Object.HP > 0)
	{
		SetTimer(22222, 5000, NULL);	//设置定时器5s 检测角色是否死亡
		auto curr_tast = tools::getInstance()->ReadTxt(cfg_file_path);
		//继续执行保存的任务

		return;
	}

	mfun.small_exit();//小退
	Sleep(2000);
	mfun.presskey(GetCurrentProcessId());
	Sleep(200);
	mfun.presskey(GetCurrentProcessId());
	if (*r.m_roleproperty.Object.HP > 0)
	{
		SetTimer(22222, 5000, NULL);	//设置定时器5s 检测角色是否死亡
		auto curr_tast = tools::getInstance()->ReadTxt(cfg_file_path);
		//继续执行保存的任务

		return;
	}
	shareCli.m_pSMAllData->m_sm_data[shareindex].server_alive = false;//由控制台大退
}

// Lua脚本线程 
void CTestDlg::RunLuaScriptInThread(LPVOID p, lua_State* L, const std::string& scriptPath, std::function<void(const std::string&)> errorCallback)
{
	CTestDlg* pdlg = (CTestDlg*)p;

	// 获取_G全局环境
	lua_getglobal(L, "_G");
	if (lua_isnil(L, -1)) {
		errorCallback("Lua 错误: _G 不存在");
		lua_pop(L, 1);
		return;
	}
	// 获取triggerSystem对象
	lua_getfield(L, -1, "triggerSystem");
	if (!lua_istable(L, -1)) {
		errorCallback("Lua 错误: triggerSystem 不存在或不是表");
		lua_pop(L, 2);
		return;
	}
	// 获取clear方法
	lua_getfield(L, -1, "clear");
	if (!lua_isfunction(L, -1)) {
		errorCallback("Lua 错误: clear 方法不存在或不是函数");
		lua_pop(L, 3);
		return;
	}
	// 将 self（triggerSystem 表）压入栈
	lua_pushvalue(L, -2);  // 复制 triggerSystem 到栈顶
	// 调用clear方法
	int result = lua_pcall(L, 1, 0, 0);
	if (result != LUA_OK) {
		if (lua_isstring(L, -1)) {
			const char* luaError = lua_tostring(L, -1);
			errorCallback("Lua 错误: " + std::string(luaError));
		}
		else {
			errorCallback("Lua 错误: 未知错误");
		}
		lua_pop(L, 1);
	}

	// 清理栈
	lua_pop(L, 3); // 弹出clear方法 triggerSystem对象 _G全局环境

	// 加载并执行 Lua 脚本
	// 调用前验证函数类型
	lua_getglobal(L, "executefile");
	if (!lua_isfunction(L, -1)) {
		std::string errorMsg = "致命错误: executefile 未定义或类型错误";
		errorCallback(errorMsg); // 通过回调传递错误
		lua_pop(L, 1);
		return;
	}

	// 准备参数
	lua_pushstring(L, scriptPath.c_str());
	// 调用函数 (1 参数, 0 返回值)
	result = lua_pcall(L, 1, 0, 0);
	if (result != LUA_OK) {
		if (lua_isstring(L, -1)) {
			const char* luaError = lua_tostring(L, -1);
			errorCallback("Lua 错误: " + std::string(luaError));
		}
		else {
			errorCallback("Lua 错误: 未知错误");
		}
		lua_pop(L, 1);
	}
	else {
		lua_pop(L, 1); // 弹出函数
	}
	pdlg->GetDlgItem(IDC_BTN_LUATST)->EnableWindow(TRUE);
}
/*寻路线程*/
UINT __cdecl CTestDlg::threadGoto(LPVOID p)
{
	CTestDlg* pDlg = (CTestDlg*)p;
	size_t i = 0;
	while (pDlg->tflag_goto)
	{
		MapXY xy = pDlg->map_xy[i];
		shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = std::string( "正在寻路到");
		//MapGoto_Point(pDlg->map_xy[i].x, pDlg->map_xy[i].y, pDlg->tflag_goto);
		i++;
		if (i = pDlg->map_xy.size())i = 0;
	}
	return 0;
}

/*打怪线程*/
UINT __cdecl CTestDlg::threadAttack(LPVOID p)
{
	CTestDlg* pDlg = (CTestDlg*)p;
	std::cout<<"打怪线程开始 "<<std::endl;
	mfun.start_end_AutoAttack(pDlg->tflag_attack);	
	auto poision_skill_ID = m_skill.getSkillId("施毒术");
	while (pDlg->tflag_attack)
	{
		shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = std::string("打怪");
		//Auto_Attack(pDlg, pDlg->attack_monlist, pDlg->s_ID);
		mfun.start_end_AutoAttack(pDlg->tflag_attack);
		Sleep(100);
		DWORD* pt= mfun.getTargetP(r);
		if (nullptr != pt&& poision_skill_ID)//判断是否有目标
		{
			auto t_mon= MONSTER_PROPERTY(pt);
			if (r_bag.ifHasPoison() > 0)//判断是否有毒药
			{
					if (*t_mon.IsPosion < 0x40)//是否中毒 0没毒，0x40红毒，0x80绿毒，0xc0红绿毒,
					{
						mfun.start_end_AutoAttack(false);
						Sleep(900);
						mfun.useSkillTo(poision_skill_ID, *t_mon.X,*t_mon.Y,*t_mon.ID);
						Sleep(ATTACK_SLEEP);
						mfun.useSkillTo(poision_skill_ID, *t_mon.X, *t_mon.Y, *t_mon.ID);
						Sleep(100);
						mfun.start_end_AutoAttack(pDlg->tflag_attack);
					}
			}
		}
		Sleep(2400);
	}
	mfun.start_end_AutoAttack(false);
	shareCli.m_pSMAllData->m_sm_data[shareindex].cscript = std::string("空闲");
	std::cout << "打怪线程停止 " << std::endl;
	return 0;
}

/*包裹处理*/
UINT __cdecl CTestDlg::threadBagPocess(LPVOID p)
{
	CTestDlg* pDlg = (CTestDlg*)p;
	std::cout << "处理包裹线程开始 " << std::endl;
	while (pDlg->tflag_processBag)
	{
		////打开快捷回收
		//try
		//{
		//	_asm
		//	{
		//		pushad
		//		mov ecx, dword ptr ds : [CALL_ECX]
		//		mov edi, 0x6EE710
		//		call edi
		//		popad
		//	}
		//}
		//catch (...) {}
		//Sleep(200);
		//mfun.ChooseCmd("@一键回收");
		//Sleep(200);
		//if (r_bag.caclGoodsNumber("RMB兑换卷") > 0) {
		//			mfun.useGoods(r_bag.getGoodsIndex("RMB兑换卷"));
		//			Sleep(200);
		//		}
		//mfun.ChooseCmd("@回收逆魔装备");
		//Sleep(200);
		//mfun.ChooseCmd("@回收将军装备");
		//Sleep(200);
		//mfun.ChooseCmd("@回收技能书籍");
		//Sleep(200);
		//	pDlg->AutoRecvGoods();
		if (r_bag.caclGoodsNumber("1积分卷") > 0) {
			mfun.useGoods(r_bag.getGoodsIndex("1积分卷"));
			Sleep(200);
		}
		if (r_bag.caclGoodsNumber("2积分卷") > 0) {
			mfun.useGoods(r_bag.getGoodsIndex("2积分卷"));
			Sleep(200);
		}
		if (r_bag.caclGoodsNumber("3积分卷") > 0) {
			mfun.useGoods(r_bag.getGoodsIndex("3积分卷"));
			Sleep(200);
		}
			if (r_bag.caclGoodsNumber("1元宝") > 0) {
				mfun.useGoods(r_bag.getGoodsIndex("1元宝"));
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("2元宝") > 0) {
				mfun.useGoods(r_bag.getGoodsIndex("2元宝"));
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("5元宝") > 0) {
				mfun.useGoods(r_bag.getGoodsIndex("5元宝"));
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("10元宝") > 0) {
				mfun.useGoods(r_bag.getGoodsIndex("10元宝"));
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("3元宝") > 0)
			{
				mfun.useGoods(r_bag.getGoodsIndex("3元宝"));
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("声望令牌(一)") > 0)
			{
				mfun.useGoods(r_bag.getGoodsIndex("声望令牌(一)")>0);
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("声望令牌(二)") > 0)
			{
				mfun.useGoods(r_bag.getGoodsIndex("声望令牌(二)") > 0);
				Sleep(200);
			}
			if (r_bag.caclGoodsNumber("声望令牌(三)") > 0)
			{
				mfun.useGoods(r_bag.getGoodsIndex("声望令牌(三)") > 0);
				Sleep(200);
			}
		//if (r_bag.getBagSpace() < 10)
		//{
		//	if (r_bag.caclGoodsNumber("强效太阳神水") > REMAIN_TAIYANG)
		//	{
		//		mfun.useGoods(r_bag.getGoodsIndex("强效太阳神水"));
		//		Sleep(100);
		//	}
		//	if (r_bag.caclGoodsNumber("治疗药水") > REMAIN_TAIYANG) {
		//		mfun.useGoods(r_bag.getGoodsIndex("治疗药水"));
		//		Sleep(200);
		//	}
		//}
		Sleep(5000);
	}
	std::cout << "处理包裹线程停止 " << std::endl;
	return 0;
}

/*智能闪避线程*/
UINT __cdecl CTestDlg::threadAutoAvoidMon(LPVOID p)
{
	CTestDlg* pDlg = (CTestDlg*)p;
	std::cout << "智能闪避线程开始 " << std::endl;
	while (pDlg->tflag_autoavoid)
	{
		pDlg->AutoAvoidMonsters();
		Sleep(2000);
	}
	std::cout << "智能闪避线程停止 " << std::endl;
	return 0;
}

/*捡物线程*/
UINT __cdecl CTestDlg::threadPickup(LPVOID p)
{
	CString s;
	CTestDlg* pDlg = (CTestDlg*)p;
	std::cout << "捡物线程开始 " << std::endl;
	while (pDlg->tflag_pickup)
	{
		std::vector<GROUND_GOODS> need2pick_list = mfun.sort_groud_goods(r, pDlg->pick_goods_list);
		if (need2pick_list.size())
		{
			GROUND_GOODS pick_temp = need2pick_list[0];
			std::vector<MONSTER_PROPERTY> near_mon = mfun.sort_aroud_monster(r, pDlg->attack_monlist, 6);
			if (near_mon.size() < 5)
			{
				pDlg->m_threadAttack->SuspendThread();
				s.Format("正在拾取物品:%s 坐标：%d,%d", pick_temp.pName, *pick_temp.X, *pick_temp.Y);
				std::cout << s << std::endl;
				int pick_try_accounts = 0;
				do
				{
					mfun.Run_or_Step_To(*pick_temp.X, *pick_temp.Y, 1);
					Sleep(800);
					mfun.Run_or_Step_To(*pick_temp.X, *pick_temp.Y, 1);
					Sleep(800);
					mfun.Run_or_Step_To(*pick_temp.X, *pick_temp.Y, 1);
					Sleep(800);
					pick_try_accounts++;
					if ((*r.m_roleproperty.Object.X == *pick_temp.X) && (*r.m_roleproperty.Object.X == *pick_temp.Y))
					{
						mfun.pickupGoods(*pick_temp.X, *pick_temp.Y);
					}
					need2pick_list.clear();
					need2pick_list = mfun.sort_groud_goods(r, pDlg->pick_goods_list);
					if (!need2pick_list.size())break;
				} while ((need2pick_list[0].X == pick_temp.X) && (need2pick_list[0].Y == pick_temp.Y) && (pick_try_accounts < 4));
				pDlg->m_threadAttack->ResumeThread();
			}
			else
			{
				Sleep(10000);//打怪
			}
		}
		Sleep(1000);
	}
	std::cout << "捡物线程停止 " << std::endl;
	return 0;
}

// TODO: 功能测试
void CTestDlg::OnBnClickedButton9()
{
	r.init();
	r_bag.maxSize = *r.m_roleproperty.Bag_Size;
	r_bag.bagBase = (DWORD)r.m_roleproperty.p_Bag_Base;
	r_bag.init();

	std::cerr << "cerr test message!" << std::endl;
std::cout << "cout test message!" << std::endl;	

//打开快捷回收
try
{
	_asm
	{
		pushad
		mov ecx, dword ptr ds : [CALL_ECX]
		mov edi, 0x6EE710
		call edi
		popad
	}
}
catch (...){}
mfun.ChooseCmd("@一键回收");


	//m_skill.skillBase = (DWORD)r.m_roleproperty.p_Skill_Base;
	//m_skill.init();
	//m_team.team_Base = r.m_roleproperty.Team_pointer;
	//CString s;

	//m_luaInterface.getGoodsProcessIndex();
	//std::cout<<m_luaInterface.getStoreGoodsNumber()<<std::endl;
	//std::cout << m_luaInterface.getSellClothesNumber() << std::endl;
	//std::cout << m_luaInterface.getSellJewelryNumber() << std::endl;
	//std::cout << m_luaInterface.getSellWeaponNumber() << std::endl;




	//m_luaInterface.buyMedicine("超级魔法药", 5);
	//r_bag.getGoodsProcessIndex();
	//m_luaInterface.sellMedicine(r_bag.index_vec_sell_medci);
	////m_luaInterface.applySJLP();

	 //获取周围怪物信息

	//// 查找路径
	//std::string start_name = "落霞岛";
	//std::string end_name = "尸王殿";
	//auto  path_with_positions=m_luaInterface.find_path_with_positions(start_name, end_name);
	//if(!path_with_positions.empty())
	//{
	//	for (const auto& entry : path_with_positions) 
	//	{
	//		const std::string& id = entry.first;
	//		const std::vector<Position>& positions = entry.second;
	//		// 获取地图名称
	//		std::string map_name = map_names.count(id) ? map_names.at(id) : "未知地图";
	//		std::cout << "Map Name: " << map_name << ", Positions: ";
	//		for (const auto& pos : positions) {
	//			std::cout << "(" << pos.x << ", " << pos.y << ") ";
	//		}
	//		std::cout << std::endl;
	//	}
	//}
	//else {
	//	std::cout << "未找到从 " << start_name << " 到 " << end_name << " 的路径\n";
	//}

/*自动打怪 需要启动：①打怪线程优先级正常 ②遍历周围对象、地面并拾取线程 优先级中高 ③寻路线程、智能闪避 优先级最高
*
*          ****选怪
*自动打怪***   打怪：判断打死
*          ****继续打怪
* */

}

// 队伍遍历+任务遍历
void CTestDlg::OnBnClickedButton4()
{
	r.init();
	m_team.team_Base = r.m_roleproperty.Team_pointer;
	m_team.init();
	CString s;
	s.Format("队伍指针:%p", m_team.team_Base);
	std::cout<<s<<std::endl;
	if (m_team.m_team_list.size())
	{
		for (size_t i = 0; i < m_team.m_team_list.size(); i++)
		{
			s.Format("名字:%s NEXT:%p Pre:%p \n", m_team.m_team_list[i].pName, m_team.m_team_list[i].Next, m_team.m_team_list[i].Previous);
			std::cout << s << std::endl;
		}
	}
}

//组队CHECK单击事件
void CTestDlg::OnBnClickedChkTeam()
{
	// TODO: 组队CHECK单机事件
	pBtn = (CButton*)GetDlgItem(IDC_CHK_TEAM);  //获得组队复选框控件的句柄

	if (pBtn->GetCheck()&&(m_team_check_id==0))
	{
		if (*r.m_roleproperty.Team_is_allow != 1)mfun.team_open_close(1);//允许组队
		m_team_check_id = SetTimer(11111, 30000, NULL);
	}
	else
	{
		if (m_team_check_id != 0)
		{			
			KillTimer(11111);
			m_team_check_id = 0;
			if (strcmp(team_list[0].c_str(),r.m_roleproperty.Object.pName)!=0)mfun.team_open_close(0);//队员直接关闭队伍开关
			else//队长 逐个删除队员之后再关闭队伍开关
			{
				shareCli.m_pSMAllData->team_info = 0;
				std::cout << "你是队长,正在逐个关闭队员队伍开关,稍等10s" << std::endl;
			}
		}
	}
}

//智能闪避，自动躲避怪物
void CTestDlg::AutoAvoidMonsters()
{
	if (!r.init()) return;

	// 获取周围怪物信息
	if (!r.Get_Envionment(m_obj.p_pets,m_obj.p_npcs,m_obj.p_monster,m_obj.p_players))
	{
		std::cout << "获取周围怪物信息错误" << std::endl;
		return;
	}
	m_obj.init();

	// 遍历怪物列表，找到最近的怪物
	MONSTER_PROPERTY* nearestMonster = nullptr;
	float minDistance = FLT_MAX;
	for (auto& monster : m_obj.m_monster)
	{
		float distance = mfun.caclDistance(*r.m_roleproperty.Object.X, *r.m_roleproperty.Object.Y, *monster.X, *monster.Y);
		if (distance < minDistance)
		{
			minDistance = distance;
			nearestMonster = &monster;
		}
	}
	// 如果找到最近的怪物，并且距离小于一定值，则进行躲避
	if (nearestMonster && minDistance < 3.0f) // 假设3.0f是需要躲避的距离阈值
	{
		// 计算躲避方向
		int rx = *r.m_roleproperty.Object.X;
		int ry = *r.m_roleproperty.Object.Y;
		int dx = rx - *nearestMonster->X;
		int dy = ry - *nearestMonster->Y;
		

		// 尝试移动到多个方向，直到成功
		std::vector<std::pair<int, int>> directions = {
			{dx, dy}, {0, -3}, {0, 3}, {-3, -3}, {-3, 0}, {-3, 3}, {3, 3}, {3, 0}, {3, -3}
		};

		for (const auto& dir : directions)
		{
			int newX = rx + dir.first;
			int newY = ry + dir.second;
			mfun.Run_or_Step_To(newX, newY, 2);
			Sleep(500);

			// 如果成功移动到新位置，则退出循环
			if (*r.m_roleproperty.Object.X != rx || *r.m_roleproperty.Object.Y != ry)
			{
				break;
			}
		}
	}
	
	
}

/*定时器 1.组队  2.判断角色死亡 3.(一次性)初始化LUA环境*/
void CTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 11111:
		MakeTeam();
		break;
	case 22222:
		RoleIsDeath();
		break;
	case 99999:
	{
		KillTimer(99999);
		init_global_objects();
		{
			// 在 Lua 状态中设置全局变量 luaStopFlag,运行初始化脚本
			std::string scriptPath = (std::string)shareCli.m_pSMAllData->currDir + "script\\init_execute_env.lua";
			luaStopFlag.store(false);//重置停止标志
			lua_pushboolean(g_mgr.get()->L, luaStopFlag.load());
			lua_setglobal(g_mgr.get()->L, "luaStopFlag");
			g_mgr.get()->start(scriptPath);
		
		}
	}
		break;

	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);

}


// TODO: 使用内置自动打怪挂机
void CTestDlg::OnBnClickedBtnGj()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_BTN_GJ);
	if (m_threadAttack==NULL)
	{
		tflag_attack = true;
		m_threadAttack = AfxBeginThread(threadAttack, (LPVOID)this);
		if (*r.m_roleproperty.Job != 0)
		{
			tflag_autoavoid = true;
		}// 不是战士时开启智能闪避
		else tflag_autoavoid = false;
		if (tflag_autoavoid&&(m_threadAutoAvoid==NULL))
		{
			// 设置智能闪避线程，用于自动躲避怪物
			m_threadAutoAvoid=AfxBeginThread(threadAutoAvoidMon, (LPVOID)this);
		}
		if (pButton != nullptr)
		{
			pButton->SetWindowText(_T("停止挂机"));
		}
	}
	else
	{
		tflag_attack = false;
		WaitForSingleObject(m_threadAttack, 60000);
		m_threadAttack = NULL;
		if (pButton != nullptr)	pButton->SetWindowText(_T("开始挂机"));
		if (m_threadAutoAvoid !=NULL)
		{
			tflag_autoavoid = false;
			WaitForSingleObject(m_threadAutoAvoid ,60000);
			m_threadAutoAvoid = NULL;
		}
	}
}

//录制NPC的hook函数
_declspec(naked) void CallRecord()
{
	_asm pushad
	char* command;
	char path[MAX_PATH];
	char* roleName;
	char* dir;
	const char* data;
	dir = shareCli.m_pSMAllData->currDir;
	strcpy_s(path, dir);
	data = "script\\";
	strcat_s(path, data);
	roleName = r.m_roleproperty.Object.pName;
	strcat_s(path, roleName);
	data = ".lua";
	strcat_s(path, data);
	command = (char*)hook_npc_cmd.EAX;
	tools::getInstance()->write2file_c(path, command,hook_npc_cmd.EDX);
	_asm  popad
	_asm ret
}

bool rec_flag = true;
//hook录制NPC
void CTestDlg::OnBnClickedBtnRecnpc()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_BTN_RECNPC);
	if (rec_flag)
	{
		std::string scriptpath = (std::string)shareCli.m_pSMAllData->currDir + "script";
		CreateDirectory(scriptpath.c_str(), NULL);// 创建 script 文件夹（如果不存在）
		if (hook_npc_cmd.hookReg(0xCAC543, 10, &CallRecord))
		{
			rec_flag = false;
			std::cout << "开始录制NPC对话，存储到/script/(角色名字).lua文件中" << std::endl;
			//添加按钮修改名字的代码
			if (pButton != nullptr)
			{
				pButton->SetWindowText(_T("停止录制"));
			}
		}

	}
	else
	{
		rec_flag = true;
		hook_npc_cmd.Unhook();
		if (pButton != nullptr)
		{
			pButton->SetWindowText(_T("录制NPC"));
			std::cout << "停止录制NPC对话" << std::endl;
		}
	}
}


//lua脚本测试
void CTestDlg::OnBnClickedBtnLuatst()
{
	
	if (m_EditLuaPath.IsEmpty())
	{
		MessageBox(_T("请选择Lua文件！"), _T("错误"), MB_ICONERROR);
		return;
	}
	std::cout<<"test!!!!!!!"<<std::endl;
	// 准备参数
	std::string scriptPath = m_EditLuaPath.GetString();
   // 定义错误回调函数
	auto errorCallback = [](const std::string& errorMessage) {
		std::cerr << "Error in Lua script: " << errorMessage << std::endl;
		};
	auto L= g_mgr.get()->L;
	luaStopFlag.store(false);//重置停止标志
	// 更新 Lua 状态中的全局变量 luaStopFlag
	if (L) {
		lua_pushboolean(L, luaStopFlag.load());
		lua_setglobal(L, "luaStopFlag");
	}
	// Lua全局变量 luaStopFlag  已在智能指针初始化后注册 用于控制脚本的停止
	// 使用 lambda 表达式调用成员函数
	std::thread scriptThread([this,L,scriptPath, errorCallback]() {this->RunLuaScriptInThread(LPVOID(this), L, scriptPath, errorCallback); });
	scriptThread.detach();  // 分离线程，使其独立运行
	// 主线程可以继续执行其他任务
	GetDlgItem(IDC_BTN_LUATST)->EnableWindow(FALSE); // 禁用按钮并设置为灰色
}

//停止lua脚本
void CTestDlg::OnBnClickedButton6()
{
	// 设置停止标志
	luaStopFlag.store(true);
	// 更新 Lua 状态中的全局变量 stopScript
	if (g_mgr.get()->L) {
		lua_pushboolean(g_mgr.get()->L, luaStopFlag.load());
		lua_setglobal(g_mgr.get()->L, "luaStopFlag");
	}

}

BOOL CTestDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return true;
}

//处理包裹
void CTestDlg::OnBnClickedBtnBagproc()
{
	// TODO: 在此添加控件通知处理程序代码
	CButton* pButton = (CButton*)GetDlgItem(IDC_BTN_BAGPROC);
	if (m_threadBagProcess==NULL)
	{
		tflag_processBag = true;
		m_threadBagProcess = AfxBeginThread(threadBagPocess, (LPVOID)this);
		if (pButton != nullptr)pButton->SetWindowText(_T("关整背包"));
	}
	else
	{
		tflag_processBag = false;
		if (pButton != nullptr)pButton->SetWindowText(_T("开整背包"));
		WaitForSingleObject(m_threadBagProcess, 60000);
		m_threadBagProcess = NULL;
	}
}

//浏览选择Lua脚本文件
void CTestDlg::OnBnClickedBtnChooseLuaFile()
{
	// 设置文件过滤器
	CString strFilter = _T("Lua Files (*.lua)|*.lua|All Files (*.*)|*.*||");

	// 创建文件对话框
	CFileDialog fileDlg(TRUE, _T("lua"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, strFilter);

	if (fileDlg.DoModal() == IDOK)
	{
		m_EditLuaPath = fileDlg.GetPathName();
		UpdateData(FALSE); // 更新编辑框中的内容
	}
	else
		return;

}

//重设edit颜色
HBRUSH CTestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

// 设置 Edit 控件的文本颜色
	//if (pWnd->GetDlgCtrlID() == IDC_EDIT2) // IDC_EDIT1 是 Edit 控件的 ID
	//{
	//	pDC->SetTextColor(RGB(147, 112, 219)); // 设置文本颜色为蓝紫色
	//	pDC->SetBkMode(TRANSPARENT);       // 设置背景为透明
	//}

	return hbr;
}
