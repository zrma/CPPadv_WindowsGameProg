// Lambda.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <list>
#include <string>

class Character
{
public:
	Character() {}
	~Character() {}

	void Name( const std::string& name )
	{
		m_Name = name;
	}
	std::string& GetName()
	{
		return m_Name;
	}
private:
	std::string	m_Name;
};

void CharacterSortByName()
{
	std::list<Character> charList;

	Character agebreak;
	agebreak.Name( "agebreak" );
	charList.push_back( agebreak );

	Character sm9;
	sm9.Name( "sm9" );
	charList.push_back( sm9 );

	Character miku;
	miku.Name( "miku" );
	charList.push_back( miku );

	charList.sort( []( Character iter1, Character iter2 )
	{
		return iter1.GetName() < iter2.GetName();
	});

	for ( auto iter : charList )
	{
		printf_s( "%s \n", iter.GetName().c_str() );
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	CharacterSortByName();

	getchar();
	return 0;
}

