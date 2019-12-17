// ��������� �� ��������� "��������� ����������� �����������"
// �����, ������������ �� ������, ������������� ����������
// ���. ?? 
// ���������� MFS_CLIENT (���������� ����������)
// ������������ ������������� ������, ������������ �� ������, 
// ��� �������� ������ ����� ����������, ������������������� ����������
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

// �������������� ��������-���������, ������� ������������
// ��� ������������� �����, ������������� ������ ���������
HANDLE hSemSend;
HANDLE hSemRecv;
HANDLE hSemTermination;
// ��� ��������-��������� ��� ������������� ������ � ������ �� ������������� �����
CHAR lpSemSendName[] =
"$MySemSendName$";
CHAR lpSemRecvName[] =
"$MySemRecvName$";
// ��� �������-�������� ��� ���������� ��������
CHAR lpSemTerminationName[] =
"$MySemTerminationName$";
// ��� ���������� ����� �� ������
CHAR lpFileShareName[] =
"$MyFileShareName$";
// ������������� ����������� ����� �� ������
HANDLE hFileMapping;
// ��������� �� ������������ ������� ������
LPVOID lpFileMap;

int main()
{
	CHAR str[80];
	DWORD dwRetCode;
	printf("Mapped and shared file, Sem sync, client process\n"
		"\n\nEnter  <Exit> to terminate...\n");
	// ��������� �������-������� ��� ������������� 
	// ������ � ������
	hSemSend = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lpSemSendName); //������ �������� �����
	hSemRecv = OpenSemaphore(SYNCHRONIZE, FALSE, lpSemRecvName); //������ �������� �� �����!!!
	if (hSemSend == NULL || hSemRecv == NULL)
	{
		fprintf(stdout, "OpenSem: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// ��������� ������-������� ��� ������������ �
	// ���������� �������� 
	hSemTermination = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lpSemTerminationName); //������ �������� �����
	if (hSemTermination == NULL)
	{
		fprintf(stdout, "OpenSem (Termination): Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// ��������� ������-�����������
	hFileMapping = OpenFileMapping(
		FILE_MAP_READ | FILE_MAP_WRITE, FALSE, lpFileShareName);
	// ���� ������� �� �������, ������� ��� ������
	if (hFileMapping == NULL)
	{
		fprintf(stdout, "OpenFileMapping: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// ��������� ����������� ����� �� ������.
	// � ���������� lpFileMap ����� ������� ��������� ��
	// ������������ ������� ������
	lpFileMap = MapViewOfFile(hFileMapping,
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// ���� ��������� ����������� �� �������,
	// ������� ��� ������
	if (lpFileMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}

	// ���� ������/������ ������. ���� ���� ��������� ���� ������,
	// ����� ������������ �������� ������ <Exit>, 
	while (TRUE)
	{
		printf("\n:CMD> ");
		// ������ ��������� ������
		scanf("%70s", str);
		// ���������� ������ � ������������ ������,
		// ��������� ���������� ��������
		strcpy((char*)lpFileMap, str);
		// ���� ������� ������� <Exit>, ��������� ����
		if (!strcmp(str, "exit") || !strcmp(str, "Exit") || !strcmp(str, "EXIT"))
			break;
		// ���������� ������ � ������������ ������,
		// ��������� ���������� ��������
		strcpy((char*)lpFileMap, str);
		// ������������� ������-������� � ����������
		// ���������
		ReleaseSemaphore(hSemSend, 1, NULL);
		// ���� ������
		dwRetCode = WaitForSingleObject(hSemRecv, INFINITE);
		// ���� ����� ������� - �������, ���� ������ - ����������!
		if (dwRetCode == WAIT_OBJECT_0) {
			if (strstr((char*)lpFileMap, "-1"))
				fprintf(stderr, "\nCannot open file\n");
			else
				printf("\nRESULT: %s", ((LPSTR)lpFileMap));
		}
		if (dwRetCode == WAIT_ABANDONED || dwRetCode == WAIT_FAILED)
		{
			printf("\nError waiting response! Error:%ld\n", GetLastError());
			//break;
		}
	}
	ReleaseSemaphore(hSemTermination, 1, NULL);

	CloseHandle(hSemSend);
	CloseHandle(hSemRecv);
	CloseHandle(hSemTermination);

	UnmapViewOfFile(lpFileMap);

	CloseHandle(hFileMapping);

	return 0;
}


