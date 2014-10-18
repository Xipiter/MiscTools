// sdbg.cpp : an attempt at a WinDBG extension.
// My first windows program EVAR! Stephen 12Nov2004
// Props to Lawler for help with the windowsisms.
// Props to Mike for his emotional support.
//
// Buildin' this thing commandline stylez:
// 1. run vcvars32.bat to set ENV
// 2. cl sdbg.cpp /IC:\WINDDK\2600.1106\inc\w2k /c
// 3. link /dll /def:sdbg.def sdbg.obj  <--- my .def is below DONT EXCLUDE /DLL 
// 4. peview sdbg.dll  <----- like "nm" to check that all the shit we wanted exported, was done properly in the Address Table in .rdata 

//My .def looked like this:
//LIBRARY "sdbg.dll"
//
//EXPORTS
//	WinDbgExtensionDllInit
//	ExtensionApiVersion
//	satori
//	satorihelp

// Inside windbg load the module with !load <PATH>sdbg.dll in the command window
// then try one of our functions !satorihelp and !satori

#define _WIN32_WINNT 0x500 //directive for CL to get all its OS-specific shit
#include <windows.h> //CL is dumb it needs to know *EVERYTHING*.
#include <winnt.h> 
#include "wdbgexts.h" //WinDBG API.
#include "ntsdexts.h" //If you are looking for documentation on the DDK debug APIs, this is prolly the best place to start.
					  //Depending the version of your DDK, functions exported by the WinDBG APIs might be also in windbgexts.h

//This is the Global struct for versioning
//EXT_API_VERSION declared in wdbgexts.h
EXT_API_VERSION g_ExtApiVersion = {
		5,
		5,
		EXT_API_VERSION_NUMBER,
		0
};

extern "C" LPEXT_API_VERSION WDBGAPI ExtensionApiVersion(void) //Any functions exported get __declspec(dllexport) a directive for CL to flag function for linker
// purpose: WinDBG calls this function to get the version of our API
// params: void
// returns: pointer to EXT_API_VERSION struct
{
	return &g_ExtApiVersion;
};


extern "C" VOID WDBGAPI WinDbgExtensionDllInit (PWINDBG_EXTENSION_APIS lpExtensionApis, USHORT usMajorVersion, USHORT usMinorVersion) 
//purpose: WinDBG calls this function to initialize our API
//params: pointer to API functions, Major Version, Minor Version
//returns: nan demo nai
{
	ExtensionApis = *lpExtensionApis;
};

//Global Variable Needed for Functions
WINDBG_EXTENSION_APIS ExtensionApis = {0};


//The DECLARE_API is defined in ntsdexts.h but CL was bitching about not finding the DEFINE in our headers
//so we cut and pasted it here to get it to shutup.
#undef DECLARE_API
#define DECLARE_API(s) \
    CPPMOD VOID                                    \
    s(                                             \
        HANDLE                 hCurrentProcess,    \
        HANDLE                 hCurrentThread,     \
        ULONG                  dwCurrentPc,        \
        ULONG                  dwProcessor,        \
        PCSTR                  args                \
     )

DECLARE_API (satorihelp) 
//!satorihelp
//purpose: WinDBG will call this function in our API when the user types !satorihelp
//params: nan demo nai
//returns: nan demo nai
{
	dprintf("Our test Debug Extension. YAY!\n\n");
	dprintf("The contents of our \"Help Page\" would go here!\n");
};


DECLARE_API (satori) 
//!satori
//purpose: WinDBG will call this function in our API when the user types !satori
//params: We can accept params but we arent gonna do that shit yet cuz this might not even work.
//returns: are you fucking kidding!? did you read the line above?
{
	dprintf ("\nWe are actually inside our function now, YAY!\n");
};

