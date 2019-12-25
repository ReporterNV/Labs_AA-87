#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <conio.h>


DWORD dwErrCode;

SERVICE_STATUS ss;

SERVICE_STATUS_HANDLE ssHandle;
#define MYServiceName "Sampleservice"

extern int Server();
extern int ServiceStart();
extern void ServiceStop();
extern HANDLE hFileMapping, lpFileMap;

void WINAPI ServiceMain(DWORD dwArgc, LPSTR* lpszArv);
void WINAPI ServiceControl(DWORD dwControlCode);
void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
PROCESS_INFORMATION pid;

int addLogMessage(const char* text)
{
	DWORD res, Sz;
	HANDLE hFile;
	char buf[256];
	hFile = CreateFile("C:\\logfile.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (!hFile) return (-1);
	else
	{
		GetFileSize(hFile, &Sz);
		SetFilePointer(hFile, 0, NULL, FILE_END);
		sprintf(buf, "%s\r\n", text);
		WriteFile(hFile, buf, strlen(buf), &res, NULL);
		CloseHandle(hFile);
		return (int)res;
	}
}


void main(int agrc, char* argv[])
{
	char buffer[256];

	SERVICE_TABLE_ENTRY DispatcherTable[] =
	{
		{

			(LPSTR)MYServiceName,

			(LPSERVICE_MAIN_FUNCTION)ServiceMain
		},
		{
			NULL,
			NULL
		}
	};
	addLogMessage("\n\nSample service entry point");

	if (!StartServiceCtrlDispatcher(DispatcherTable))
	{
		sprintf(buffer, "StartServiceCtrlDispatcher: Error %ld\n", GetLastError());
		addLogMessage(buffer);
		return;
	}
}

void WINAPI ServiceMain(DWORD argc, LPSTR* argv)
{
	char buf[256];
	int res = 0;

	ssHandle = RegisterServiceCtrlHandler(MYServiceName, ServiceControl);
	if (!ssHandle)
	{
		addLogMessage("Error RegisterServiceCtrlHandler");
		return;
	}

	ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	ss.dwServiceSpecificExitCode = 0;

	ReportStatus(SERVICE_START_PENDING, NO_ERROR, 30000);
	addLogMessage("Service starting...");

	res = ServiceStart();
	if (res < 0)
	{
		sprintf(buf, "Error init server %d", res);
		addLogMessage(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
		return;
	}

	ReportStatus(SERVICE_RUNNING, NOERROR, 0);
	addLogMessage("Service started!");

	if (Server() > 0)
	{
		addLogMessage("Server MF started!");
	}
	else
	{
		sprintf(buf, "Error starting server %d", res);
		addLogMessage(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
	}
	return;
}


void WINAPI ServiceControl(DWORD dwControlCode)
{

	switch (dwControlCode)
	{

	case SERVICE_CONTROL_STOP:
	{

		ss.dwCurrentState = SERVICE_STOP_PENDING;
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		addLogMessage("Service stopping...");

		ServiceStop();

		ReportStatus(SERVICE_STOPPED, NOERROR, 0);
		addLogMessage("Service stopped!");
		break;
	}

	case SERVICE_CONTROL_INTERROGATE:
	{

		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}

	default:
	{
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}
	}
}

void ReportStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	if (dwCurrentState == SERVICE_START_PENDING)
		ss.dwControlsAccepted = 0;
	else
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ss.dwCurrentState = dwCurrentState;
	ss.dwWin32ExitCode = dwWin32ExitCode;
	ss.dwWaitHint = dwWaitHint;
	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		ss.dwCheckPoint = 0;
	else
		ss.dwCheckPoint = dwCheckPoint++;
	SetServiceStatus(ssHandle, &ss);
}