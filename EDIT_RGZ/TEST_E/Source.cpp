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

char* outname(char* inname) {
    char* a = inname;
    if ((a = strrchr(inname, '.')) == NULL)
        strcat(a, ".out");
    else
        strcpy(a, ".out");

    return inname;
}


void TTL(){

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
LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel2$";

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


void TimeToFile(const char* fname)
{
    FILE* f = fopen(fname, "wb");

    if (f == NULL)
    {
        return;
    }

    char dtime[20];
    time_t now;
    struct tm* ptr;

    while (1)
    {
        now = time(NULL);
        ptr = localtime(&now);
        strftime(dtime, sizeof(dtime) - 1, "%d.%m.%y %H:%M:%S", ptr);

        fseek(f, 0, SEEK_SET);

        fprintf(f, "%s", dtime);

        Sleep(1000);
    }
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

    serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}


int main(int argc, char** argv)
{
    SERVICE_TABLE_ENTRY serviceTable[] =
    {
       {(LPSTR)SERVICE_NAME, ServiceMain},
       {0, 0}
    };

    StartServiceCtrlDispatcher(serviceTable);

    return 0;
}