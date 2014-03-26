// AsyncIO.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>

void CALLBACK FileWriteCompletionCallback( DWORD errCode, DWORD bytes, LPOVERLAPPED overlapped )
{
	wprintf_s( L"FILE %s Write completed. (error code : %u) \n", (wchar_t*)overlapped->hEvent, errCode );
	wprintf_s( L"Transferred bytes : %u \n", bytes );
}

const wchar_t* fileName = L"newfile.txt";
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hFile = CreateFile( fileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, 0 );
	// Async I/O 모드로 파일 오픈

	if ( INVALID_HANDLE_VALUE == hFile )
	{
		return -1;
	}

	OVERLAPPED overlapped;
	memset( &overlapped, 0, sizeof( overlapped ) );
	overlapped.hEvent = (HANDLE)fileName; // 추가로 정보를 전달하는 경로

	char* writeData = (char*)malloc( 1024 * 1024 * 100 );
	WriteFileEx( hFile, writeData, 1024 * 1024 * 100, &overlapped, FileWriteCompletionCallback );

	// 여기에서 바로 다른 일이 가능함
	for ( int i = 0; i < 1000; ++i )
	{
		wprintf_s( L"doing something.... \n" );
	}

	// Alertable 상태로 만들어 주면, I/O 작업이 완료 될 때 callback 함수를 커널에서 호출해 줌
	SleepEx( INFINITE, TRUE );

	CloseHandle( hFile );
	
	getchar();
	return 0;
}

