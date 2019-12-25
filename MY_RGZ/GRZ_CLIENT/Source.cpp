#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>


HANDLE hEventSend;
HANDLE hEventRecv;
HANDLE hMailslot1, hMailslot2;
LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel2$";
LPSTR  szMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel1$";


DWORD cbWritten;
HANDLE hFileMapping;

LPVOID lpFileMap;
int main(int argc, char* argv[])
{	
	char   szBuf[512];
	CHAR str[80], letter[80];
	printf("Mapped and shared file, event sync, client process in Session 1\n"
		"\n\nEnter  <Exit> to terminate...\n");

	printf("MAILS1");
	hMailslot1 = CreateFile(szMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateFile for send: Error %ld\n",	GetLastError());
		_getch();
		return 0;
	}
	printf("MAILS@");
	hMailslot2 = CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMailslot2 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateMailslot for reply: Error %ld\n", GetLastError());
		CloseHandle(hMailslot2);
		_getch();
		return 0;
	}
	strcpy(szBuf,"te");
	if (!WriteFile(hMailslot1, szBuf, strlen(szBuf) + 1, &cbWritten, NULL));

	while (TRUE)
	{

		printf("Path>");
		gets_s(str);
		if (!strcmp(str, "exit") || !strcmp(str, "Exit") || !strcmp(str, "EXIT"))
			break;
		strcat(str, " ");
		printf("Letter:");
		gets_s(letter);
		if (letter == NULL)
		{
			printf("Wrong letter");
			getchar();
			break;
		}
		strcat(str, letter);
	}
	return 0;
}