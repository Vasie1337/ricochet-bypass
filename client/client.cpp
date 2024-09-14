#include <../.shared/shared.hpp>

DWORD FindProcessId(const std::wstring& processName)
{
	DWORD processId = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);
		if (Process32First(hSnapshot, &pe))
		{
			do
			{
				if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0)
				{
					processId = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe));
		}
		CloseHandle(hSnapshot);
	}
	return processId;
}

UINT64 GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName)
{
	UINT64 baseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnapshot, &me))
		{
			do
			{
				if (_wcsicmp(me.szModule, moduleName.c_str()) == 0)
				{
					baseAddress = (UINT64)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &me));
		}
		CloseHandle(hSnapshot);
	}
	return baseAddress;
}

int main() 
{
	DWORD processId = FindProcessId(L"cod22-cod.exe");
	if (processId == 0)
	{
		std::wcout << L"Process not found" << std::endl;
		return 1;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (hProcess == NULL)
	{
		std::wcout << L"Failed to open process" << std::endl;
		return 1;
	}

	std::wcout << L"Process opened" << std::endl;

	UINT64 baseAddress = GetModuleBaseAddress(processId, L"cod22-cod.exe");
	if (baseAddress == 0)
	{
		std::wcout << L"Failed to get module base address" << std::endl;
		return 1;
	}

	std::wcout << L"Module base address: " << std::hex << baseAddress << std::endl;

	while (true)
	{
		BYTE buffer[4];
		SIZE_T bytesRead;
		if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddress, buffer, sizeof(buffer), &bytesRead))
		{
			std::wcout << L"Failed to read memory" << std::endl;
			return 1;
		}

		std::wcout << L"Read " << bytesRead << L" bytes" << std::endl;

		for (int i = 0; i < 4; i++)
		{
			std::wcout << std::hex << (int)buffer[i] << L" ";
		}
		std::wcout << std::endl;

		Sleep(3000);
	}

	CloseHandle(hProcess);

	return 0;
}