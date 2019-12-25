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
// �������������� �������� - �������, ������� ������������
// ��� ������������� �����, ������������� ������ ���������, ��� ������ - ����������!
HANDLE hEventSend;
HANDLE hEventRecv;
HANDLE hEventTermination;
HANDLE hEvents[2];
// ��� �������-������� ��� ������������� ������ � ������ �� ������������� �����
CHAR lpEventSendName[] = "Global\\$MylEventSendNameFinal$";
CHAR lpEventRecvName[] = "Global\\$MylEventRecvNameFinal$";
// ��� ���������� ����� �� ������
CHAR lpFileShareName[] = "Global\\$MyFileShareNameFinal$";
// ������������� ����������� ����� �� ������
HANDLE hFileMapping;
// ��������� �� ������������ ������� ������
LPVOID lpFileMap;
int Server()
{
	DWORD dwRetCode;
	CHAR str[80], buf[80], tea[80];
	FILE* hdl, * hdw;
	DWORD   total = 0;
	int i = 0, k = 0, j;
	// ����� ���  ��������� �� ������, ����������
	char message[80] = { 0 };
	addLogMessage("Mapped and shared file, event sync, server process\n");
	// ������� ������� � ServiceStart()
	// ���� ������/������. ���� ���� ��������� ���� ������
	// ��� ���������� �������� �����
	addLogMessage("Server Ready!");
	while (TRUE)
	{
		// ��������� �������� ���������� ������ ������ ��������
		addLogMessage("WaitClient...");
		dwRetCode = WaitForSingleObject(hEventSend, INFINITE);
		// ���� �������� ���� ��������, ��� ���� ��������� ������, ��������� ����
		if (dwRetCode == WAIT_ABANDONED_0 || dwRetCode == WAIT_FAILED)
			break;
		// ������ ������ (��� ����� ��� ���������) �� ������������ ������� ������, 
		// ���������� ���� ���������� ���������, � ���������� ��� � ���������� ����
		else
		{
			puts(((LPSTR)lpFileMap));
			addLogMessage("Get data!");
			total = 0;
			// ��������� ������
			strcpy(str, ((LPSTR)lpFileMap));
			addLogMessage(str);

			k = atoi(buf);
			if ((hdl = fopen(str, "rt")) && (hdw = fopen(strcat(str, ".out"), "w"))) {
				// ���� ������ �� ����� ����� 
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
				// ��������� � ������� ������ 
				sprintf(message, "(Server): file:%s rebuild\n", str);
				addLogMessage(message);
				// ��������� � ����� 
				sprintf(message, "Real number of changes:%d", (int)total);
				strcpy(((LPSTR)lpFileMap), message);
				// �������� �����
				fclose(hdl);
				fclose(hdw);
			}
			else {
				// ��������� � ����� 
				sprintf(message, "(Server)Can't open %s!", str);
				addLogMessage(message);
				strcpy(((LPSTR)lpFileMap), message);
			}
			addLogMessage("Send responce!");
			SetEvent(hEventRecv);
		}
	}

	// ��������� �������������� ��������-�������  � ServiceStop()
	return 0;
}
int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	// ������� �������-������� ��� ������������� ������ � ������ � ������������ ����, ������������ � ������ ���������
	// ����� ����� �������� ������������, ����������� ������������ ������� ���� ������
	// � �� ��������� � ���������������, ��� �� ���������...
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	addLogMessage("Creating security attributes ALL ACCESS for EVERYONE!!!\n");
	// ������� ���������� ������������
	/*InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// DACL �� ���������� (FALSE) - ������ ���������
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	// ����������� �������� ������������, ��������� ���� ��������� �� ���������� ������������ sd � ������� ������-�������
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor= &sd;
	sa.bInheritHandle = false;
	// ��������� ��������� ����������� ������������
	if (!IsValidSecurityDescriptor(&sd))
		{
			res = GetLastError();
			addLogMessage("Security descriptor is invalid.\n");
			sprintf(message, "The last error code: %u\n", res);
			return -(int)res;
		}
	// ������������� ����� ���������� �����
	hEventSend = CreateEvent( &sa, FALSE, FALSE, lpEventSendName);
	hEventRecv = CreateEvent(&sa, FALSE, FALSE, lpEventRecvName);*/

	hEventSend = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)lpEventSendName);
	hEventRecv = CreateEvent(NULL, FALSE, FALSE, (LPCSTR)lpEventRecvName);
	// �������������� ������� ���� ���� �� ���
	SetSecurityInfo(hEventSend, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	SetSecurityInfo(hEventRecv, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	// ���� ��������� ������, �������� � ���������� �� ���, � ����� ��������� ������ ����������
	addLogMessage("Creating events\n");
	if (hEventSend == NULL || hEventRecv == NULL)
	{
		sprintf(message, "CreateEvent: Error %ld\n", GetLastError());
		addLogMessage(message);
		return (-1);
	}
	// ������� ������-�����������, ���� �� �������!!!
	hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, //&sa
		NULL, PAGE_READWRITE, 0, 100, lpFileShareName);
	SetSecurityInfo(hFileMapping, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	addLogMessage("Creating Mapped file\n");
	// ���� ������� �� �������, ������� ��� ������
	if (hFileMapping == NULL)
	{
		sprintf(message, "CreateFileMapping: Error %ld\n", GetLastError());
		addLogMessage(message);
		//getch();
		return -2;
	}
	// ��������� ����������� ����� �� ������.
	// � ���������� lpFileMap ����� ������� ��������� �� ������������ ������� ������
	lpFileMap = MapViewOfFile(hFileMapping,
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// ���� ��������� ����������� �� �������, ������� ��� ������
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
	// �������� ����������� �����
	UnmapViewOfFile(lpFileMap);
	// ����������� ������������� ���������� �������-�����������
	CloseHandle(hFileMapping);
	addLogMessage("All Kernel objects closed!");
}