#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BUF_SIZE 256

HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out, CHAR* buff);

int main(int argc, LPTSTR argv[]) {

	hLib = LoadLibrary((LPCSTR)"Dll1.dll");
	if (hLib == NULL) {
		printf("Cannot load library");
		exit(-1);
	}
	Process = (int(*)(HANDLE, HANDLE, CHAR*))GetProcAddress(hLib, "Process");
	if (Process == NULL) {
		printf("TestHello function not found");
		exit(-2);
	}


	HANDLE file, out;

	CHAR Buffer[BUF_SIZE];
	unsigned int count = 0;
	if (argc != 3) {
		printf("Uncorrect args\n");
		return  1;
	}

	file = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (file == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть входной файл. Ошибка: %x\n", GetLastError());
		return  2;
	}

	out = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (out == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть выходной файл. Ошибка: %x\n", GetLastError());
		return 3;
	}


	if ((*Process)(file, out, Buffer)) {

		printf("\nERROR\n");
		return 4;
	}

	FreeLibrary(hLib);

	CloseHandle(file);
	CloseHandle(out);
	return 0;
}
