// 20140313_Quiz1.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <windows.h>

#define MAX_SIZE	255

int _tmain(int argc, _TCHAR* argv[])
{
	char sInput[] = "Are you my Master?";
	char sOutput[MAX_SIZE];

	ZeroMemory( sOutput, MAX_SIZE );

	int length = strlen( sInput );
	bool isSpace = true;
	int count = 0;
	int spacePosition = 0;

	while ( length > 0 && isSpace )
	{
		isSpace = false;
		count = 0;
		spacePosition = 0;

		while ( count < length )
		{
			if ( sInput[count] == ' ' )
			{
				spacePosition = count;
				isSpace = true;
			}
			++count;
		}

		if ( spacePosition > 0 )
		{
			char temp[MAX_SIZE];
			ZeroMemory( temp, MAX_SIZE );
			
			for ( int i = spacePosition + 1; i < length; ++i )
			{
				sprintf_s( sOutput, "%s%c", sOutput, sInput[i] );
			}

			sprintf_s( sOutput, "%s ", sOutput );

			length = spacePosition;
		}
		else
		{
			for ( int i = 0; i < length; ++i )
			{
				sprintf_s( sOutput, "%s%c", sOutput, sInput[i] );
			}
		}
		
	}

	printf_s( "Output : %s \n", sOutput );

	getchar();
	return 0;
}

