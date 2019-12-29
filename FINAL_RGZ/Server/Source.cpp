#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <conio.h>

#define MYServiceName "RGZ service"
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out);



char* outname(char* inname) {
	char* a = inname;
	if ((a = strrchr(inname, '.')) == NULL)
		strcat(a, ".out");
	else
		strcpy(a, ".out");

	return inname;
}

void ReportStatus(DWORD dwCurrentState,	DWORD dwWin32ExitCode, DWORD dwWaitHint){

	static DWORD dwCheckPoint = 1;
	if (dwCurrentState == SERVICE_START_PENDING)
		ServiceStatus.dwControlsAccepted = 0;
	else
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
		ServiceStatus.dwCheckPoint = 0;
	else
		ServiceStatus.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus(hStatus, &ServiceStatus);
}

void WINAPI ServiceControl(DWORD dwControlCode) {
	switch (dwControlCode) {
	case SERVICE_CONTROL_STOP: {     
		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		ReportStatus(ServiceStatus.dwCurrentState, NOERROR, 0);
		ReportStatus(SERVICE_STOPPED, NOERROR, 0);       break;     
	}     case SERVICE_CONTROL_INTERROGATE:     {     
	ReportStatus(ServiceStatus.dwCurrentState, NOERROR, 0);       break;
	}     default:    
	{       ReportStatus(ServiceStatus.dwCurrentState, NOERROR, 0);       break;     }
	}
}

int addLogMessage(const char* text){
	DWORD res, Sz;
	HANDLE hFile;
	char buf[256];
	hFile = CreateFile("C:\\logfile.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (!hFile) {
		return (-1);
	}else{
		GetFileSize(hFile, &Sz);
		SetFilePointer(hFile, 0, NULL, FILE_END);
		sprintf(buf, "%s\r\n", text);
		WriteFile(hFile, buf, strlen(buf), &res, NULL);
		CloseHandle(hFile);
		return (int)res;
	}
}

void ServiceMain() {

	hStatus = RegisterServiceCtrlHandler(MYServiceName, ServiceControl);
	if (!hStatus){
		addLogMessage("Error RegisterServiceCtrlHandler");
		return;
	}

	hLib = LoadLibrary((LPCSTR)"Dll1.dll");
	if (hLib == NULL) {
		printf("Cannot load library");
		exit(-1);
	}

	Process = (int(*)(HANDLE, HANDLE))GetProcAddress(hLib, "Process");

	if (Process == NULL) {
		printf("Function not found");
		exit(-2);
	}

	HANDLE file, out;
	BOOL   fReturnCode;
	DWORD  cbMessages;
	DWORD  cbMsgNumber;
	HANDLE hMailslot1 = 0;
	HANDLE hMailslot2 = 0;

	LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel1$";
	LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\*\\mailslot\\$Channel2$";

	char   szBuf[512];
	DWORD  cbRead;
	DWORD  cbWritten;

	char message[80] = { 0 };

	printf("Mailslot server demo\n");

	hMailslot1 = CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMailslot1 == INVALID_HANDLE_VALUE) {
		fprintf(stdout, "CreateMailslot: Error %ld\n",
			GetLastError());
		_getch();
	}

	fprintf(stdout, "Mailslot created\n");

	while (1) {

		file = 0;

		fReturnCode = GetMailslotInfo(hMailslot1, NULL, &cbMessages, &cbMsgNumber, NULL);
		if (!fReturnCode) {
			fprintf(stdout, "GetMailslotInfo: Error %ld\n", GetLastError());
			_getch();
			break;
		}

		if (cbMsgNumber != 0) {
			if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL)) {

				printf("Received: <%s>\n", szBuf);

				if (!strcmp(szBuf, "exit")) {
					break;
				}
				else {
					hMailslot2 = CreateFile(lpszWriteMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hMailslot2 == INVALID_HANDLE_VALUE) {
						fprintf(stdout, "CreateFile for send: Error %ld\n", GetLastError());
						_getch();
						break;
					}

					file = CreateFile(szBuf, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
					if (file == INVALID_HANDLE_VALUE) {
						sprintf(message, "(Server)Can't open %s!", szBuf);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
						printf("\n");
						WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
					}
					else {
						out = CreateFile(outname(szBuf), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if ((out == INVALID_HANDLE_VALUE)) {
							sprintf(message, "(Server)Can't create %s!", szBuf);
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
							printf("\n");
							WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
						}
						else {

							int count = (int)(*Process)(file, out);

							sprintf(message, "(Server): File complited:%s, replaces = %d\n", outname(szBuf), count);
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
							WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
							CloseHandle(out);
						}
					}
					CloseHandle(file);
					fprintf(stderr, "\n\n");
				}
			}
			else {
				fprintf(stdout, "ReadFile: Error %ld\n", GetLastError());
				_getch();
				break;
			}
		}

		Sleep(500);

	}

	FreeLibrary(hLib);

	CloseHandle(hMailslot1);
	CloseHandle(hMailslot2);
}

void main(int agrc, char* argv[])
{
	char buffer[256];
	SERVICE_TABLE_ENTRY DispatcherTable[] =
	{
		{
			// Имя сервиса
			(LPSTR)MYServiceName,
			// Функция main сервиса
			(LPSERVICE_MAIN_FUNCTION)ServiceMain
		},
		{
			NULL,
			NULL
		}
	};

	addLogMessage("Sample service entry point");

	if (!StartServiceCtrlDispatcher(DispatcherTable))
	{
		sprintf(buffer, "StartServiceCtrlDispatcher: Error %ld\n", GetLastError());
		addLogMessage(buffer);
		return;
	}
}