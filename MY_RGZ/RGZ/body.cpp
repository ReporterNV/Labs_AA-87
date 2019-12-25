#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 6248)
#pragma warning (disable: 6387)
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h> // for SetSecurityInfo
extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop();
// Идентификаторы объектов - событий, которые используются
// для синхронизации задач, принадлежащих разным процессам, для службы - ГЛОБАЛЬНЫЕ!
HANDLE hEventSend;
HANDLE hEventRecv;
HANDLE hEventTermination;
HANDLE hEvents[2];
// Имя объекта-событий для синхронизации записи и чтения из отображаемого файла
CHAR lpEventSendName[] = "Global\\$MylEventSendNameFinal$";
CHAR lpEventRecvName[] = "Global\\$MylEventRecvNameFinal$";
// Имя отображния файла на память
CHAR lpFileShareName[] = "Global\\$MyFileShareNameFinal$";
// Идентификатор отображения файла на память
HANDLE hFileMapping;
// Указатель на отображенную область памяти
LPVOID lpFileMap;
int Server()
{
	DWORD dwRetCode;
	CHAR str[80], buf[80], tea[80];
	FILE* hdl, * hdw;
	DWORD   total = 0;
	int i = 0, k = 0, j;
	// буфер для  сообщения об ошибке, результата
	char message[80] = { 0 };
	addLogMessage("Mapped and shared file, event sync, server process\n");
	// создаем объекты в ServiceStart()
	// Цикл чтения/записи. Этот цикл завершает свою работу
	// при завершении процесса ввода
	addLogMessage("Server Ready!");
	while (TRUE)
	{
		// Выполняем ожидание завершения записи данных клиентом
		addLogMessage("WaitClient...");
		dwRetCode = WaitForSingleObject(hEventSend, INFINITE);
		// Если ожидание было отменено, или если произошла ошибка, прерываем цикл
		if (dwRetCode == WAIT_ABANDONED_0 || dwRetCode == WAIT_FAILED)
			break;
		// Читаем данные (имя файла для обработки) из отображенной области памяти, 
		// записанный туда клиентским процессом, и отображаем его в консольном окне
		else
		{
			puts(((LPSTR)lpFileMap));
			addLogMessage("Get data!");
			total = 0;
			// обработка данных
			strcpy(str, ((LPSTR)lpFileMap));
			addLogMessage(str);

			k = atoi(buf);
			if ((hdl = fopen(str, "rt")) && (hdw = fopen(strcat(str, ".out"), "w"))) {
				// цикл чтения до конца файла 
				total = 0;
				while (TRUE)
				{
					if (fgets(tea, 100, hdl) != NULL)
					{
						for (j = 0; j < strlen(str); j++)
						{
							if (tea[j] == ' ')
							{
								tea[j] = 'z';
								total++;
							}
						}
						fputs(tea, hdw);
					}
					else break;
				}
				// сообщение в консоль ошибок 
				sprintf(message, "(Server): file:%s rebuild\n", str);
				addLogMessage(message);
				// сообщение в канал 
				sprintf(message, "Real number of changes:%d", (int)total);
				strcpy(((LPSTR)lpFileMap), message);
				// закрытие файла
				fclose(hdl);
				fclose(hdw);
			}
			else {
				// сообщение в канал 
				sprintf(message, "(Server)Can't open %s!", str);
				addLogMessage(message);
				strcpy(((LPSTR)lpFileMap), message);
			}
			addLogMessage("Send responce!");
			SetEvent(hEventRecv);
		}
	}

	// Закрываем идентификаторы объектов-событий  в ServiceStop()
	return 0;
}
int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	// Создаем объекты-события для синхронизации записи и чтения в отображаемый файл, выполняемого в разных процессах
	// Здесь нужны атрибуты безопасности, позволяющие использовать объекты ВСЕМ юзерам
	// а не создателю и администраторам, как по умолчанию...
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	addLogMessage("Creating security attributes ALL ACCESS for EVERYONE!!!\n");
	// Создаем дескриптор безопасности
	/*InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// DACL не установлен (FALSE) - объект незащищен
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	// Настраиваем атрибуты безопасности, передавая туда указатель на дескриптор безопасности sd и создаем объект-событие
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor= &sd;
	sa.bInheritHandle = false;
	// проверяем структуру дескриптора безопасности
	if (!IsValidSecurityDescriptor(&sd))
		{
			res = GetLastError();
			addLogMessage("Security descriptor is invalid.\n");
			sprintf(message, "The last error code: %u\n", res);
			return -(int)res;
		}
	// устанавливаем новый дескриптор безоп
	hEventSend = CreateEvent( &sa, FALSE, FALSE, lpEventSendName);
	hEventRecv = CreateEvent(&sa, FALSE, FALSE, lpEventRecvName);*/

	hEventSend = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)lpEventSendName);
	hEventRecv = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)lpEventRecvName);
	// альтернативное задание прав всем на все
	SetSecurityInfo(hEventSend, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	SetSecurityInfo(hEventRecv, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	// Если произошла ошибка, получаем и отображаем ее код, а затем завершаем работу приложения
	addLogMessage("Creating events\n");
	if (hEventSend == NULL || hEventRecv == NULL)
	{
		sprintf(message, "CreateEvent: Error %ld\n", GetLastError());
		addLogMessage(message);
		return (-1);
	}
	// Создаем объект-отображение, файл не создаем!!!
	hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, //&sa
		NULL, PAGE_READWRITE, 0, 100, lpFileShareName);
	SetSecurityInfo(hFileMapping, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	addLogMessage("Creating Mapped file\n");
	// Если создать не удалось, выводим код ошибки
	if (hFileMapping == NULL)
	{
		sprintf(message, "CreateFileMapping: Error %ld\n", GetLastError());
		addLogMessage(message);
		//getch();
		return -2;
	}
	// Выполняем отображение файла на память.
	// В переменную lpFileMap будет записан указатель на отображаемую область памяти
	lpFileMap = MapViewOfFile(hFileMapping,
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// Если выполнить отображение не удалось, выводим код ошибки
	if (lpFileMap == 0)
	{
		sprintf(message, "MapViewOfFile: Error %ld\n", GetLastError());
		addLogMessage(message);
		return -3;
	}
	return 0;
}
void ServiceStop()
{
	CloseHandle(hEventSend);
	CloseHandle(hEventRecv);
	// Отменяем отображение файла
	UnmapViewOfFile(lpFileMap);
	// Освобождаем идентификатор созданного объекта-отображения
	CloseHandle(hFileMapping);
	addLogMessage("All Kernel objects closed!");
}