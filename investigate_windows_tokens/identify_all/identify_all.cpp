//See what is going on with token shit.
//#include "stdafx.h"
//#include <stdio.h>
//#include <windows.h>
//#include <psapi.h>
//#include <iostream> //iostream sux
//#include <commctrl.h>
//#include <Winuser.h>
#include "sa7_win.h"

// PSAPI declarations
typedef BOOL (WINAPI *pfEnumProcesses)( DWORD *pidlist, DWORD bufSize, DWORD *bufNeeded );
typedef BOOL (WINAPI *pfEnumProcessModules)( HANDLE hp, HMODULE *phm, DWORD cb, LPDWORD lpcbNeeded );
typedef DWORD (WINAPI *pfGetModuleBaseName)( HANDLE hp, HMODULE hm, LPTSTR pName, DWORD bufSize );



int main( int argc, char *argv[] );
bool getPriv( const char *privName );
bool dumpToken( HANDLE hp, bool doDisplay = true );
void showSid( PSID ps );
BOOL Sid2Text( PSID ps, char *buf, int bufSize );
BOOL IsLogonSid( PSID ps );
BOOL IsLocalSid( PSID ps );
BOOL IsInteractiveSid( PSID ps );



#define	gle (GetLastError())
#define isBadHandle(h) ((h) == NULL || (h) == INVALID_HANDLE_VALUE)
#define lenof(x) (sizeof (x) / sizeof ((x)[0]))



const int MAXPID = 1024;
const int MAXSIZE = 16384; // size _does_ matter



// global display on/off flag -- set to false if you want to know how long all this takes
const bool showInfo = true;



int main( int argc, char *argv[] )
{
	int i, nPIDs, interactiveProcesses;
	DWORD bufNeeded;
	HANDLE hp;
	DWORD *pid = NULL;
	HINSTANCE hPsapi;
	HMODULE hm;
	char moduleName[MAX_PATH];
	LARGE_INTEGER t0, t1, fr;

	pfEnumProcesses pfEP;
	pfEnumProcessModules pfEPM;
	pfGetModuleBaseName pfGMBN;

	if ( argc != 1 )
	{
		printf( "\nUsage: %s\n", argv[0] );
		puts( "\nThis program iterates over all processes in the system" );
		puts( "(using EnumProcesses(), tries to open their process tokens," );
		puts( "and dumps the SIDs in those tokens." );
		return 1;
	}

	// try to acquire SeDebugPrivilege, if fail, then do without
	getPriv( SE_DEBUG_NAME );

	hPsapi = LoadLibrary( "psapi.dll" );
	if ( hPsapi == NULL )
	{
		printf( "LoadLibrary( \"psapi.dll\" ): gle = %lu\n", gle );
		return 1;
	}

	pfEP = (pfEnumProcesses) GetProcAddress( hPsapi, "EnumProcesses" );
	pfEPM = (pfEnumProcessModules) GetProcAddress( hPsapi, "EnumProcessModules" );
	pfGMBN = (pfGetModuleBaseName) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
	if ( pfEP == NULL || pfEPM == NULL || pfGMBN == NULL )
	{
		printf( "GetProcAddress(): one or more PSAPI functions not found\n" );
		return 1;
	}

	// here, we start with room for 16 DWORDS. If EnumProcesses() comes back
	// and tells us that the space was _all_ used, we try again with more,
	// until EP() doesn't fill all of it. That's when we know that we supplied
	// enough.

	QueryPerformanceCounter( &t0 );

	interactiveProcesses = 0;
	nPIDs = 0; // starts with one increment more, actually
	pid = NULL;
	do
	{
		nPIDs += 16;
		free( pid );
		pid = (DWORD *) malloc( nPIDs * sizeof DWORD );
		if ( ! pfEP( pid, (DWORD) nPIDs * 4U, &bufNeeded ) )
		{
			printf( "EnumProcesses(): gle = %lu\n", gle );
			return 1;
		}
	} while ( (int) ( (DWORD) nPIDs * 4U - bufNeeded ) <= 0 );

	// The next line computes the _actual_ number of PIDs retrieved (doh!).
	// Kudos to Chris Scheers <asi&airmail.net> for the bug report and this fix!
	nPIDs = bufNeeded / sizeof DWORD;

	if ( showInfo )
		printf( "\n%d PIDs found.\n", nPIDs );

	// for each PID:
	for ( i = 0; i < nPIDs; i ++ )
	{
		// possibly attempt to add ourselves to the target's ACL?
		// SeDebugPrivilege makes this wholly unnecessary for this sample

		// open process
		hp = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid[i] );
		if ( isBadHandle( hp ) )
		{
			if ( showInfo )
				printf( "\nOpenProcess( pid = %lu ): gle = %lu\n", pid[i], gle );
			continue;
		}

		// we only want the first HMODULE
		if ( showInfo && pfEPM( hp, &hm, sizeof hm, &bufNeeded ) )
		{
			if ( ! pfGMBN( hp, hm, moduleName, sizeof moduleName ) )
				strcpy( moduleName, "--unknown--" ); // this means, module list OK but no name
		}
		else
			strcpy( moduleName, "==unknown==" ); // this means, no module list

		if ( showInfo )
			printf( "\npid %lu [%s]:\n", pid[i], moduleName );

		// now, to the meat of the matter
		if ( dumpToken( hp, showInfo ) )
			++ interactiveProcesses;

		// close handle
		CloseHandle( hp );
	}

	QueryPerformanceCounter( &t1 );
	QueryPerformanceFrequency( &fr );

	printf( "%d PIDs (of which %d are interactive) took %.3lf usec.\n",
		nPIDs, interactiveProcesses,
		1000.0 * (double) ( t1.QuadPart - t0.QuadPart ) / (double) fr.QuadPart );

	delete [] pid;
	FreeLibrary( hPsapi );

	return 0;
}



bool dumpToken( HANDLE hp, bool doDisplay /* = true */ )
{
	bool querySource = true;
	DWORD needed;
	HANDLE ht;
	DWORD i;

	TOKEN_OWNER *pto;
	TOKEN_USER *ptu;
	TOKEN_PRIMARY_GROUP *ptpg;
	TOKEN_GROUPS *ptg;

	// these three keep track of what we found in the token
	bool haveLocalSid = false, haveLogonSid = false, haveInteractiveSid = false;

	//* possibly attempt to add ourselves to the target's ACL?

	// open process token

	if ( ! OpenProcessToken( hp, TOKEN_QUERY | TOKEN_QUERY_SOURCE, &ht ) )
	{
		if ( doDisplay )
			printf( "  OpenProcessToken( T_Q_S ): gle = %lu, "
				"trying without TOKEN_QUERY_SOURCE\n", gle );
		querySource = false;

		if ( ! OpenProcessToken( hp, TOKEN_QUERY, &ht ) )
		{
			if ( doDisplay )
				printf( "  OpenProcessToken(): gle = %lu\n", gle );
			return false;
		}
	}

	// dump token information

	if ( querySource && doDisplay )
	{
		TOKEN_SOURCE ts;
		if ( ! GetTokenInformation( ht, TokenSource, &ts, sizeof ts, &needed ) )
			printf( "  GetTokenInformation( TokenSource ): gle = %lu\n", gle );
		else
			printf( "  Token source: \"%-8.8s\" (luid = %I64d)\n",
				ts.SourceName, ts.SourceIdentifier );
	}

	// token owner
	pto = (TOKEN_OWNER *) malloc( MAXSIZE );
	if ( ! GetTokenInformation( ht, TokenOwner, pto, MAXSIZE, &needed ) )
		printf( "  GetTokenInformation( TokenOwner ): gle = %lu\n", gle );
	else
	{
		if ( doDisplay )
		{
			printf( "  Token owner:\n    " );
			showSid( pto->Owner );
			putchar( '\n' );
		}
	}

	// token user
	ptu = (TOKEN_USER *) malloc( MAXSIZE );
	if ( ! GetTokenInformation( ht, TokenUser, ptu, MAXSIZE, &needed ) )
		printf( "  GetTokenInformation( TokenUser ): gle = %lu\n", gle );
	else
	{
		if ( doDisplay )
		{
			printf( "  Token user:\n    " );
			showSid( ptu->User.Sid );
			putchar( '\n' );
		}
	}

	// token primary group
	ptpg = (TOKEN_PRIMARY_GROUP *) malloc( MAXSIZE );
	if ( ! GetTokenInformation( ht, TokenPrimaryGroup, ptpg, MAXSIZE, &needed ) )
		printf( "  GetTokenInformation( TokenPrimaryGroup ): gle = %lu\n", gle );
	else
	{
		if ( doDisplay )
		{
			printf( "  Token primary group:\n    " );
			showSid( ptpg->PrimaryGroup );
			putchar( '\n' );
		}
	}

	// token groups
	ptg = (TOKEN_GROUPS *) malloc( MAXSIZE );
	if ( ! GetTokenInformation( ht, TokenGroups, ptg, MAXSIZE, &needed ) )
		printf( "  GetTokenInformation( TokenGroups ): gle = %lu\n", gle );
	else
	{
		if ( ptg->GroupCount == 0 )
			printf( "  Token groups: (none)\n" );
		else
		{
			if ( doDisplay )
				printf( "  Token groups:\n" );
			for ( i = 0; i < ptg->GroupCount; ++ i )
			{
				if ( doDisplay )
				{
					printf( "    " );
					showSid( ptg->Groups[i].Sid );
					putchar( '\n' );
				}
				if ( IsLocalSid( ptg->Groups[i].Sid ) )
					haveLocalSid = true;
				if ( IsLogonSid( ptg->Groups[i].Sid ) )
					haveLogonSid = true;
				if ( IsInteractiveSid( ptg->Groups[i].Sid ) )
					haveInteractiveSid = true;
			}
		}
	}

	if ( doDisplay && haveLocalSid && haveInteractiveSid && haveLogonSid )
	{
		printf( "  YES! This process runs under an interactive user:\n    " );
		showSid( ptu->User.Sid );
		putchar( '\n' );
	}
	else if ( doDisplay && ( haveLocalSid || haveInteractiveSid || haveLogonSid ) )
	{
		printf( "  YES! This process *probably* runs under an interactive user:\n    " );
		showSid( ptu->User.Sid );
		putchar( '\n' );
	}
	else if ( doDisplay )
		printf( "  Nope. Looks as if this process had not been run by an interactive user.\n" );

	CloseHandle( ht );
	free( pto );
	free( ptu );
	free( ptpg );
	free( ptg );

	return haveLocalSid || haveInteractiveSid || haveLogonSid;
}



void showSid( PSID ps )
{
	char textSid[MAX_PATH], user[MAX_PATH], domain[MAX_PATH];
	DWORD sizeUser, sizeDomain;
	SID_NAME_USE snu;
	const char *t;
	const char *sep = "\\"; // separator for domain\user display

	Sid2Text( ps, textSid, sizeof textSid );
	printf( "%s ", textSid );

	sizeUser = sizeof user;
	sizeDomain = sizeof domain;
	if ( ! LookupAccountSid( NULL, ps, user, &sizeUser, domain, &sizeDomain, &snu ) )
	{
		DWORD rc = gle;

		if ( IsLogonSid( ps ) )
			printf( "(interactive logon session SID)" );
		else
			printf( "[LAS(): gle = %lu]", rc );
		return;
	}

	switch ( snu )
	{
		case SidTypeUser:
			t = "user";
			break;
		case SidTypeGroup:
			t = "group";
			break;
		case SidTypeDomain:
			t = "domain";
			break;
		case SidTypeAlias:
			t = "alias";
			break;
		case SidTypeWellKnownGroup:
			t = "well-known group";
			break;
		case SidTypeDeletedAccount:
			t = "deleted";
			break;
		case SidTypeInvalid:
			t = "invalid";
			break;
		case SidTypeUnknown:
			t = "unknown";
			break;
		case SidTypeComputer:
			t = "computer";
			break;
		default:
			t = "*?unknown?*";
			break;
	}

	if ( domain[0] == '\0' || user[0] == '\0' )
		sep = "";

	printf( "\"%s%s%s\" (%s)", domain, sep, user, t );
}



BOOL IsLocalSid( PSID ps )
{
	static PSID pComparisonSid = NULL;

	if ( pComparisonSid == NULL )
	{
		// build "BUILTIN\LOCAL" SID for comparison: S-1-2-0
		SID_IDENTIFIER_AUTHORITY sia = SECURITY_LOCAL_SID_AUTHORITY;
		AllocateAndInitializeSid( &sia, 1, 0, 0, 0, 0, 0, 0, 0, 0, &pComparisonSid );
	}

	return EqualSid( ps, pComparisonSid );
}



BOOL IsInteractiveSid( PSID ps )
{
	static PSID pComparisonSid = NULL;

	if ( pComparisonSid == NULL )
	{
		// build "BUILTIN\LOCAL" SID for comparison: S-1-5-4
		SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY; // "-5-"
		AllocateAndInitializeSid( &sia, 1, 4, 0, 0, 0, 0, 0, 0, 0, &pComparisonSid );
	}

	return EqualSid( ps, pComparisonSid );
}



BOOL IsLogonSid( PSID ps )
{
	static SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;

	// a logon SID has: sia = 5, subauth count = 3, first subauth = 5
	// the following three lines test these three conditions
	if ( ! memcmp( GetSidIdentifierAuthority( ps ), &sia, sizeof sia )	&& // is sia == 5?
	  *GetSidSubAuthorityCount( ps ) == 3								&& // is subauth count == 3?
	  *GetSidSubAuthority( ps, 0 ) == 5									)  // first subauth == 5?
		return TRUE;
	else
		return FALSE;
}



// nearly straight from the SDK
BOOL Sid2Text( PSID ps, char *buf, int bufSize )
{
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev = SID_REVISION;
	DWORD i;
	int n, size;
	char *p;

	// Validate the binary SID.

	if ( ! IsValidSid( ps ) )
		return FALSE;

	// Get the identifier authority value from the SID.

	psia = GetSidIdentifierAuthority( ps );

	// Get the number of subauthorities in the SID.

	dwSubAuthorities = *GetSidSubAuthorityCount( ps );

	// Compute the buffer length.
	// S-SID_REVISION- + IdentifierAuthority- + subauthorities- + NULL

	size = 15 + 12 + ( 12 * dwSubAuthorities ) + 1;

	// Check input buffer length.
	// If too small, indicate the proper size and set last error.

	if ( bufSize < size )
	{
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return FALSE;
	}

	// Add 'S' prefix and revision number to the string.

	size = wsprintf( buf, "S-%lu-", dwSidRev );
	p = buf + size;

	// Add SID identifier authority to the string.

	if ( psia->Value[0] != 0 || psia->Value[1] != 0 )
	{
		n = wsprintf( p, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
		(USHORT) psia->Value[0], (USHORT) psia->Value[1],
		(USHORT) psia->Value[2], (USHORT) psia->Value[3],
		(USHORT) psia->Value[4], (USHORT) psia->Value[5] );
		size += n;
		p += n;
	}
	else
	{
		n = wsprintf( p, "%lu", ( (ULONG) psia->Value[5] ) +
		( (ULONG) psia->Value[4] << 8 ) + ( (ULONG) psia->Value[3] << 16 ) +
		( (ULONG) psia->Value[2] << 24 ) );
		size += n;
		p += n;
	}

	// Add SID subauthorities to the string.

	for ( i = 0; i < dwSubAuthorities; ++ i )
	{
		n = wsprintf( p, "-%lu", *GetSidSubAuthority( ps, i ) );
		size += n;
		p += n;
	}

	return TRUE;
}



bool getPriv( const char *privName )
{
	bool rc;
	HANDLE hToken;
	LUID privValue;
	TOKEN_PRIVILEGES tkp;

	if ( ! OpenProcessToken( GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		return false;

	if ( !LookupPrivilegeValue( NULL, privName, &privValue ) )
	{
		CloseHandle( hToken );
		return false;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = privValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	rc = !! AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof tkp, NULL, NULL );

	CloseHandle( hToken );

	return rc;
}

