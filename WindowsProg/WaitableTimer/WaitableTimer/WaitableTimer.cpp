// WaitableTimer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>

int g_sequence = 0;

void CALLBACK TimeCallback( LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue )
{
	wprintf_s( L"%d : On Timer.... \n", ++g_sequence );
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hTimer = CreateWaitableTimer( NULL, FALSE, NULL );
	if ( NULL == hTimer )
	{
		return -1;
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000; // 1초 후부터 100ms 주기로 동작

	if ( !SetWaitableTimer( hTimer, &liDueTime, 100, TimeCallback, NULL, TRUE ) )
	{
		return -1;
	}

	for ( int i = 0; i < 100; ++i )
	{
		SleepEx( INFINITE, TRUE );
		wprintf_s( L"%d : Outside of Timer \n", ++g_sequence );
	}

	CloseHandle( hTimer );
	getchar();
	return 0;
}

