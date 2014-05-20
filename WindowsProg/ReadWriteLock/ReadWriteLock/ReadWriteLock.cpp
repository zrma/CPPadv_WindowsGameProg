// ReadWriteLock.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include <process.h>

const int	DATA_SIZE = 100;
char		g_SharedData[DATA_SIZE] = { 0, };
SRWLOCK		g_Lock;

unsigned int WINAPI ReadThreadFunction( LPVOID lpParam )
{
	for ( int id = 0; id < 100000; ++id )
	{
		AcquireSRWLockShared( &g_Lock );

		char firstChar = g_SharedData[0];
		for ( int i = 1; i < DATA_SIZE; ++i )
		{
			assert( g_SharedData[i] == firstChar );
		}

		ReleaseSRWLockShared( &g_Lock );
	}

	return 0;
}

unsigned int WINAPI WriteThreadFuntion( LPVOID lpParam )
{
	for ( int id = 0; id < 100000; ++id )
	{
		AcquireSRWLockExclusive( &g_Lock );

		char w = (char)( rand() % 26 + 65 );
		for ( int i = 0; i < DATA_SIZE; ++i )
		{
			g_SharedData[i] = w;
		}

		ReleaseSRWLockExclusive( &g_Lock );
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	InitializeSRWLock( &g_Lock );

	HANDLE hThread[5] = { 0, };
	DWORD dwThreadID = 0;

	for ( int i = 0; i < 4; ++i )
	{
		hThread[i] = (HANDLE)_beginthreadex( NULL, 0, ReadThreadFunction, NULL, CREATE_SUSPENDED, (unsigned*)&dwThreadID );
	}

	hThread[4] = (HANDLE)_beginthreadex( NULL, 0, WriteThreadFuntion, NULL, CREATE_SUSPENDED, (unsigned*)&dwThreadID );

	for ( int i = 0; i < 5; ++i )
	{
		ResumeThread( hThread[i] );
	}

	WaitForMultipleObjects( 5, hThread, TRUE, INFINITE );

	getchar();
	return 0;
}

