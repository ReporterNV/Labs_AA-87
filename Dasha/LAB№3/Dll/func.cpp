#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "pch.h"
#define BUF_SIZE 1 // Do not set here a value other than 1. Yes, you need to get the values character-by-character so that there are no problems with the replacement count.

int Process(HANDLE in, HANDLE out) {

	DWORD num_file, num_out;
	char Buffer[BUF_SIZE];

	int counter = 0;
	while (ReadFile(in, Buffer, BUF_SIZE, &num_file, NULL) && num_file > 0) {
		for (int i = 0;i < BUF_SIZE; i++) {
			if (Buffer[i] > 48){
				Buffer[i] = ' ';
				counter = counter + 1;
			}
		}
		WriteFile(out, Buffer, num_file, &num_out, NULL);

		if (num_file != num_out) 
			return 4;
	}
	return (int)counter;
}