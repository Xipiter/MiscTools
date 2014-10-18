//.def
//LIBRARY "sdbg.dll"
//
//EXPORTS
//	WinDbgExtensionDllInit
//	ExtensionApiVersion
//	satori
//	satorihelp

#define PEB_LOC (0x7ffdf000) //this was obtained by looking at PEB entry in any processes !teb for this platform, maybe soon this will
							 //actually get it from kernel or something or wherever its normally stored, right now its hardcoded.
#define NUM_HEAPS_PEB_OFFSET (0x088)
#define HEAP_HANDLE_PEB_OFFSET (0x090)
#define THREAD_OFFSET_FROM_PEB (0x1000)
#define TEB_FS_SELF (0x18)
#define TEB_STACK_BASE (0x04)
#define TEB_STACK_LIMIT (0x08) 

#define _WIN32_WINNT 0x500 
#include <windows.h> 
#include <winnt.h> 
#include "wdbgexts.h" 
#include "ntsdexts.h"

//This is the Global struct for versioning:
//EXT_API_VERSION declared in wdbgexts.h
EXT_API_VERSION g_ExtApiVersion = {
		5,
		5,
		EXT_API_VERSION_NUMBER,
		0
};

extern "C" LPEXT_API_VERSION WDBGAPI ExtensionApiVersion(void) 
{
	return &g_ExtApiVersion;
};


extern "C" VOID WDBGAPI WinDbgExtensionDllInit (PWINDBG_EXTENSION_APIS lpExtensionApis, USHORT usMajorVersion, USHORT usMinorVersion) 
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

DECLARE_API (countheaps) 
{
	ULONG num_heaps, bytes_transferred;
	int index;	bool retval = TRUE;
	dprintf("Attempting to access PEB at: 0x%x ...", PEB_LOC);
	ReadMemory(PEB_LOC+NUM_HEAPS_PEB_OFFSET, &num_heaps, sizeof(num_heaps), &bytes_transferred);
	if (retval == FALSE) {
		dprintf("...failure!\n");
	} else {
		dprintf ("...success! Read %d bytes.\nI found %x heap segments.\n", sizeof(num_heaps), num_heaps);
		for (index = 0; index < num_heaps; index++) {
			dprintf("%d-", num_heaps);
		};

	};

};

DECLARE_API (stack) {
	ULONG StackBase, StackLimit, addr_self, bytes_transferred;
	int index; bool retval = TRUE;

    ReadMemory(PEB_LOC, &addr_self, sizeof(addr_self), &bytes_transferred); //just a test
    if (retval == FALSE) {
        dprintf("...failure!\n");
    } else {
		for (index = 1; index <= 3; index++) {
			ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_FS_SELF), &addr_self, sizeof(addr_self), &bytes_transferred);
			if ((PEB_LOC-(THREAD_OFFSET_FROM_PEB * index)) == addr_self) {
				ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_STACK_BASE), &StackBase, sizeof(StackBase),
&bytes_transferred);
				ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_STACK_LIMIT), &StackLimit, sizeof(StackLimit), &bytes_transferred);
				dprintf("%d:  %x - %x\n", index, StackBase, StackLimit);
			};
		};
	};

};

DECLARE_API (findp2p) {
	ULONG start, end;
	start = GetExpression(args); end = GetExpression(args[1]);
	dprintf("\nStarting at: %x Ending at: %x", start, end);
};

DECLARE_API (dumpheapstrings) {
	dprintf("\nUnimplemented!");
};

DECLARE_API (dumpstackstrings) 
//purpose: Getting arguements to our command on teh windbg commmandline
//			ripped from: http://www.codeproject.com/debug/cdbntsd4.asp
//we could cheat and use WinDBG's data structures, but we are leet and find the stacks of each
//thread ourselves.
//params: 
//returns: 

// BUG: going past StackLimit in search... needs to be fixed
{
    static ULONG Address = 0;
    ULONG  StackBase, StackLimit, GetAddress = 0, StringAddress, Index = 0, Bytes, addr_self, bytes_transferred;
    WCHAR MyString[51] = {0}; //plus \0
	int index; bool retval = TRUE;

    if(GetAddress != 0){
        Address = GetAddress;
    };
	GetAddress = GetExpression(args);

	dprintf("Assuming PEB is at 0x%x", PEB_LOC);
	ReadMemory(PEB_LOC, &addr_self, sizeof(addr_self), &bytes_transferred); //just a test
	if (retval == FALSE) {
		dprintf("...failure!\n");
	} else {
		for (index = 1; index <= 3; index++) {
			ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_FS_SELF), &addr_self, sizeof(addr_self), &bytes_transferred);
			if ((PEB_LOC-(THREAD_OFFSET_FROM_PEB * index)) == addr_self) {
	            ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_STACK_BASE), &StackBase, sizeof(StackBase), &bytes_transferred);
				ReadMemory(PEB_LOC-((THREAD_OFFSET_FROM_PEB * index)-TEB_STACK_LIMIT), &StackLimit, sizeof(StackLimit), &bytes_transferred);
				dprintf("\nTEB's self matches thread base! Valid thread found at: %x \nSearching its stack at (%x - %x)\n", PEB_LOC-(THREAD_OFFSET_FROM_PEB * index), StackBase, StackLimit);
			    for(Index = 0; Index < StackLimit; Index+=4) {
					memset(MyString, 0, sizeof(MyString));
        			Bytes = 0;
        			ReadMemory(StackBase - Index, &StringAddress, sizeof(StringAddress), &Bytes); //- Index cuz stack grows down
					if(Bytes) {
						Bytes = 0;
						ReadMemory(StringAddress, MyString, sizeof(MyString) - 2, &Bytes);
           				if(Bytes) {
          					dprintf("%08x : %08x = (UNICODE) \"%ws\"\n",
                   				StackBase - Index, StringAddress, MyString);
           					dprintf("%08x : %08x = (ANSI)    \"%s\"\n",
                     			StackBase - Index, StringAddress, MyString);
           				} else {
              			//	dprintf("%08x : %08x =  Address Not Valid\n",
						//	Address + Index, StringAddress);
           				}
        			} else {
           				//dprintf("%08x : Address Not Valid\n", Address + Index);
        			}
    			}
			};
		};
	};
};

void printusage() {
	dprintf ("\n!helpme findstrings <addressA> <addressB> //Helps find strings between addresses A and B");

};

DECLARE_API (helpme) {
	dprintf("%s", args);
};
