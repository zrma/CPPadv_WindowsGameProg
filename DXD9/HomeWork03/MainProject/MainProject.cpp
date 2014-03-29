// MainProject.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "DXMeshLibrary.h"

int _tmain(int argc, _TCHAR* argv[])
{
	DXMeshLibrary dxMesh;
	dxMesh.Create( 640, 480 );

	return 0;
}