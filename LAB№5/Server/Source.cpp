#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <conio.h>

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

int main() {

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
		return 0;
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
	return 0;
}
