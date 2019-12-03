#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <conio.h>

HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out);

char* outname(char* inname)
{
	char* a = inname;
	if ((a = strrchr(inname, '.')) == NULL)
		strcat(a, ".out");
	else
		strcpy(a, ".out");

	return inname;
}

int main()
{
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

	// Код возврата из функций
	BOOL   fReturnCode;
	// Размер сообщения в байтах
	DWORD  cbMessages;
	// Количество сообщений в канале Mailslot
	DWORD  cbMsgNumber;
	// Идентификатор канала Mailslot
	HANDLE hMailslot1 = 0;
	HANDLE hMailslot2 = 0;
	// Имя серверного канала Mailslot
	LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel1$";
	// Имя клиентского канала Mailslot
	LPSTR  lpszWriteMailslotName = (LPSTR)"\\\\*\\mailslot\\$Channel2$";
	// Буфер для передачи данных через канал
	char   szBuf[512];
	// Количество байт данных, принятых через канал
	DWORD  cbRead;
	// Количество байт данных, переданных через канал
	DWORD  cbWritten;
	DWORD   total = 0;
	// буфер для  сообщения об ошибке, результата
	char message[80] = { 0 };

	printf("Mailslot server demo\n");
	// Создаем канал Mailslot, имеющий имя lpszReadMailslotName
	hMailslot1 = CreateMailslot(
		lpszReadMailslotName, 0,
		MAILSLOT_WAIT_FOREVER, NULL);
	// Если возникла ошибка, выводим ее код и зваершаем
	// работу приложения
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateMailslot: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// Выводим сообщение о создании канала
	fprintf(stdout, "Mailslot created\n");
	// Ожидаем соединения со стороны клиента
	// Цикл получения команд через канал
	while (1)
	{
		//fprintf(stdout, "Waiting for connect...\n");
		// Определяем состояние канала Mailslot
		fReturnCode = GetMailslotInfo(
			hMailslot1, NULL, &cbMessages,
			&cbMsgNumber, NULL);
		if (!fReturnCode)
		{
			fprintf(stdout, "GetMailslotInfo: Error %ld\n",
				GetLastError());
			_getch();
			break;
		}
		// Если в канале есть Mailslot сообщения,
		// читаем первое из них и выводим на экран
		if (cbMsgNumber != 0)
		{
			if (ReadFile(hMailslot1, szBuf, 512, &cbRead, NULL))
			{
				// Выводим принятую строку на консоль 
				printf("Received: <%s>\n", szBuf);
				// Если пришла команда "exit", 
				// завершаем работу приложения
				if (!strcmp(szBuf, "exit"))
					break;
				// Иначе считаем что принято имя файла
				//}
				else
				{
					// Открываем канал с процессом MSLOTCLIENT
					hMailslot2 = CreateFile(
						lpszWriteMailslotName, GENERIC_WRITE,
						FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					// Если возникла ошибка, выводим ее код и 
					// завершаем работу приложения
					if (hMailslot2 == INVALID_HANDLE_VALUE)
					{
						fprintf(stdout, "CreateFile for send: Error %ld\n",
							GetLastError());
						_getch();
						//CloseHandle(hMailslot2);
						//return 0;
						break;
					}

						file = CreateFile(szBuf, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

						if (file == INVALID_HANDLE_VALUE) {
							sprintf(message, "(Server)Can't open %s!", szBuf);
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
							printf("\n");
							WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
							printf("Bytes sent %d\n", cbWritten);
						}

						out = CreateFile(outname(szBuf), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

						if (out == INVALID_HANDLE_VALUE) {
							sprintf(message, "(Server)Can't open %s!", szBuf);
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
							printf("\n");
							WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
							printf("Bytes sent %d\n", cbWritten);
						}
						int count = 0;
						count = (int)(*Process)(file, out);

						sprintf(message, "(Server): File complited:%s, replaces = %d\n", outname(szBuf), count);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
						// сообщение в канал
						sprintf(message, "%d", total);
						WriteFile(hMailslot2, message, strlen(message) + 1, &cbWritten, NULL);
						printf("Bytes sent %d\n", cbWritten);
						// закрытие файла
		

						CloseHandle(file);
						CloseHandle(out);
					
					}
				}
			else
			{
				fprintf(stdout, "ReadFile: Error %ld\n",
					GetLastError());
				_getch();
				break;
			}
		}
		// Выполняем задержку на  500 миллисекунд
		Sleep(500);
		//конец цикоа while
	}

	FreeLibrary(hLib);

	CloseHandle(hMailslot1);
	CloseHandle(hMailslot2);
	return 0;
}
