#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

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

int main(int argc, LPTSTR argv[]) {

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
	int count = 0;
	char filename[80] = { 0 };
	char message[80] = { 0 };
	char result[10] = { 0 };
	DWORD cbWritten;

	WriteFile(GetStdHandle(STD_ERROR_HANDLE), "(Child)Get filename for changes:", 36, &cbWritten, NULL);

	ReadFile(GetStdHandle(STD_INPUT_HANDLE), filename, 80, &cbWritten, NULL);

	filename[cbWritten - 2] = 0;

	file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);


	if (file == INVALID_HANDLE_VALUE) {
		printf(message, "Cannot open input file. ERROR: %x\n", GetLastError());
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
		return -3;
	}

	out = CreateFile(outname(filename), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (out == INVALID_HANDLE_VALUE) {
		sprintf(message, "Cannot create/open file. ERROR: %x\n", GetLastError());
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
		return -4;
	}

	count = (int)(*Process)(file, out);

		sprintf(message, "\n(Child): %lu, file:%s, replaces = %d\n", GetCurrentProcessId(), filename, count);

		WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);

		sprintf(result, "%d", count);
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), result, strlen(result) + 1, &cbWritten, NULL);

	FreeLibrary(hLib);

	CloseHandle(file);
	CloseHandle(out);

	//printf("\tNumber of replaces: %i\n", (int)count);

	return count;
	return 0;
}