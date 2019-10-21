#include <stdio.h>
#include <windows.h>
#include "pch.h"
#define BUF_SIZE 256

void MAXIMENKO(char* a, int* b, int c) {
	for (DWORD i = 0; i < c; i++) { //обработка строки
		if (int(a[i]) < 48) {

			a[i] = ' ';

			*b++;

		}

	}
}