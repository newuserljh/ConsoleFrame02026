// TestDll.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "TestDll.h"
#include "TestDlg.h"
#include <thread>
#include <memory>
#include <tlhelp32.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CTestDlg* pDlg = NULL;
//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CTestDllApp

BEGIN_MESSAGE_MAP(CTestDllApp, CWinApp)
END_MESSAGE_MAP()


// CTestDllApp 构造

CTestDllApp::CTestDllApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CTestDllApp 对象

CTestDllApp theApp;


// CTestDllApp 初始化

BOOL CTestDllApp::InitInstance()
{
//#ifdef _DEBUG
//	// 在 Debug 模式下创建并重定向控制台
//	if (AllocConsole())
//	{
//		// 重定向标准输入/输出到控制台
//		freopen("CONIN$", "r", stdin);
//		freopen("CONOUT$", "w", stdout);
//		freopen("CONOUT$", "w", stderr);
//
//		// 设置控制台标题
//		SetConsoleTitle("Debug Console");
//
//		// 输出测试信息到控制台
//		std::cerr << "Debug console error." << std::endl;
//		// 输出测试信息到控制台
//		std::cout << "Debug console initialized." << std::endl;
//	}
//#else
//// 在 Release 模式下不创建控制台
//FreeConsole();
//#endif
	CWinApp::InitInstance();

	return init();
}
HANDLE dllThread = NULL;
bool CTestDllApp::init()
{
	//自定义初始化函数
	//这种注入方式，线程会随着this dll卸载退出线程！
	dllThread =::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(threadFunc), NULL, NULL, NULL);
	return true;
}

//LRESULT CALLBACK KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//	if ((wParam == VK_HOME) && ((lParam & 0x40000000) != 0))
//	{
//		if(pDlg->IsWindowVisible())pDlg->ShowWindow(SW_HIDE);
//		else pDlg->ShowWindow(SW_SHOW); //SW_SHOW
//
//	}
//	return CallNextHookEx(0, nCode, wParam, lParam);
//}

DWORD GetMainThreadId(DWORD processId = 0)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (processId == 0)
		processId = GetCurrentProcessId();

	DWORD threadId = 0;
	THREADENTRY32 te32 = { sizeof(te32) };
	HANDLE threadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (Thread32First(threadSnap, &te32))
	{
		do
		{
			if (processId == te32.th32OwnerProcessID)
			{
				threadId = te32.th32ThreadID;
				break;
			}
		} while (Thread32Next(threadSnap, &te32));
	}
	return threadId;
}

//设置子窗口透明
void SetTransparent(HWND hwnd, BYTE alpha)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// 设置窗口为分层窗口
	LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if (!(exStyle & WS_EX_LAYERED))
	{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
	}

	// 设置透明度
	SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

// 该函数接收窗口类名和目标PID，并返回匹配的窗口句柄
HWND FindWindowByClassAndPid(const TCHAR* className, DWORD targetPid) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// 使用FindWindow查找具有指定类名的第一个顶级窗口
	HWND hwnd = FindWindow(className, NULL);

	while (hwnd != NULL) {
		DWORD pid;
		// 获取该窗口对应的进程ID
		GetWindowThreadProcessId(hwnd, &pid);

		if (pid == targetPid) {
			// 如果找到的窗口PID与目标PID匹配，则返回该窗口句柄
			return hwnd;
		}

		// 查找下一个相同类名的窗口（如果存在）
		hwnd = FindWindowEx(NULL, hwnd, className, NULL);
	}

	// 如果没有找到匹配的窗口，返回NULL
	return NULL;
}


void EmbedIntoGameWindow(HWND dlgHwnd, HWND gameHwnd) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// Set the dialog as a child of the game's main window
	if (!::SetParent(dlgHwnd, gameHwnd)) {
		OutputDebugString("SetParent failed\n");
		return;
	}

	// Modify the window style to remove popup and add child styles
	LONG_PTR style = ::GetWindowLongPtr(dlgHwnd, GWL_STYLE);
	style &= ~WS_POPUP; // Remove popup style
	style |= WS_CHILD;  // Add child style
	if (!::SetWindowLongPtr(dlgHwnd, GWL_STYLE, style)) {
		OutputDebugString("SetWindowLong failed\n");
		return;
	}

	 //Optionally modify extended window styles
	LONG_PTR exStyle = ::GetWindowLongPtr(dlgHwnd, GWL_EXSTYLE);
	exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	if (!::SetWindowLongPtr(dlgHwnd, GWL_EXSTYLE, exStyle)) {
		OutputDebugString("SetWindowLongEx failed\n");
		return;
	}

	// Adjust the size and position of the dialog to fit within the game window
	RECT rc, rc_child;
	if (!::GetClientRect(gameHwnd, &rc)) {
		OutputDebugString("GetClientRect failed\n");
		return;
	}

	if (!::GetClientRect(dlgHwnd, &rc_child) ){
		OutputDebugString("Child GetClientRect failed\n");
		return;
	}

	if (!::MoveWindow(dlgHwnd, 0, 0, rc_child.right - rc_child.left, rc_child.bottom - rc_child.top, TRUE)) {
		OutputDebugString("MoveWindow failed\n");
		return;
	}

  // 设置透明度为 128 (100透明)
	SetTransparent(dlgHwnd, 128);

	OutputDebugString("Successfully embedded MFC dialog into game window\n");
}
//线程函数，用来创建函数窗口
void threadFunc()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//mhook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardProc, 0, GetMainThreadId());
	if (pDlg == NULL)
	{
		pDlg = new CTestDlg;
		if(!pDlg->Create(IDD_DLGTEST)) {
			OutputDebugString("MFC dialog creation failed\n");
			return;
		}
		pDlg->ShowWindow(SW_SHOW);
		OutputDebugString("MFC dialog created successfully\n");

		DWORD processId = GetCurrentProcessId();
		HWND hGameWnd = FindWindowByClassAndPid("WOLIICLIENT",processId);
		if (hGameWnd) {
			EmbedIntoGameWindow(pDlg->m_hWnd, hGameWnd);
		}
		else {
			OutputDebugString("Main window handle is invalid\n");
		}
	
	}
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))

	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return;
}

void CTestDllApp::DllStop()
{
	ExitInstance();
}


int CTestDllApp::ExitInstance()
{
	// TODO: 在此添加专用代码和/或调用基类
	  // 释放控制台
	//FreeConsole();
	pDlg->OnDestroy();
	if (dllThread)
	{
		TerminateThread(dllThread, 0);
		CloseHandle(dllThread);
	}
	//delete pDlg;
	return CWinApp::ExitInstance();
}
