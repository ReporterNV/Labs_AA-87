#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>

int main(int argc, LPTSTR argv[]){

	DWORD cbWritten, cbRead;
	HANDLE hReadPipe1, hWritePipe1, hReadPipe2, hWritePipe2;
	TCHAR Command[] = { "pipe_child.exe" };
	TCHAR filename[80], result[80] = { 0 };
	SECURITY_ATTRIBUTES PipeSA = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	PROCESS_INFORMATION ProcInfoChild;
	STARTUPINFO StartInfoChild;
	//LPTSTR targv = SkipArg(GetCommandLine());
	if (argc < 2)
	{
		printf("(Parent)Enter filename to count spaces:");
		scanf("%s", filename);
	}
	else strcpy(filename, argv[1]);
	GetStartupInfo(&StartInfoChild);
	/* �������� �������, 1� - ����, 2� - ������� */
	CreatePipe(&hReadPipe1, &hWritePipe1, &PipeSA, 0);
	CreatePipe(&hReadPipe2, &hWritePipe2, &PipeSA, 0);
	/* ������������� ����������� ����/����� � ������� �������� �������. */
	StartInfoChild.hStdInput = hReadPipe1;//GetStdHandle(hReadPipe1);
	StartInfoChild.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	StartInfoChild.hStdOutput = hWritePipe2;
	StartInfoChild.dwFlags = STARTF_USESTDHANDLES;
	if (!CreateProcess(NULL, (LPTSTR)Command, NULL, NULL, TRUE /* ������������ �����������. */, 0, NULL, NULL, &StartInfoChild, &ProcInfoChild))
	{
		printf("CreateProcess failed.\n");
		printf("%lu", GetLastError());
		exit(-2);
	}
	/* ��������� ��� ����� � �����. */
	WriteFile(hWritePipe1, filename, strlen(filename) + 2, &cbWritten, NULL);
	printf("(Parent)Bytes written:%d\n", (int)cbWritten);
	/* ������� ���������� ������ ������, ��������� �� ������ �� �����, ����� ������ ������� ����� ���������� ����� �����. */

	/* ������� ��������� ��������� ��������. */
	ReadFile(hReadPipe2, result, 80, &cbRead, NULL);
	printf("\n(Parent)Bytes read:%d, content:%s\n", (int)cbRead, result);
	/* ������� ���������� ������ �� ������, ��������� �� ������ �� �����, ����� ������ ������� ����� ���������� ����� �����. */
	WriteFile(hWritePipe1," ", strlen(" "), &cbWritten, NULL);
	CloseHandle(hReadPipe2);
	CloseHandle(hWritePipe1);
	/* ����� ����� �������� � ���������� ����� ��� ��������� �� ������. */
	if (atoi(result) > 0)
		printf("(Parent)File %s processed by %u has %d spaces\n", filename, ProcInfoChild.dwProcessId, atoi(result));
	else
		printf("(Parent)File %s can't processed by %u because of %s\n", filename, ProcInfoChild.dwProcessId, result);
	/* ������� ���������� ��������� ��������. */
	WaitForSingleObject(ProcInfoChild.hProcess, INFINITE);
	CloseHandle(ProcInfoChild.hProcess);
	return 0;
}
