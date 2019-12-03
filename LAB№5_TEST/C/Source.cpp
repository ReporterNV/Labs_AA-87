// ��������� �� ��������� "��������� ����������� �����������"
// �������� ����� � Windows
// ���. ?? 
// ���������� MSLOTCLIENT (���������� ����������)
// ������������ ������������� �������� ������ 
// ��� �������� ������ ����� ����������

#include <windows.h>
#include <stdio.h>
#include <conio.h>

int main(int argc, char* argv[])
{
	// �������������� ������� Mailslot
	HANDLE hMailslot1, hMailslot2;
	// ��� ����������� ������ Mailslot
	LPSTR  lpszReadMailslotName = (LPSTR)"\\\\.\\mailslot\\$Channel2$";
	// ����� ��� ����� ������ Mailslot
	char   szMailslotName[256];
	// ����� ��� �������� ������ ����� �����
	char   szBuf[512];
	// ��� �������� �� �������
	BOOL   fReturnCode;
	// ������ ��������� � ������
	DWORD  cbMessages;
	// ���������� ��������� � ������ Mailslot2
	DWORD  cbMsgNumber;
	// ���������� ����, ���������� ����� �����
	DWORD  cbWritten;
	// ���������� ���� ������, �������� ����� �����
	DWORD  cbRead;
	printf("Mailslot client demo\n");
	printf("Syntax: mslotclient [servername]\n");
	// ���� ��� ������� ���� ������� ��� ������,
	// ��������� ��� � ����� ������ Mailslot
	if (argc > 1)
		sprintf(szMailslotName, "\\\\%s\\mailslot\\$Channel1$",
			argv[1]);
	// ���� ��� ������� ������ �� ����, ������� �����
	// � ��������� ���������
	else
		strcpy(szMailslotName, "\\\\.\\mailslot\\$Channel1$");
	// ��������� ����� � ��������� MSLOTSERVER
	hMailslot1 = CreateFile(
		szMailslotName, GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	// ���� �������� ������, ������� �� ��� � 
	// ��������� ������ ����������
	if (hMailslot1 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateFile for send: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}
	// ������� ��������� � �������� ������ � ��������
	fprintf(stdout, "Connected. Type 'exit' to terminate\n");
	// ������� ����� Mailslot2, ������� ��� lpszReadMailslotName
	hMailslot2 = CreateMailslot(
		lpszReadMailslotName, 0,
		MAILSLOT_WAIT_FOREVER, NULL);
	// ���� �������� ������, ������� �� ��� � ���������
	// ������ ����������
	if (hMailslot2 == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateMailslot for reply: Error %ld\n",
			GetLastError());
		CloseHandle(hMailslot2);
		_getch();
		return 0;
	}
	// ������� ��������� � �������� ������
	fprintf(stdout, "Mailslot for reply created\n");
	// ���� ������� ������ ����� �����
	while (1)
	{
		// ������� ����������� ��� ����� �������
		printf("cmd>");
		// ������ ��������� ������
		scanf("%510s", szBuf);
		// �������� ��������� ������ ���������� �������� � �������� �������
		if (!WriteFile(hMailslot1, szBuf, strlen(szBuf) + 1,
			&cbWritten, NULL))
			break;
		// � ����� �� ������� "exit" ��������� ����
		// ������ ������� � ��������� ���������
		if (!strcmp(szBuf, "exit"))
			break;
		// ������� �����
		fprintf(stdout, "Waiting for reply...\n");
		// ���������� ��������� ������ Mailslot2
		fReturnCode = GetMailslotInfo(
			hMailslot2, NULL, &cbMessages,
			&cbMsgNumber, NULL);
		if (!fReturnCode)
		{
			fprintf(stdout, "GetMailslotInfo for reply: Error %ld\n",
				GetLastError());
			_getch();
			break;
		}
		// ������ ������
		// ���� � ������ ���� Mailslot ���������,
		// ������ ������ �� ��� � ������� �� �����
		//if (cbMsgNumber != 0)
		//while (cbMsgNumber)
		{
			if (ReadFile(hMailslot2, szBuf, 512, &cbRead, NULL))
			{
				// ������� �������� ������ �� ������� 
				printf("Received: <%s>\n", szBuf);
			}
		}
	}
	// ��������� ������������� ������
	CloseHandle(hMailslot1);
	CloseHandle(hMailslot2);
	return 0;
}
