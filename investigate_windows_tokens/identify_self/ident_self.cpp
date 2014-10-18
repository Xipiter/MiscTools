//tok_play.cpp
//#include "stdafx.h"
//#include <stdio.h>
//#include <windows.h>
//#include <psapi.h>
//#include <iostream> //iostream sux
//#include <commctrl.h>
//#include <Winuser.h>
#include "sa7_win.h"


int main(int argc, char* argv[]) {
	DWORD pid; 
	pid = GetCurrentProcessId();
	printf ("My process ID: 0x%.8x", pid);
};
