#include <Windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <cstdio>

#pragma comment(lib, "psapi.lib")

DWORD
GetProcNameFromPid(DWORD procId, wchar_t *procName)
{
    HANDLE   hProc    = INVALID_HANDLE_VALUE;
    DWORD    dwError  = 0;
    HMODULE  hMod     = 0;
    DWORD    len      = MAX_PATH;
    wchar_t *temp     = NULL;
    DWORD    cbNeeded = 0;

    printf("Parent PID: %lu\n", procId);
    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procId);
    if (!hProc)
    {
        dwError = GetLastError();
        printf("OpenProcess: %lu (Likely have parent PID)\n", dwError);
        goto EXIT;
    }

    if (!EnumProcessModules(hProc, &hMod, sizeof(hMod), &cbNeeded))
    {
        dwError = GetLastError();
        printf("EnumProcess: %lu\n", dwError);
        goto EXIT;
    }

    if(!GetModuleBaseNameW(hProc, hMod, procName, MAX_PATH))
    {
        dwError = GetLastError();
        printf("GetModuleBaseName: %lu\n", dwError);
        goto EXIT;
    }

EXIT:
    return dwError;
}

DWORD
GetMainProcId(const wchar_t *procName)
{
    DWORD   procId                       = 0;
    HANDLE  hSnap                        = INVALID_HANDLE_VALUE;
    wchar_t parentProcName[MAX_PATH * 2] = { 0 };

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (INVALID_HANDLE_VALUE != hSnap)
    {
        PROCESSENTRY32 procEntry = { 0 };
        procEntry.dwSize         = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp((const wchar_t *)procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    printf("Found PID: %lu\n", procId);
                    if (!GetProcNameFromPid(procEntry.th32ParentProcessID, parentProcName))
                    {
                        if (!_wcsicmp(parentProcName, procName))
                        {
                            procId = procEntry.th32ParentProcessID;
                        }
                    }
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }

    CloseHandle(hSnap);
    return procId;
}

DWORD
GetProcId(const wchar_t *procName)
{
    DWORD  procId = 0;
    HANDLE hSnap  = INVALID_HANDLE_VALUE;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (INVALID_HANDLE_VALUE != hSnap)
    {
        PROCESSENTRY32 procEntry = { 0 };
        procEntry.dwSize         = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp((const wchar_t *)procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }

    CloseHandle(hSnap);
    return procId;
}

int
main ()
{
    const wchar_t *name = L"Discord.exe";
    //printf("ProcID: %lu\n", GetProcId(name));
    printf("ProcID: %lu\n", GetMainProcId(name));
}
