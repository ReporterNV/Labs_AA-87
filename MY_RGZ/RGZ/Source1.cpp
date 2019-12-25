#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 6248)
#pragma warning (disable: 6387)
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h> 
extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop(); 

DWORD  cbMessages;

DWORD  cbMsgNumber;

BOOL   fReturnCode;

LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel1$";

LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\*\\mailslot\\$Channel2$";


CHAR lpFileShareName[] = "Global\\$MyFileShareNameFinal$";

HANDLE hFileMapping;

LPVOID lpFileMap;
HANDLE hMailslot1, hMailslot2;
int Server()
{

	DWORD dwRetCode;
	CHAR str[80], buf[80], tea[80], szBuf[512];
	DWORD  cbRead;
	FILE* hdl, * hdw;
	DWORD   total = 0;
	int i = 0, k = 0, j;
	char message[80] = { 0 };
	addLogMessage("Mailslot RGZ\n");
	addLogMessage("Server Ready!");



	if (cbMsgNumber != 0)
		if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL))
		{
			sprintf(message, "\n%s\n", szBuf);
			addLogMessage(message);
		}

	while (TRUE)
	{
		//addLogMessage("WaitClient...");


		
		fReturnCode = GetMailslotInfo(hMailslot1, NULL, &cbMessages, &cbMsgNumber, NULL);
		if (!fReturnCode)
		{
			fprintf(stdout, "GetMailslotInfo: Error %ld\n",	GetLastError());	_getch();
			break;
		}

		
		if (cbMsgNumber != 0)
		{

			addLogMessage("Get data!");
			total = 0;
			strcpy(str, szBuf);
			addLogMessage(str);


			

			sprintf(message, "(Server): file:%s rebuild\n", str);
			addLogMessage(message);
			//sprintf(message, "Real number of changes:%d", (int)total);
			//strcpy(((LPSTR)lpFileMap), message);
			//fclose(hdl);
			//fclose(hdw);
		}
			/*}
			else { 
				sprintf(message, "(Server)Can't open %s!", str);
				addLogMessage(message);
				strcpy(((LPSTR)lpFileMap), message);
			}
			addLogMessage("Send responce!");
			CloseHandle(hMailslot1);
//			CloseHandle(hMailslot2);

	*/
	
	}
		CloseHandle(hMailslot1);
		CloseHandle(hMailslot2);
	return 0;
}
int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	
	addLogMessage("Creating Mailslots\n");

	hMailslot1 = CreateMailslot(lpszReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		sprintf(message, "CreateMailslot: Error %ld\n", GetLastError());	_getch();
		addLogMessage(message);
		return 0;
	}
	hMailslot2 = CreateFile(lpszWriteMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hMailslot2 == INVALID_HANDLE_VALUE)
	{
		sprintf(message, "CreateMailslot: Error %ld\n", GetLastError());	_getch();
		addLogMessage(message);	CloseHandle(hMailslot1);
		return 0;
	}

	return 0;
}
void ServiceStop()
{

	UnmapViewOfFile(lpFileMap);
	CloseHandle(hFileMapping);
	addLogMessage("All Kernel objects closed!");
}