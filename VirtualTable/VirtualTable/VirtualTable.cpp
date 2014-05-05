#include "stdafx.h"
#include <iostream>

class Base
{
public:
	virtual void mfunc()
	{
		std::cout << "Base::mfunc() " << std::endl;
	}
};

class Derived: public Base
{
public:
	virtual void mfunc()
	{
		std::cout << "Derived::mfunc() " << std::endl;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{

	typedef void ( Base::*bfunc )( );
	typedef void ( Derived::*dfunc )( );

	bfunc bb = &Base::mfunc;
	dfunc dd = &Derived::mfunc;

	if ( bb == dd )
	{
		std::cout << "called?" << std::endl;
	}

	Base b;
	( b.*bb )( );
	Derived d;
	( d.*bb )( );
	( d.*dd )( );

	getchar();
	return 0;
}

