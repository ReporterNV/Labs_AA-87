#include <stdio.h>
#include <windows.h>
#include "pch.h"

__declspec(dllexport) int __cdecl MAXIMENKO(char* , int* , int);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBox(NULL, (LPCSTR)"The DLL is loaded", (LPCSTR)"DLL Skeleton", MB_OK);
		break;
	case DLL_THREAD_ATTACH:
		MessageBox(NULL, (LPCSTR)"A thread is created in this process", (LPCSTR)"DLL Skeleton", MB_OK);
		break;
	case DLL_THREAD_DETACH:
		MessageBox(NULL, (LPCSTR)"A thread is destroyed in this process", (LPCSTR)"DLL Skeleton", MB_OK);
		break;
	case DLL_PROCESS_DETACH:
		MessageBox(NULL, (LPCSTR)"The DLL is unloaded", (LPCSTR)"DLL Skeleton", MB_OK);
		break;
	}
	return TRUE;
}