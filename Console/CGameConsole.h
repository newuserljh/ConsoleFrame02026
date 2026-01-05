#pragma once
#include "afxdialogex.h"
#include <string>
#include <atomic>
#include "../Common/SMstruct.h"
#include "../Common/shareMemorySer.h"
// CGameConsole 对话框

class CGameConsole : public CDialogEx
{
	DECLARE_DYNAMIC(CGameConsole)

public:
	CGameConsole(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CGameConsole();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_GAMECONSLE };
#endif

private:
	std::atomic<bool> stopThread{ false };  // 用于通知线程停止
	void threadCallBack();//刷新ListCtrl
	CWinThread* m_pThread_login = nullptr; // 保存线程指针
	static UINT __cdecl ThreadLogin(LPVOID pParam);
	void updateDate();
	std::shared_ptr<shareMemorySer>m_shareMemSer;
public:
	CListCtrl m_listConsole;
	// 初始化表单信息
	bool initTables();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMRClickListconsole(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowGame();
	afx_msg void OnInjectDll();
	afx_msg void OnUnIstallDll();
	afx_msg void OnDestroy();
};



