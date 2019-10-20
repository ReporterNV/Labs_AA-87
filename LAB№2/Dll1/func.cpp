#include <stdio.h>
#include <windows.h>
#include "pch.h"
#define BUF_SIZE 256

int Process(HANDLE in, HANDLE out, CHAR * buff) {
	DWORD num_file, num_out;
	while (ReadFile(in, buff, BUF_SIZE, &num_file, NULL) && num_file > 0) {
		for (unsigned short i = 0; i < BUF_SIZE; i++)
			if (buff[i] < 48)
				buff[i] = ' ';
		WriteFile(out, buff, num_file, &num_out, NULL);

		if (num_file != num_out) {
			//printf("Неустранимая ошибка записи: %x\n", GetLastError());
			return 4;
		}
	}
	return 0;
}