#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> 
#include <conio.h>

#define SERVICE_NAME TEXT("My First Service")

HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out);

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE stopServiceEvent = 0;

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


char* outname(char* inname) {
    char* a = inname;
    if ((a = strrchr(inname, '.')) == NULL)
        strcat(a, ".out");
    else
        strcpy(a, ".out");

    return inname;
}


void TTL(){

addLogMessage("Wait for a client\n");
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

LPSTR  lpszReadMailslotName  = (LPSTR)"\\\\.\\mailslot\\$Channel1$";
LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel2$";

char   szBuf[512];
DWORD  cbRead;
DWORD  cbWritten;

char message[80] = { 0 };

printf("Mailslot server demo\n");

SECURITY_DESCRIPTOR sd;
InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
SetSecurityDescriptorDacl(&sd, true, NULL, false);

SECURITY_ATTRIBUTES sa;
sa.lpSecurityDescriptor = &sd;
sa.bInheritHandle = true;

hMailslot1 = CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, &sa);
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
                sprintf(message,"End with the file: %s\n", szBuf);
                addLogMessage(message);
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

void WINAPI ServiceControlHandler(DWORD controlCode)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);

        SetEvent(stopServiceEvent);
        return;

    case SERVICE_CONTROL_PAUSE:
        break;

    case SERVICE_CONTROL_CONTINUE:
        break;

    default:
        if (controlCode >= 128 && controlCode <= 255)
        {
            break;
        }
        else
        {
            break;
        }
    }

    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI ServiceMain(DWORD argc, TCHAR* argv[])
{

    serviceStatus.dwServiceType = SERVICE_WIN32;
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    serviceStatus.dwControlsAccepted = 0;
    serviceStatus.dwWin32ExitCode = NO_ERROR;
    serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;

    serviceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceControlHandler);

    addLogMessage("Start SCH\n");

    if (!serviceStatusHandle)
    {
        return;
    }
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    stopServiceEvent = CreateEvent(0, FALSE, FALSE, 0);

    serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)TTL, 0, 0, 0);
    WaitForSingleObject(stopServiceEvent, INFINITE);

    serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    CloseHandle(stopServiceEvent);
    stopServiceEvent = 0;
    addLogMessage("Service stop working\n");
    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
    addLogMessage("\nEND\n");
    addLogMessage("\n_______________________________\n");
}


int main(int argc, char** argv)
{
    SERVICE_TABLE_ENTRY serviceTable[] =
    {
       {(LPSTR)SERVICE_NAME, ServiceMain},
       {0, 0}
    };
    addLogMessage("Service Work!\n");
    StartServiceCtrlDispatcher(serviceTable);

    return 0;
}
