#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>
#include <conio.h>

HANDLE file, out;
HINSTANCE hLib;
int(*Process)(HANDLE in, HANDLE out);
char   szBuf[512];

char* outname(char* inname)
{
	char* a = inname;
	if ((a = strrchr(inname, '.')) == NULL)
		strcat(a, ".out");
	else
		strcpy(a, ".out");

	return inname;
}
