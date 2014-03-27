// 20140327_Quiz2.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <string>

#define MAX_CHARACTER 26

bool IsUnion( const char* source )
{
	if ( strlen( source ) > 26 )
	{
		return false;
	}

	int counterSet[MAX_CHARACTER] = { 0, };
	int i = 0;

	while ( source[i] )
	{
		int thisIndex = source[i++] - 'a';

		if ( counterSet[thisIndex] < 1 )
		{
			++counterSet[thisIndex];
		}
		else
		{
			return false;
		}
	}

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	const char* ValueSet[] = { "False", "True" };

	const char* testA = "abcdefghijk";
	const char* testB = "abcddfghijk";
	const char* testC = "abcdefghijklmnopqrstuvwxyz";
	const char* testD = "abcdefghijklmnopqrstuvwxyza";
	bool resultA = IsUnion( testA );
	bool resultB = IsUnion( testB );
	bool resultC = IsUnion( testC );
	bool resultD = IsUnion( testD );

	printf_s( "%s -> %s \n", testA, ValueSet[resultA] );
	printf_s( "%s -> %s \n", testB, ValueSet[resultB] );
	printf_s( "%s -> %s \n", testC, ValueSet[resultC] );
	printf_s( "%s -> %s \n", testD, ValueSet[resultD] );

	getchar();
	return 0;
}

