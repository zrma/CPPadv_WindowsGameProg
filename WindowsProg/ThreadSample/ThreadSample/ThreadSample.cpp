// ThreadSample.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>
#include <process.h>

unsigned int WINAPI	ThreadProc( LPVOID lpParam )
{
	for ( int i = 0; i < 100; ++i )
	{
		printf_s( "doing something....\n" );
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwThreadId;
	HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, ThreadProc, NULL, CREATE_SUSPENDED, (unsigned int*)&dwThreadId );
	
	ResumeThread( hThread );

	const char* fileName = "Save.dat";
	HANDLE hSaveFile = CreateFileA( fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( hSaveFile == INVALID_HANDLE_VALUE )
	{
		printf_s( "Create File Error - %s : %d \n", fileName, GetLastError() );

		CloseHandle( hThread );
		return false;
	}

	char* dummyData = new char[1024 * 1024 * 30]();
	DWORD numOfByteWritten = 0;
	WriteFile( hSaveFile, dummyData, 1024 * 1024 * 30, &numOfByteWritten, NULL );

	if ( hThread )
	{
		WaitForSingleObjectEx( hThread, INFINITE, TRUE );
	}
	CloseHandle( hSaveFile );

	printf_s( "FILE %s Write Completed. (error code : %d) \n", fileName, GetLastError() );
	printf_s( "Transferred bytes : %d \n", numOfByteWritten );

	getchar();
	return 0;
}

