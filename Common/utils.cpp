#include "utils.h"
#include <TlHelp32.h>

tools* tools::m_pInstance = nullptr;
std::mutex tools::m_mutex;
/*
函数功能：
单例模式指针获取
返回值：this指针，指向工具类
*/
tools* tools::getInstance()
{
	if (m_pInstance == nullptr)
	{
		m_mutex.lock();
		if (nullptr == m_pInstance)
		{
			m_pInstance = new tools;
		}
		m_mutex.unlock();
	}
	return m_pInstance;
}

tools::~tools()
{

}

tools::tools()
{

}

tools::tools(const tools&)
{

}

tools& tools::operator=(const tools&)
{
	return *this;
}


/*
函数功能：打印调试信息
参数1：数据格式
参数2：具体可变的参数
返回值：无
*/
void tools::message(char* fmt, ...)
{
	__try{
		char szbufFormat[0x1000];//字符串缓冲区，为vsprintf_s准备
		char szBufFormat_Game[0x1000] = "HXL:";//多8字节是为了加Game这个前缀
		va_list argList;
		va_start(argList, fmt);//参数列表初始化
		vsprintf_s(szbufFormat, fmt, argList);
		strcat_s(szBufFormat_Game, szbufFormat);//追加字符串
		OutputDebugString(szBufFormat_Game);
		va_end(argList);//清理工作
	}
	__except (1)
	{
		return;
	}
}
/*
函数功能：提升权限
返回值：BOOL 
*/
bool tools::improveProcPriv()
{
	HANDLE token;
	//提升权限
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
	{
		return false;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		return false;
	}
	CloseHandle(token);
	return true;
}

/*
函数功能：查找特征码
参数1：开始地址
参数2：地址大小
参数3：特征码
参数4：特征码掩码
返回值：特征码的开始地址
*/
DWORD tools::findPattern(DWORD startAddress, DWORD fileSize, unsigned char* pattern, char mask[])
{
	DWORD pos = 0;
	int searchLen = strlen(mask) - 1;
	//从内存内逐个字节进行比较
	for (DWORD retAddress = startAddress; retAddress < startAddress + fileSize; retAddress++)
	{
		//判断当前地址是否有效
		if (IsBadReadPtr((const void *)retAddress, 1))
		{
			pos = 0;
			continue;
		}
		if (*(PBYTE)retAddress == pattern[pos] || mask[pos] == '?')
		{
			if (mask[pos + 1] == '\0')
			{
				return (retAddress - searchLen);
			}

			pos++;
		}
		else
		{
			pos = 0;
		}
	}
	return NULL;
}
/*
函数功能：返回模块信息
参数：模块名字
返回值：模块信息
*/
MODULEINFO tools::getModuleInfo(const char* moduleName)
{
	MODULEINFO moudleInfo = { NULL };
	HMODULE hMoudle = GetModuleHandle(moduleName);
	if (hMoudle)
	{
		GetModuleInformation(GetCurrentProcess(), hMoudle, &moudleInfo, sizeof(MODULEINFO));
	}
	return moudleInfo;
}
/*
函数功能：记录日志文件
参数1：文件名
参数2：数据
参数3：打开模式
返回值：BOOL
*/
bool tools::log2file(std::string fileName, std::string data, std::ios_base::open_mode _model/*= std::ios::app*/)
{
	std::ofstream fout;
	fout.open(fileName, _model);
	if (!fout)
	{
		return false;
	}
	std::stringstream ss;
	time_t nowTime = time(NULL);
	ss << "time: " << nowTime<<" ";
	fout << ss.str() <<data << std::endl;
	fout.close();
	return true;
}

/*
函数功能：string写入文件+换行
参数1：文件名
参数2：数据
参数3：打开模式
返回值：BOOL
*/
bool tools::write2file(std::string fileName, std::string data, std::ios_base::open_mode _model/*= std::ios::app*/)
{
	std::ofstream fout;
	fout.open(fileName, _model);
	if (!fout)
	{
		return false;
	}
	fout << data << std::endl;
	fout.close();
	return true;
}

/*
函数功能：c方式写文件，用于录制NPC的ID 和对话内容
参数1：文件名
参数2：数据
参数3：NPC的ID
参数4：打开模式
返回值：BOOL
*/
bool tools::write2file_c(const char* fileName, const char* data, DWORD npc_id, const char* _model)
{
	FILE* fp;
	errno_t err = fopen_s(&fp, fileName, _model); // 使用 fopen_s 代替 fopen
	if (err != 0 || fp == NULL) {
		perror("Error opening file");
		return false;
	}
	fprintf(fp, "NPC ID: %x  %s\n", npc_id, data); // 将格式化的数据写入文件
	fclose(fp); // 关闭文件
	return true;
}


/*
函数功能：写内存字节集
参数1: 内存地址
参数2：字节集
参数3：尺寸
返回值：BOOL
*/
bool tools::write(DWORD _addr, const char* _data, int _size)
{
	if (!improveProcPriv())
	{
		return false;
	}
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)_addr, _size, PAGE_READWRITE, &oldProtect))
	{
		tools::getInstance()->message("erroe1\n");
		return false;
	}
	memcpy((void*)_addr, _data, _size);
	if (!VirtualProtect((void*)_addr, _size, oldProtect, &oldProtect))
	{
		tools::getInstance()->message("erroe1\n");
		return false;
	}
	return  true;
}
/*
函数功能：读内存字节集
参数1: 内存地址
参数2：字节集
参数3：尺寸
返回值：BOOL
*/
bool tools::read(DWORD _addr, const char* _data, int _size)
{
	unsigned long oldProtect = 0;
	if ( S_OK != VirtualProtect((void*)_addr, _size, PAGE_READONLY, &oldProtect))
	{
		return false;
	}
	memcpy((void*)_data, (void*)_addr, _size);
	if (S_OK !=VirtualProtect((void*)_addr, _size, oldProtect, 0))
	{
		return false;
	}
	return  true;
}

bool tools::byte2hex(BYTE src, char* desc)
{
	byte low = 0, high = 0;
	//取高地位 低位取余数
	low = src % 16;
	high = src / 16;

	if (high > 9)
	{
		//判断高位为A -F
		desc[0] = 'A' + high - 10;
	}
	else
	{
		desc[0] = '0' + high;
	}

	//对低位进行赋值
	if (low > 9)
	{
		desc[1] = 'A' + low - 10;
	}
	else
	{
		desc[1] = '0' + low;
	}
	desc[2] = ',';
	desc[3] = '\0';
	return true;
}

bool tools::lower2upper(char& ch)
{
	if (ch >= 'a' && ch <= 'z')
	{
		ch = ch + 'A' - 'a';
	}
	return true;
}

bool tools::hexstr2upper(char* src)
{
	DWORD ndLen = strlen(src);
	for (DWORD i = 0; i != ndLen; i++)
	{
		lower2upper(src[i]);
	}
	return true;
}

char tools::hex2byte(const char& high, const char& low)
{
	int result = 0;

	//如果高位为数字
	//计算方法就是   high - '0'
	if (high >= '0' && high <= '9')
	{
		result = high - '0';
	}
	else if (high >= 'A' && 'F')
	{
		//如果高位为字母
		//计算方法Wie  high - 'A' + 10
		result = high - 'A' + 10;
	}
	else
	{
		return -1;
	}
	//低位计算方法
	if (low >= '0' && low <= '9')
	{
		return result * 16 + low - '0';
	}
	else if (low >= 'A' && low <= 'F')
	{
		return result * 16 + low - 'A' + 10;
	}
	else
	{
		return -1;
	}
}

bool tools::bytes2hexstr(BYTE* src, DWORD ndSize, char* desc)
{

	//首先是将所有字符转化为大写字母
	__try{

		for (int i = 0; i != ndSize; i++)
		{
			byte2hex(src[i], &desc[i * 3]);
		}
	}
	__except (1)
	{

		tools::getInstance()->message("字节转换异常！\n");
	}
	//DWORD ndLen = strlen(src);

	return hexstr2upper(desc);
}

bool tools::hexstr2bytes(char* src, char* desc)
{
	hexstr2upper(src);
	DWORD ndLen = strlen(src);
	for (int i = 0; i != ndLen; i++)
	{
		desc[i] = hex2byte(src[i * 2], src[i * 2 + 1]);
	}
	return hexstr2upper(desc);
}

/*
函数功能：判断文件是否存在
参数1：文件名
返回值：bool
*/
bool tools::fileIsexist(std::string fileName)
{
	std::fstream ss;
	ss.open(fileName,std::ios::in);
	bool result;
	if (ss)
		result = true;
	else
		result = false;
	ss.close();
	return result;
	/* 下面是使用GetFileAttributesA 函数 Windows API函数，用于获取指定文件或目录的属性。
	    它返回文件属性的一个位掩码。
		INVALID_FILE_ATTRIBUTES：如果 GetFileAttributesA 返回这个值，表示文件或目录不存在。
        检查文件存在性：通过检查 fileAttributes 是否等于 INVALID_FILE_ATTRIBUTES，并确认它不是一个目录（使用 FILE_ATTRIBUTE_DIRECTORY 标志），来判断文件是否存在。
	//DWORD fileAttributes = GetFileAttributesA(fileName.c_str());*/
	//return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}


/*
函数功能：按行读取文件
参数1：文件名
返回值：存储数据的vector
*/
std::vector<std::string> tools::ReadTxt(std::string file_Path/*文件路径*/)
{
	std::fstream f(file_Path);
	std::vector<std::string> temp;
	std::string line;
	if (!f.is_open())
	{
		tools::getInstance()->message("文件打开失败！\n");
	}
	while (!f.eof())//会自动把换行符去掉
	{
		std::getline(f, line);
		if(line.size())temp.push_back(line); //空行不push
	}
	f.close();
	return temp;
}


/*
函数功能：分割整行字符串
参数1：字符串
返回值：存储数据的vector
*/
std::vector<std::string> tools::splitString(std::string str)
{
	std::regex delimiters("\\s+");
	std::vector<std::string> temp(std::sregex_token_iterator(str.begin(), str.end(), delimiters, -1), std::sregex_token_iterator());
	return temp;
}

/*
函数功能：获取当前exe或者dll的路径
参数1：字符串
返回值：当前exe所在路径
*/
std::string tools::GetCurrDir()
{	
	TCHAR szFull[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	::GetModuleFileName(NULL, szFull, sizeof(szFull) / sizeof(TCHAR));
	_splitpath_s(szFull, szDrive, sizeof(szDrive) / sizeof(TCHAR), szDir, sizeof(szDir) / sizeof(TCHAR), NULL, 0, NULL, 0);
	_tcscpy_s(szFull, szDrive);
	_tcscat_s(szFull, szDir);
	std::string strPath(szFull);
	return strPath;
}

/*
函数功能：取文件夹名字,取上一路径
参数1：路径字符串 " X:\\DIR1\\DIR2\\DIR3\\DIR4\\"
返回值：上一级目录 " X:\\DIR1\\DIR2\\DIR3\\"
*/
 std::string tools::getParentPath(std::string str)
{
	std::string::size_type idx = str.rfind('\\', str.length());
	std::string folder = str.substr(0, idx);
	idx = folder.rfind('\\', folder.length());
	std::string folder1 = folder.substr(0, idx + 1);
	return folder1;
}


 /*
函数功能： char* 转 wchar_t*
参数1：
返回值：
*/
 wchar_t* tools::char2wchar(const char* cchar)
 {
	 wchar_t* m_wchar;
	 int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	 m_wchar = new wchar_t[len + 1];
	 MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	 m_wchar[len] = '\0';
	 return m_wchar;
 }


 /*
函数功能： wchar_t* 转 char_t*
参数1：
返回值：
*/
 char* tools::wchar2char(const wchar_t* wchar)
 {
	 char* m_char;
	 int len = WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), NULL, 0, NULL, NULL);
	 m_char = new char[len + 1];
	 WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), m_char, len, NULL, NULL);
	 m_char[len] = '\0';
	 return m_char;
 }

 /*
函数功能： 释放资源到指定目录
参数1：资源ID
参数2：释放的完整路径（包含目录+文件名。如"C:\windwos\test.dll"）
参数3：释放资源的类型，在资源文件中可以看到
参数4：是否设置释放的文件属性为系统和隐藏，bool型，默认false
返回值：
*/
 bool tools::ReleaseResource(int resourceId, const std::string& outputPath, 
	 const std::string& resourceType, bool hiddenSystem) 
 {
	 HMODULE hModule =  GetModuleHandle(NULL);
	 HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceId), resourceType.c_str());
	 if (hResource == NULL) {
		 return false;
	 }

	 HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
	 if (hLoadedResource == NULL) {
		 return false;
	 }

	 LPVOID pLockedResource = LockResource(hLoadedResource);
	 DWORD dwResourceSize = SizeofResource(hModule, hResource);
	 if (pLockedResource == NULL || dwResourceSize == 0) {
		 return false;
	 }

	 std::ofstream outFile(outputPath, std::ios::binary);
	 outFile.write((const char*)pLockedResource, dwResourceSize);
	 outFile.close();

	 if (hiddenSystem) {
		 // 设置文件为隐藏和系统文件
		 SetFileAttributes(outputPath.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
	 }

	 return true;
 }

 /*
函数功能： 注入 DLL 到目标进程中
参数1：注入目标进程PID
参数2：dll路径
返回值：
*/
 bool tools::InjectDLL(DWORD pid, const wchar_t* dllPath) {
	 // 1. 获取目标进程句柄
	 HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	 if (!hProcess) {
		 std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
		 return false;
	 }

	 // 2. 在目标进程分配内存
	 SIZE_T pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);
	 LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
	 if (!remoteMem) {
		 CloseHandle(hProcess);
		 std::cerr << "VirtualAllocEx failed: " << GetLastError() << std::endl;
		 return false;
	 }

	 // 3. 写入DLL路径
	 if (!WriteProcessMemory(hProcess, remoteMem, dllPath, pathSize, NULL)) {
		 VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
		 CloseHandle(hProcess);
		 std::cerr << "WriteProcessMemory failed: " << GetLastError() << std::endl;
		 return false;
	 }

	 // 4. 创建远程线程调用LoadLibrary
	 LPTHREAD_START_ROUTINE loadLibAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryW");
	 HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, loadLibAddr, remoteMem, 0, NULL);
	 if (!hThread) {
		 VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
		 CloseHandle(hProcess);
		 std::cerr << "CreateRemoteThread failed: " << GetLastError() << std::endl;
		 return false;
	 }

	 // 5. 等待线程执行完成
	 WaitForSingleObject(hThread, INFINITE);

	 // 6. 清理资源
	 DWORD exitCode;
	 GetExitCodeThread(hThread, &exitCode);
	 VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
	 CloseHandle(hThread);
	 CloseHandle(hProcess);

	 return (exitCode != 0); // 返回是否成功加载DLL
 }

 /*
函数功能： 查找目标DLL的模块句柄
参数1：进程pid
参数2：dll名字
返回值：无
*/
 HMODULE tools::FindRemoteModule(DWORD pid, const wchar_t* dllName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

    MODULEENTRY32W me32 = { sizeof(me32) };
    HMODULE hModule = NULL;
    if (Module32FirstW(hSnapshot, &me32)) {
        do {
            if (_wcsicmp(me32.szModule, dllName) == 0) {
                hModule = me32.hModule;
                break;
            }
        } while (Module32NextW(hSnapshot, &me32));
    }
    CloseHandle(hSnapshot);
    return hModule;
}

 /*
函数功能： 若DLL支持自定义卸载协议，可远程调用清理函数
参数1：dll模块pid
参数2：dll名字
返回值：无
*/
 void tools::SafeCleanup(DWORD pid, const wchar_t* dllName) {
	 HMODULE hModule = FindRemoteModule(pid, dllName);
	 HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	 LPVOID pCleanFunc = GetProcAddress(hModule, "DllStop");
	 if (pCleanFunc) {
		 HANDLE hThread = CreateRemoteThread(
			 hProcess, NULL, 0,
			 (LPTHREAD_START_ROUTINE)pCleanFunc, NULL, 0, NULL);
		 WaitForSingleObject(hThread, 15000); // 等待清理完成
		 CloseHandle(hThread);
	 }
 }

 /*
函数功能： 强制卸载DLL
参数1：dll模块pid
参数2：dll名字
返回值：bool
*/
 bool tools::ForceUnloadDLL(DWORD pid, const wchar_t* dllName) {
	 HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	 if (!hProcess) return false;

	 HMODULE hModule = FindRemoteModule(pid, dllName);
	 if (!hModule) {
		 CloseHandle(hProcess);
		 return false;
	 }

	 // 创建远程线程调用FreeLibrary
	 LPTHREAD_START_ROUTINE pFreeLib = (LPTHREAD_START_ROUTINE)
		 GetProcAddress(GetModuleHandle("kernel32.dll"), "FreeLibrary");
	 HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pFreeLib, hModule, 0, NULL);

	 if (hThread) {
		 WaitForSingleObject(hThread, INFINITE);
		 CloseHandle(hThread);
	 }
	 CloseHandle(hProcess);
	 return (hThread != NULL);
 }

 /*
函数功能： 劫持 EIP（指令指针寄存器）来注入 DLL 到目标进程中
参数1：dll路径
参数2：注入目标进程的的进程信息 PROCESS_INFORMATION 结构体
返回值：
*/
 bool tools::eipinjectDll(WCHAR* dllname, PROCESS_INFORMATION pi) {

	 SuspendThread(pi.hThread); //挂起线程 
	 CONTEXT ct = { 0 };
	 ct.ContextFlags = CONTEXT_CONTROL;
	 GetThreadContext(pi.hThread, &ct); //获取，保存线程寄存器相关 

	 DWORD dwSize = sizeof(WCHAR) * 1024; //0-0x100 写代码 之后写数据 
	 BYTE* pProcessMem = (BYTE*)::VirtualAllocEx(pi.hProcess, NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	 if (NULL == pProcessMem)return  false;
	 DWORD dwWrited = 0;
	 if (!::WriteProcessMemory(pi.hProcess, (pProcessMem + 0x100), (LPVOID)dllname, //先把路径（数据）写到内存里，从0x100开始 
		 (wcslen(dllname) + 1) * sizeof(WCHAR), &dwWrited))return false;

	 // 获取 LoadLibraryW 函数地址
	 FARPROC pLoadLibraryW = (FARPROC)::GetProcAddress(::GetModuleHandle("Kernel32"), "LoadLibraryW");
	 if (NULL == pLoadLibraryW)return false;

	 // 构造 ShellCode
	 BYTE ShellCode[32] = { 0 };
	 DWORD* pdwAddr = NULL;

	 ShellCode[0] = 0x60; // pushad 
	 ShellCode[1] = 0x9c; // pushfd 
	 ShellCode[2] = 0x68; // push 
	 pdwAddr = (DWORD*)&ShellCode[3]; // ShellCode[3/4/5/6] 
	 *pdwAddr = (DWORD)(pProcessMem + 0x100);
	 ShellCode[7] = 0xe8;//call 
	 pdwAddr = (DWORD*)&ShellCode[8]; // ShellCode[8/9/10/11] 
	 *pdwAddr = (DWORD)pLoadLibraryW - ((DWORD)(pProcessMem + 7) + 5); // 因为直接call地址了，所以对应机器码需要转换，计算VA 
	 ShellCode[12] = 0x9d; // popfd 
	 ShellCode[13] = 0x61; // popad 
	 ShellCode[14] = 0xe9; // jmp 
	 pdwAddr = (DWORD*)&ShellCode[15]; // ShellCode[15/16/17/18] 
	 *pdwAddr = ct.Eip - ((DWORD)(pProcessMem + 14) + 5); //因为直接jmp地址了，所以对应机器码需要转换，计算VA 

	 // 将 ShellCode 写入目标进程内存
	 if (!::WriteProcessMemory(pi.hProcess, pProcessMem, ShellCode, sizeof(ShellCode), &dwWrited))return false;

	 // 修改 EIP 指向 ShellCode
	 ct.Eip = (DWORD)pProcessMem;
	 ::SetThreadContext(pi.hThread, &ct);
	 ::ResumeThread(pi.hThread);
	 return true;
 }


 /*
函数功能： 创建目标进程
参数1：可执行文件名字+路径
参数2：进程执行目录
参数3：进程结构体
返回值：
*/
 bool tools::exeload(const std::string& filename, const std::string& excutedirectory, PROCESS_INFORMATION& pi)
 {
	 STARTUPINFO si; // 启动信息
	 memset(&si, 0, sizeof(STARTUPINFO));
	 si.cb = sizeof(STARTUPINFO);
	 si.dwFlags = STARTF_USESHOWWINDOW;
	 si.wShowWindow = SW_SHOW;

	 if (!CreateProcess(
		 NULL, // 应用程序名称
		 const_cast<char*>(filename.c_str()), // 命令行参数
		 NULL, // 进程安全属性
		 NULL, // 线程安全属性
		 FALSE, // 是否继承句柄
		 0, // 创建标志
		 NULL, // 新进程的环境变量
		 const_cast<char*>(excutedirectory.c_str()), // 当前目录
		 &si, // 启动信息
		 &pi // 进程信息
	 )) return false;

	 return true;
 }


 // 解析配置文件并返回一个 map，其中键是列表名，值是列表内容
 std::unordered_map<std::string, std::unordered_map<std::string, std::string>> tools::parseIniFile(const std::string& filename)
 {
	 std::ifstream file(filename);
	 if (!file.is_open()) {
		 throw std::runtime_error("Could not open file: " + filename);
	 }
	 std::unordered_map<std::string, std::unordered_map<std::string, std::string>> config;
	 std::string line;
	 std::string currentSection;

	 while (std::getline(file, line)) {
		 // Trim whitespace
		 line.erase(0, line.find_first_not_of(" \t"));
		 line.erase(line.find_last_not_of(" \t") + 1);

		 if (line.empty() || line[0] == ';') continue; // Skip empty lines and comments

		 if (line[0] == '[' && line[line.size() - 1] == ']') {
			 // Section header
			 currentSection = line.substr(1, line.size() - 2);
		 }
		 else {
			 size_t delimiterPos = line.find('=');
			 if (delimiterPos != std::string::npos) {
				 std::string key = line.substr(0, delimiterPos);
				 std::string value = line.substr(delimiterPos + 1);
				 config[currentSection][key] = value;
			 }
		 }
	 }
	 return config;
 }

 void tools::copyFile(const std::string& srcPath, const std::string& destPath) {
	 std::ifstream src(srcPath, std::ios::binary);
	 std::ofstream dest(destPath, std::ios::binary);

	 if (!src) {
		 std::cerr << "无法打开源文件: " << srcPath << std::endl;
		 return;
	 }
	 if (!dest) {
		 std::cerr << "无法创建目标文件: " << destPath << std::endl;
		 return;
	 }

	 // 使用缓冲区进行高效复制
	 dest << src.rdbuf();
 }