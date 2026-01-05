#pragma once

#ifndef SECURITY_WIN32
#define  SECURITY_WIN32
#endif // !SECURITY_WIN32


#include <mutex>
#include <memory>
#include <stdio.h>
#include <windows.h>
#include <sstream>
#include <Psapi.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <regex>
#include <tchar.h>
#include "StringCovert.h"
#pragma comment(lib , "Psapi.lib")
/*
工具类
*/
class tools
{
public:
	static tools* getInstance();
	~tools();
	void message(char* fmt, ...);
	bool improveProcPriv();
	DWORD findPattern(unsigned long startAddrSize, DWORD fileSize, unsigned char* pattern, char mask[]);
	MODULEINFO getModuleInfo(const char* moduleName);
	bool log2file(std::string fileName, std::string data, std::ios_base::open_mode _model = std::ios::app);
	bool write2file(std::string fileName, std::string data, std::ios_base::open_mode _model= std::ios::app);
	bool write2file_c(const char* fileName, const char* data, DWORD npc_id, const char* _model = "a+");
	bool read(DWORD _addr, const char* _data, int _size);
	bool write(DWORD _addr, const char* _data, int _size);
	bool byte2hex(BYTE src,  char* desc);
	bool lower2upper(char& ch);
	bool hexstr2upper( char* src);
	char hex2byte(const char& ch1, const char& ch2);
	bool bytes2hexstr(BYTE* src, DWORD ndSize, char* desc); //字节数组到hex数组的转换 
	bool hexstr2bytes(char* src, char* desc); //hex字符串到字节数组的转换
	bool fileIsexist(std::string fileName);
	std::vector<std::string> ReadTxt(std::string file_Path/*文件路径*/);
	std::vector<std::string> splitString(std::string str);
	std::string GetCurrDir();
	std::string getParentPath(std::string str);
	wchar_t* char2wchar(const char* cchar);
	char* wchar2char(const wchar_t* cchar);
	bool ReleaseResource(int resourceId, const std::string& outputPath, const std::string& resourceType, bool hiddenSystem = false);
	bool InjectDLL(DWORD pid, const wchar_t* dllPath);
	HMODULE FindRemoteModule(DWORD pid, const wchar_t* dllName);
	void SafeCleanup(HANDLE hProcess, HMODULE hModule);
	bool ForceUnloadDLL(DWORD pid, const wchar_t* dllName);
	bool eipinjectDll(WCHAR* dllname, PROCESS_INFORMATION pi);
	static bool exeload(const std::string& filename, const std::string& excutedirectory, PROCESS_INFORMATION& pi);
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> parseIniFile(const std::string& filename);
	void copyFile(const std::string& srcPath, const std::string& destPath);
public:
	tools();
	tools(const tools&);
	tools& operator= (const tools&);
	static tools* m_pInstance;
	static std::mutex m_mutex;

};