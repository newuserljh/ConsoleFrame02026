// CGameConsole.cpp: 实现文件
//
#include "stdafx.h"
#include "Console.h"
#include "afxdialogex.h"
#include "CGameConsole.h"
#include <sstream>
#include <thread>
#include <windows.h>
#include "../Common/utils.h"
#include <tlhelp32.h>
// CGameConsole 对话框

IMPLEMENT_DYNAMIC(CGameConsole, CDialogEx)

CGameConsole::CGameConsole(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_GAMECONSLE, pParent)
{

}

CGameConsole::~CGameConsole()
{
}

void CGameConsole::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCONSOLE, m_listConsole);
}


BEGIN_MESSAGE_MAP(CGameConsole, CDialogEx)
    ON_NOTIFY(NM_RCLICK, IDC_LISTCONSOLE, &CGameConsole::OnNMRClickListconsole)
    ON_COMMAND(ID_SHOWGAME, OnShowGame)
    ON_COMMAND(ID_INJECT, OnInjectDll)
    ON_COMMAND(ID_UNINSTALL, OnUnIstallDll)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CGameConsole 消息处理程序
bool CGameConsole::initTables()
{
	//设置风格

	DWORD dwStyle = m_listConsole.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;// 选中某行使整行高亮（只适用与report 风格的listctrl ） 
	dwStyle |= LVS_EX_GRIDLINES;// 网格线（只适用与report 风格的listctrl ） 
	dwStyle |= LVS_EX_CHECKBOXES;//item 前生成checkbox 控件 
	m_listConsole.SetExtendedStyle(dwStyle); // 设置扩展风格 

	//初始化表头
	int i = 0;
	m_listConsole.InsertColumn(i++, "窗口句柄", LVCFMT_LEFT, 80);
	m_listConsole.InsertColumn(i++, "窗口标题", LVCFMT_LEFT, 80);
	m_listConsole.InsertColumn(i++, "进程ID", LVCFMT_LEFT, 80);
	m_listConsole.InsertColumn(i++, "线程ID", LVCFMT_LEFT, 80);

	//插入MORE_OPEN_NUM行

	for (int i = 0; i < MORE_OPEN_NUMBER; i++)
	{
		m_listConsole.InsertItem(i, "");
	}

	return true;
}
struct WindowInfo {
    HWND hWnd;
    DWORD dwProcessId;
    DWORD dwThreadId;
    TCHAR strTitle[256];
    PROCESS_INFORMATION pi;  // 完整的进程信息结构体
};
std::vector<WindowInfo> g_WindowList;

// 获取指定进程的主线程ID和句柄
bool GetMainThreadInfo(DWORD dwProcessId, DWORD& dwThreadId, HANDLE& hThread) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) return false;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == dwProcessId) {
                dwThreadId = te32.th32ThreadID;
                hThread = OpenThread(THREAD_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwThreadId);
                break; // 默认取第一个线程作为主线程
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }
    CloseHandle(hThreadSnap);
    return (hThread != NULL);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256] = { 0 };
    ::GetClassName(hwnd, szClassName, 255);

    if (_tcsicmp(szClassName, _T("WOLIICLIENT")) == 0) {
        WindowInfo info;
        info.hWnd = hwnd;

        // 获取窗口标题
        ::GetWindowText(hwnd, info.strTitle, 255);

        // 获取进程ID和线程ID
        info.dwThreadId = ::GetWindowThreadProcessId(hwnd, &info.dwProcessId);


        // 填充PROCESS_INFORMATION结构体
        info.pi.dwProcessId = info.dwProcessId;
        info.pi.dwThreadId = info.dwThreadId;

        // 打开进程句柄（需管理员权限获取完整权限）
        info.pi.hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE,
            FALSE,
            info.dwProcessId
        );

        // 获取并填充线程句柄
        if (!GetMainThreadInfo(info.dwProcessId, info.pi.dwThreadId, info.pi.hThread)) {
            info.pi.dwThreadId = 0; // 标记获取失败
            info.pi.hThread = NULL;
        }

        g_WindowList.push_back(info);
    }
    return TRUE;
}

void FindTargetWindows()
{
    g_WindowList.clear();
    ::EnumWindows(EnumWindowsProc, 0);
}

void CGameConsole::updateDate()
{
    m_listConsole.DeleteAllItems();
    FindTargetWindows();

    for (int i = 0; i < g_WindowList.size(); i++) {
        const WindowInfo& info = g_WindowList[i];

        CString strHandle;
        strHandle.Format(_T("0x%08X"), info.hWnd);

        CString strProcessId;
        strProcessId.Format(_T("%d"), info.dwProcessId);

        // 新增：显示进程信息
        CString strThreadId;
        strThreadId.Format(_T("%d"), info.pi.dwThreadId);

        int nIndex = m_listConsole.InsertItem(i, strHandle);
        m_listConsole.SetItemText(nIndex, 1, info.strTitle);
        m_listConsole.SetItemText(nIndex, 2, strProcessId);
        m_listConsole.SetItemText(nIndex, 3, strThreadId);
    }
}

BOOL CGameConsole::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	if (!initTables())
		return FALSE;
    stopThread.store(false);  // 重置停止标志
    auto tthread = std::thread(&CGameConsole::threadCallBack, this);
    tthread.detach(); //分离线程 后台运行 不会阻塞主线程
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CGameConsole::threadCallBack()
{
    //刷新的线程回调函数
    while (!stopThread.load())// 如果收到停止信号，则提前退出循环
    {
        updateDate();
        //TRACE("111\n");
        Sleep(20000);
    }
}
void CGameConsole::OnNMRClickListconsole(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    CPoint point;
    GetCursorPos(&point);  // 获取鼠标位置
    CMenu menu;
    menu.LoadMenu(IDR_MENU1);
    CMenu* pSubMenu = menu.GetSubMenu(0);
    pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
        point.x, point.y, this);
    menu.DestroyMenu();
    *pResult = 0;
}

void CGameConsole::OnShowGame()
{

    int nItem = m_listConsole.GetNextItem(-1, LVNI_SELECTED);
    if(nItem != -1)
    {
        HWND g_TargetHwnd = g_WindowList[nItem].hWnd;
        ::ShowWindow(g_TargetHwnd, SW_SHOWNORMAL);
        ::BringWindowToTop(g_TargetHwnd); // 置顶[5]
    }
}

void CGameConsole::OnInjectDll()
{
    int nItem = m_listConsole.GetNextItem(-1, LVNI_SELECTED);
    if (nItem != -1)
    {
        std::string dllPath = std::string(tools::getInstance()->GetCurrDir().c_str()) + "TestDll.dll";
        //AfxMessageBox(dllPath.c_str());
       // CString s;
        //PROCESS_INFORMATION pi = g_WindowList[nItem].pi;
      //  s.Format("%d,%d,%d,%d,%d",pi.dwProcessId,pi.dwThreadId,pi.hProcess,pi.hThread,pi.hProcess);
        //AfxMessageBox(s);
      tools::getInstance()->InjectDLL( g_WindowList[nItem].dwProcessId, tools::getInstance()->char2wchar(dllPath.c_str()));
    }
}


void CGameConsole::OnUnIstallDll()
{
    int nItem = m_listConsole.GetNextItem(-1, LVNI_SELECTED);
    if (nItem != -1)
    {
        std::string dllName = std::string("TestDll.dll");
        //AfxMessageBox(dllPath.c_str());
       // CString s;
        //PROCESS_INFORMATION pi = g_WindowList[nItem].pi;
      //  s.Format("%d,%d,%d,%d,%d",pi.dwProcessId,pi.dwThreadId,pi.hProcess,pi.hThread,pi.hProcess);
        //AfxMessageBox(s);
        tools::getInstance()->ForceUnloadDLL(g_WindowList[nItem].dwProcessId, L"TestDll.dll");
    }
}

void CGameConsole::OnDestroy()
{
    stopThread.store(true);  // 设置刷新停止标志
    CDialogEx::OnDestroy();

    // TODO: 在此处添加消息处理程序代码
}
