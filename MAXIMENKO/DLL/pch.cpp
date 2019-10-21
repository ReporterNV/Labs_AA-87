#include <stdio.h>
#include <windows.h>
#include "pch.h"
#define BUF_SIZE 256

void MAXIMENKO(char* a, int* b, int c) {
	*b = 0;
	for (int i = 0; i < c; i++) { //обработка строки
		if (a[i] < 48) {
			
			a[i] = ' ';
			
			*b = *b + 1;


		}

	}
}