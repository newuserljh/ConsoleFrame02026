#pragma once
#include "../GameData/role.h"
#include "resource.h" 
//#include "lua.hpp"
//#include "LuaBridge/LuaBridge.h"
#include "lua_interface.h"

// CTestDlg 对话框

class CTestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTestDlg)

public:
	CTestDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTestDlg();

// 对话框数据
	enum { IDD = IDD_DLGTEST };



private:
	std::atomic<bool> luaStopFlag{ false };
	static void RunLuaScriptInThread(LPVOID p,lua_State* L, const std::string& scriptPath, std::function<void(const std::string&)> errorCallback);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton4();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChkTeam();
	CEdit m_edit2;
	bool Set_Skill();
	bool Load_coordinate();
	bool initVariable();
	bool init_team();
	/*打怪技能设置*/
	DWORD s_ID ;
	CWinThread* m_threadAttack;
	bool tflag_attack;
	static UINT __cdecl threadAttack(LPVOID lparam);
	std::vector<std::string> attack_monlist;

	/*寻路打怪坐标*/
	std::vector<MapXY> map_xy;
	size_t i_map;//寻路点下标
	CWinThread* m_threadGoto;
	bool tflag_goto;
	static UINT __cdecl threadGoto(LPVOID lparam);
	/*捡物设置*/
	std::vector<std::string> pick_goods_list;
	CWinThread* m_threadPickup;
	bool tflag_pickup;
	static UINT __cdecl threadPickup(LPVOID lparam);

	bool tflag_processBag;
	CWinThread* m_threadBagProcess;
	static UINT __cdecl CTestDlg::threadBagPocess(LPVOID lparam);

	/*组队设置*/
	CButton* pBtn;//队伍复选框
	std::vector<std::string> team_list;
	void  MakeTeam();
	int m_team_check_id;

	void  RoleIsDeath(void);//死亡处理

	//智能闪避变量
	bool tflag_autoavoid;
	CWinThread* m_threadAutoAvoid;
	static UINT __cdecl threadAutoAvoidMon(LPVOID p);

	CCriticalSection g_criticalsection; //边界锁，线程间同步用的。

	void AutoRecvGoods(void);//回收将军经验，逆魔装备元宝
	void AutoAvoidMonsters();//智能闪避

	void AutoReturnToCity(); // 回城处理函数声明
	
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnGj();
	afx_msg void OnBnClickedBtnRecnpc();


protected:

public:
	afx_msg void OnBnClickedBtnLuatst();
	lua_interface m_luaInterface; // 添加 lua_interface 成员变量
	//成员
public:
	DWORD mPid;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnBagproc();
	afx_msg void OnBnClickedButton6();
};
