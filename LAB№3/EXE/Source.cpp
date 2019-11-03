#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out);

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
	if (argc != 3) {
		printf("Uncorrect args\n");
		return  1;
	}

	file = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (file == INVALID_HANDLE_VALUE) {
		printf("Cannot open input file. ERROR: %x\n", GetLastError());
		return  2;
	}

	out = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (out == INVALID_HANDLE_VALUE) {
		printf("Cannot create/open file. ERROR: %x\n", GetLastError());
		return 3;
	}


	count = (int)(*Process)(file, out);

	FreeLibrary(hLib);

	CloseHandle(file);
	CloseHandle(out);

	printf("\tNumber of replaces: %i\n", (int)count);

	return count;
	return 0;
}
