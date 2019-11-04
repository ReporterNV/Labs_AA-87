#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#define MAX_THREAD 255


char* outname(char* inname)
{
	char* a = inname;

	if ((a = strrchr(inname, '.')) == NULL)
		strcat(inname, ".out");
	else
		strcpy(a, ".out");
	return inname;
}

int main(int argc, char* argv[])
{
	int i;
	DWORD finish, result, total = 0;
	LPDWORD res = &result;
	char line[80];
	char* ln = line;
	HANDLE hThread [MAX_THREAD];
	STARTUPINFO si [MAX_THREAD];
	PROCESS_INFORMATION pi[MAX_THREAD];
	if (argc < 2) {
		printf("No file to process!\n");
		exit(-1);
	}

	for (i = 0; i < (argc - 1); i++) {

		strcpy(ln, "Project1.exe");
		ln = strcat(ln, " ");
		ln = strcat(ln, argv[i + 1]);
		ln = strcat(ln, " ");
		ln = strcat(ln, outname(argv[i + 1]));
		ZeroMemory(&si[i], sizeof(si[i]));
		si[i].cb = sizeof(si);
		ZeroMemory(&pi[i], sizeof(pi[i]));

		if (!CreateProcess(NULL,
			line,
			NULL,
			NULL,
			TRUE,
			NULL,
			NULL,
			NULL,
			&si[i],
			&pi[i])
			)
		{
			printf("CreateProcess failed.\n");
			exit(-2);
		}
		else {
			printf("\nProcess %lu started. Output file: %s\n", pi[i].dwProcessId, argv[i + 1]);
			hThread[i] = pi[i].hProcess;
			Sleep(1000);
		}
	}

	puts("\n_____________________________________\n");

	for (i = 0; i < (argc - 1); i++)
	{
		finish = WaitForSingleObject(pi[i].hProcess, INFINITE);
		if (finish == WAIT_OBJECT_0) {
			GetExitCodeProcess(pi[i].hProcess, res);
			printf("Process %lu finished: %d replacements\n", pi[i].dwProcessId, result);
			total += result;
		}
		else
		{
			CloseHandle(pi[i].hProcess);
			CloseHandle(pi[i].hThread);
			TerminateProcess(pi[i].hProcess, 0);
		}
	}
	printf("\n%d File(s) processed with total replace %d!\n", argc - 1, total);
	return 0;
}