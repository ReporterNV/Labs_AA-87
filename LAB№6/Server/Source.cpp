#include "Header.h"

HANDLE hSemSend;
HANDLE hSemRecv;
HANDLE hSemTermination;
HANDLE hSems[2];

CHAR lpSemSendName[] =
"$MySemSendName$";
CHAR lpSemRecvName[] =
"$MySemRecvName$";

CHAR lpSemTerminationName[] =
"$MySemTerminationName$";

CHAR lpFileShareName[] =
"$MyFileShareName$";

HANDLE hFileMapping;

LPVOID lpFileMap;
LONG res;
LPLONG lpres = &res;

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

	DWORD dwRetCode;
	CHAR str[80];
	FILE* hdl;
	DWORD  cbWritten;

	char message[80] = { 0 };
	printf("Mapped and shared file, semaphore sync, server process\n");

	hSemSend = CreateSemaphore(NULL, 0, 1, lpSemSendName);
	hSemRecv = CreateSemaphore(NULL, 0, 1, lpSemRecvName);


	if (hSemSend == NULL || hSemRecv == NULL)
	{
		fprintf(stdout, "CreateSemaphore: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}


	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("\nServerApplication already started\n"
			"Press any key to exit...");
		_getch();
		return 0;
	}


	hSemTermination = CreateSemaphore(NULL, 0, 1, lpSemTerminationName);
	if (hSemTermination == NULL)
	{
		fprintf(stdout, "CreateSemaphore (Termination): Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}

	hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF,
		NULL, PAGE_READWRITE, 0, 100, lpFileShareName);

	if (hFileMapping == NULL)
	{
		fprintf(stdout, "CreateFileMapping: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}


	lpFileMap = MapViewOfFile(hFileMapping,
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);


	if (lpFileMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}

	hSems[0] = hSemTermination;
	hSems[1] = hSemSend;

	while (TRUE)
	{
		int count = 0;
		dwRetCode = WaitForMultipleObjects(2, hSems, FALSE, INFINITE);

		if (dwRetCode == WAIT_ABANDONED_0 || dwRetCode == WAIT_ABANDONED_0 + 1 || dwRetCode == WAIT_OBJECT_0 ||	dwRetCode == WAIT_FAILED){
			break;
			}
		else
		{
			puts(((LPSTR)lpFileMap));
			strcpy(szBuf, ((LPSTR)lpFileMap));

			file = CreateFile(szBuf, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (file == INVALID_HANDLE_VALUE) {
				sprintf(message, "(Server)Can't open %s!", szBuf);
				WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
				printf("\n");
				printf("Bytes sent %d\n", cbWritten);
				count = -1;
			}

			out = CreateFile(outname(szBuf), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (out == INVALID_HANDLE_VALUE) {
				sprintf(message, "(Server)Can't open %s!", szBuf);
				WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
				printf("\n");
				printf("Bytes sent %d\n", cbWritten);
				count = -1;
			}

			if (count == -1) {
				sprintf(message, "%d", (int)count);
				strcpy(((LPSTR)lpFileMap), message);
			}else{
			count = (int)(*Process)(file, out);

				sprintf(message, "(Server): file:%s; replacements = %d\n", szBuf, count);
				WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
				sprintf(message, "%d", (int)count);
				strcpy(((LPSTR)lpFileMap), message);

				CloseHandle(file);
				CloseHandle(out);
			}
			ReleaseSemaphore(hSemRecv, 1, lpres);
		}
	}

	CloseHandle(hSemSend);
	CloseHandle(hSemRecv);
	CloseHandle(hSemTermination);

	UnmapViewOfFile(lpFileMap);

	CloseHandle(hFileMapping);
	FreeLibrary(hLib);
	return 0;
}

