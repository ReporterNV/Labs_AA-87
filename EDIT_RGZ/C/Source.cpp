
#include <windows.h>
#include <stdio.h>
#include <conio.h>

int main(int argc, char* argv[])
{
	HANDLE hMailslot1, hMailslot2;
	LPSTR  ReadMailslotName  = (LPSTR)"\\\\*\\mailslot\\$Channel2$";
	LPSTR  WriteMailslotName = (LPSTR)"\\\\*\\mailslot\\$Channel1$";
	char   buff[512];
	// Код возврата из функций
	BOOL   fReturnCode;
	// Размер сообщения в байтах
	DWORD  cbMessages;
	// Количество сообщений в канале Mailslot2
	DWORD  cbMsgNumber;
	DWORD  cbWritten;

	DWORD  cbRead;
	printf("Mailslot client demo\n\n");


	hMailslot1 = CreateFile(WriteMailslotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hMailslot1 == INVALID_HANDLE_VALUE){
		fprintf(stdout, "CreateFile for send: Error %ld\n",	GetLastError());
		_getch();
		return 0;
	}

	fprintf(stdout, "Connected. Type 'exit' to terminate\n");

	hMailslot2 = CreateMailslot(ReadMailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMailslot2 == INVALID_HANDLE_VALUE) {
		fprintf(stdout, "CreateMailslot for reply: Error %ld\n", GetLastError());
		CloseHandle(hMailslot2);
		_getch();
		return 0;
	}

	fprintf(stdout, "Mailslot for reply created\n");

	while (1){

		printf("cmd>");

		scanf("%510s", buff);

		if (!WriteFile(hMailslot1, buff, strlen(buff) + 1, &cbWritten, NULL))
			break;

		if (!strcmp(buff, "exit"))
			break;

		fprintf(stdout, "Waiting for reply...\n");

		fReturnCode = GetMailslotInfo(hMailslot2, NULL, &cbMessages, &cbMsgNumber, NULL);
		if (!fReturnCode){
			fprintf(stdout, "GetMailslotInfo for reply: Error %ld\n", GetLastError());
			_getch();
			break;
		}

		if (ReadFile(hMailslot2, buff, 512, &cbRead, NULL)){
			printf("Received: <%s>\n", buff);
		}
		
	}


	CloseHandle(hMailslot1);
	CloseHandle(hMailslot2);
	return 0;
}
