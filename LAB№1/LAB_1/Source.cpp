#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#define BUF_SIZE 256

int main(int argc, LPTSTR argv[]){
	HANDLE file, out;
	DWORD num_file, num_out;
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
	
	for (int i = 0; i < 48; i++)
		printf("%c", i);

	while (ReadFile(file, Buffer, BUF_SIZE, &num_file, NULL) && num_file > 0) {
		for (unsigned short i = 0; i < BUF_SIZE; i++)
			if (Buffer[i] < 48)
				Buffer[i] = ' ';
		WriteFile(out, Buffer, num_file, &num_out, NULL);

		if (num_file != num_out) {
			printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return 4;
		}
	}/*
	FILE *file = fopen(argv[1],"r");
	char str[255];
	stpncpy(str,argv[1],255);
	strcat(str,".out");
	FILE *out = fopen(str,"w");
	char buf[255]="";
	do{if(strstr(buf, argv[2])!=0){
		fprintf(out,"%255s", buf);}
	}while(fgets(buf,255,file));
	*/
	CloseHandle(file);
	CloseHandle(out);
	return 0;
}

/*#include <windows.h> 
#include <stdio.h> 
#define BUF_SIZE 256

int main(int argc, LPTSTR argv[]) {
	HANDLE hIn, hOut;
	DWORD nIn, nOut;
	CHAR Buffer[BUF_SIZE];
	if (argc != 3) {
		printf("Использование: cpw файл1 файл2\n");
		return  1;
	}
	hIn = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть входной файл. Ошибка: %x\n", GetLastError());
		return  2;
	}
	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Невозможно открыть выходной файл. Ошибка: %x\n", GetLastError());
		return 3;
	}
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
		WriteFile(hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return 4;
		}
	}
	CloseHandle(hIn);
	CloseHandle(hOut);
	return 0;
}*/