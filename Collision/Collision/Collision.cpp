// Collision.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Collision.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

const unsigned char VK_A = 0x41;
const unsigned char VK_B = 0x42;
const unsigned char VK_C = 0x43;
const unsigned char VK_D = 0x44;
const unsigned char VK_E = 0x45;
const unsigned char VK_F = 0x46;
const unsigned char VK_G = 0x47;
const unsigned char VK_H = 0x48;
const unsigned char VK_I = 0x49;
const unsigned char VK_J = 0x4A;
const unsigned char VK_K = 0x4B;
const unsigned char VK_L = 0x4C;
const unsigned char VK_M = 0x4D;
const unsigned char VK_N = 0x4E;
const unsigned char VK_O = 0x4F;
const unsigned char VK_P = 0x50;
const unsigned char VK_Q = 0x51;
const unsigned char VK_R = 0x52;
const unsigned char VK_S = 0x53;
const unsigned char VK_T = 0x54;
const unsigned char VK_U = 0x55;
const unsigned char VK_V = 0x56;
const unsigned char VK_W = 0x57;
const unsigned char VK_X = 0x58;
const unsigned char VK_Y = 0x59;
const unsigned char VK_Z = 0x5A;

LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
LPD3DXFONT				g_pFont = NULL;

BOOL	g_IsCollisionAABB = FALSE;
BOOL	g_IsCollisionOBB = FALSE;

struct CUSTOMVERTEX
{
	FLOAT x, y, z;
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct CollisionObject
{
	void SetPosition( float x, float y, float z )
	{
		m_EyePoint = { x, y, z };
		m_LookAtPoint = { x, y, z + 1.0f };
	}

	LPD3DXMESH		m_Mesh = nullptr;
	
	D3DXVECTOR3		m_MinVertexPos = { 0, 0, 0 };
	D3DXVECTOR3		m_MaxVertexPos = { 1, 1, 1 };

	D3DXVECTOR3		m_EyePoint = { 0, 0, 0 };
	D3DXVECTOR3		m_LookAtPoint = { 0, 0, 1.0f };
	D3DXVECTOR3		m_UpVector = { 0, 1.0f, 0 };

	D3DXVECTOR3		m_AxisDir[3];
	float			m_AxisLen[3];
};

std::vector<CollisionObject>	g_COList;

BOOL CheckCollisionAABB( D3DXVECTOR3* box1MinVertex, D3DXVECTOR3* box1MaxVertex,
							D3DXVECTOR3* box2MinVertex, D3DXVECTOR3* box2MaxVertex )
{
	if ( box1MinVertex->x <= box2MaxVertex->x && box1MaxVertex->x >= box2MinVertex->x &&
		 box1MinVertex->y <= box2MaxVertex->y && box1MaxVertex->y >= box2MinVertex->y &&
		 box1MinVertex->z <= box2MaxVertex->z && box1MaxVertex->z >= box2MinVertex->z )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CheckCollisionOBB( CollisionObject* colObject1, CollisionObject* cObject2 )
{
	double c[3][3] = { 0, };
	double absC[3][3] = { 0, };
	double d[3] = { 0, };
	double r0 = 0, r1 = 0, r = 0;
	int i = 0;
	const double cutoff = 0.999999;
	bool existsParallelPair = false;
	D3DXVECTOR3 diff = colObject1->m_EyePoint - cObject2->m_EyePoint;

	for ( i = 0; i < 3; ++i )
	{
		c[0][i] = D3DXVec3Dot( &colObject1->m_AxisDir[0], &cObject2->m_AxisDir[i] );
		absC[0][i] = abs( c[0][i] );
		if ( absC[0][i] > cutoff )
			existsParallelPair = true;
	}
	d[0] = D3DXVec3Dot( &diff, &colObject1->m_AxisDir[0] );
	r = abs( d[0] );
	r0 = colObject1->m_AxisLen[0];
	r1 = cObject2->m_AxisLen[0] * absC[0][0] + cObject2->m_AxisLen[1] * absC[0][1] + cObject2->m_AxisLen[2] * absC[0][2];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	for ( i = 0; i < 3; ++i )
	{
		c[1][i] = D3DXVec3Dot( &colObject1->m_AxisDir[1], &cObject2->m_AxisDir[i] );
		absC[1][i] = abs( c[1][i] );
		if ( absC[1][i] > cutoff )
		{
			existsParallelPair = true;
		}
	}
	d[1] = D3DXVec3Dot( &diff, &colObject1->m_AxisDir[1] );
	r = abs( d[1] );
	r0 = colObject1->m_AxisLen[1];
	r1 = cObject2->m_AxisLen[0] * absC[1][0] + cObject2->m_AxisLen[1] * absC[1][1] + cObject2->m_AxisLen[2] * absC[1][2];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}

	for ( i = 0; i < 3; ++i )
	{
		c[2][i] = D3DXVec3Dot( &colObject1->m_AxisDir[2], &cObject2->m_AxisDir[i] );
		absC[2][i] = abs( c[2][i] );
		if ( absC[2][i] > cutoff )
		{
			existsParallelPair = true;
		}
	}
	d[2] = D3DXVec3Dot( &diff, &colObject1->m_AxisDir[2] );
	r = abs( d[2] );
	r0 = colObject1->m_AxisLen[2];
	r1 = cObject2->m_AxisLen[0] * absC[2][0] + cObject2->m_AxisLen[1] * absC[2][1] + cObject2->m_AxisLen[2] * absC[2][2];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( D3DXVec3Dot( &diff, &cObject2->m_AxisDir[0] ) );
	r0 = colObject1->m_AxisLen[0] * absC[0][0] + colObject1->m_AxisLen[1] * absC[1][0] + colObject1->m_AxisLen[2] * absC[2][0];
	r1 = cObject2->m_AxisLen[0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( D3DXVec3Dot( &diff, &cObject2->m_AxisDir[1] ) );
	r0 = colObject1->m_AxisLen[0] * absC[0][1] + colObject1->m_AxisLen[1] * absC[1][1] + colObject1->m_AxisLen[2] * absC[2][1];
	r1 = cObject2->m_AxisLen[1];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( D3DXVec3Dot( &diff, &cObject2->m_AxisDir[2] ) );
	r0 = colObject1->m_AxisLen[0] * absC[0][2] + colObject1->m_AxisLen[1] * absC[1][2] + colObject1->m_AxisLen[2] * absC[2][2];
	r1 = cObject2->m_AxisLen[2];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	if ( existsParallelPair == true )
		return TRUE;

	r = abs( d[2] * c[1][0] - d[1] * c[2][0] );
	r0 = colObject1->m_AxisLen[1] * absC[2][0] + colObject1->m_AxisLen[2] * absC[1][0];
	r1 = cObject2->m_AxisLen[1] * absC[0][2] + cObject2->m_AxisLen[2] * absC[0][1];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[2] * c[1][1] - d[1] * c[2][1] );
	r0 = colObject1->m_AxisLen[1] * absC[2][1] + colObject1->m_AxisLen[2] * absC[1][1];
	r1 = cObject2->m_AxisLen[0] * absC[0][2] + cObject2->m_AxisLen[2] * absC[0][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[2] * c[1][2] - d[1] * c[2][2] );
	r0 = colObject1->m_AxisLen[1] * absC[2][2] + colObject1->m_AxisLen[2] * absC[1][2];
	r1 = cObject2->m_AxisLen[0] * absC[0][1] + cObject2->m_AxisLen[1] * absC[0][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[0] * c[2][0] - d[2] * c[0][0] );
	r0 = colObject1->m_AxisLen[0] * absC[2][0] + colObject1->m_AxisLen[2] * absC[0][0];
	r1 = cObject2->m_AxisLen[1] * absC[1][2] + cObject2->m_AxisLen[2] * absC[1][1];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[0] * c[2][1] - d[2] * c[0][1] );
	r0 = colObject1->m_AxisLen[0] * absC[2][1] + colObject1->m_AxisLen[2] * absC[0][1];
	r1 = cObject2->m_AxisLen[0] * absC[1][2] + cObject2->m_AxisLen[2] * absC[1][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[0] * c[2][2] - d[2] * c[0][2] );
	r0 = colObject1->m_AxisLen[0] * absC[2][2] + colObject1->m_AxisLen[2] * absC[0][2];
	r1 = cObject2->m_AxisLen[0] * absC[1][1] + cObject2->m_AxisLen[1] * absC[1][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[1] * c[0][0] - d[0] * c[1][0] );
	r0 = colObject1->m_AxisLen[0] * absC[1][0] + colObject1->m_AxisLen[1] * absC[0][0];
	r1 = cObject2->m_AxisLen[1] * absC[2][2] + cObject2->m_AxisLen[2] * absC[2][1];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[1] * c[0][1] - d[0] * c[1][1] );
	r0 = colObject1->m_AxisLen[0] * absC[1][1] + colObject1->m_AxisLen[1] * absC[0][1];
	r1 = cObject2->m_AxisLen[0] * absC[2][2] + cObject2->m_AxisLen[2] * absC[2][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	r = abs( d[1] * c[0][2] - d[0] * c[1][2] );
	r0 = colObject1->m_AxisLen[0] * absC[1][2] + colObject1->m_AxisLen[1] * absC[0][2];
	r1 = cObject2->m_AxisLen[0] * absC[2][1] + cObject2->m_AxisLen[1] * absC[2][0];
	if ( r > r0 + r1 )
	{
		return FALSE;
	}
	return TRUE;
}

HRESULT InitD3D( HWND hWnd )
{
	if ( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );

	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	D3DXCreateFont( g_pd3dDevice, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
					DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"돋음체", &g_pFont );
	
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	return S_OK;
}

HRESULT CreateBoxMesh( CollisionObject* colObject, float xSize, float ySize, float zSize )
{
	if ( xSize == 0 || ySize == 0 || zSize == 0 )
	{
		return S_FALSE;
	}

	D3DXCreateBox( g_pd3dDevice, xSize, ySize, zSize, &( colObject->m_Mesh ), NULL );

	D3DXVECTOR3 *pVertices;
	( colObject->m_Mesh )->LockVertexBuffer( D3DLOCK_READONLY, (void**)&pVertices );
	D3DXComputeBoundingBox( pVertices, ( colObject->m_Mesh )->GetNumVertices(),
							( colObject->m_Mesh )->GetNumBytesPerVertex(),
							&( colObject->m_MinVertexPos ), &( colObject->m_MaxVertexPos ) );
	( colObject->m_Mesh )->UnlockVertexBuffer();

	colObject->m_AxisLen[0] = xSize / 2;
	colObject->m_AxisLen[1] = ySize / 2;
	colObject->m_AxisLen[2] = zSize / 2;

	colObject->m_AxisDir[0] = { 1.0f, 0, 0 };
	colObject->m_AxisDir[1] = { 0, 1.0f, 0 };
	colObject->m_AxisDir[2] = { 0, 0, 1.0f };

	return S_OK;
}

HRESULT InitGeometry()
{
	CollisionObject character;
	HRESULT hr = S_FALSE;
	if ( S_OK != ( hr = CreateBoxMesh( &character, 1.0f, 1.0f, 1.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	g_COList.push_back( character );

	CollisionObject	colObject1;
	colObject1.SetPosition( 3.0f, 0.0f, 3.0f );
	if ( S_OK != ( hr = CreateBoxMesh( &colObject1, 2.0f, 2.0f, 1.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	g_COList.push_back( colObject1 );

	CollisionObject	colObject2;
	colObject2.SetPosition( -5.0f, 0.0f, 0.0f );
	if ( S_OK != ( hr = CreateBoxMesh( &colObject2, 3.0f, 3.0f, 3.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	g_COList.push_back( colObject2 );

	return S_OK;
}

VOID SetupMatrices()
{
	D3DXMATRIXA16 worldMatrix;
	D3DXMatrixIdentity( &worldMatrix );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix );

	D3DXVECTOR3 eyePoint( 0.0f, 5.0f, -30.0f );
	D3DXVECTOR3 lookAtPoint( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 upVector( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 viewMatrix;
	D3DXMatrixLookAtLH( &viewMatrix, &eyePoint, &lookAtPoint, &upVector );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &viewMatrix );

	D3DXMATRIXA16 projMatrix;
	D3DXMatrixPerspectiveFovLH( &projMatrix, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &projMatrix );
}

VOID Cleanup()
{
	for ( auto toBeDelete : g_COList )
	{
		if ( toBeDelete.m_Mesh )
		{
			toBeDelete.m_Mesh->Release();
		}
	}
	if ( g_pFont != NULL )
	{
		g_pFont->Release();
	}

	if ( g_pd3dDevice != NULL )
	{
		g_pd3dDevice->Release();
	}

	if ( g_pD3D != NULL )
	{
		g_pD3D->Release();
	}
}

VOID Update()
{
	static DWORD prevTime = GetTickCount();
	DWORD currentTime = GetTickCount();
	DWORD elapsedTime = currentTime - prevTime;
	prevTime = currentTime;

	if ( g_COList.empty() )
	{
		return;
	}
	if ( GetAsyncKeyState( VK_W ) )
	{
		g_COList[0].m_EyePoint.z += elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.z += elapsedTime * 0.01f;
	}
	if ( GetAsyncKeyState( VK_S ) )
	{
		g_COList[0].m_EyePoint.z -= elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.z += elapsedTime * 0.01f;
	}
	if ( GetAsyncKeyState( VK_A ) )
	{
		g_COList[0].m_EyePoint.x -= elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.x -= elapsedTime * 0.01f;
	}
	if ( GetAsyncKeyState( VK_D ) )
	{
		g_COList[0].m_EyePoint.x += elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.x += elapsedTime * 0.01f;
	}
	if ( GetAsyncKeyState( VK_C ) )
	{
		g_COList[0].m_EyePoint.y += elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.y += elapsedTime * 0.01f;
	}
	if ( GetAsyncKeyState( VK_V ) )
	{
		g_COList[0].m_EyePoint.y -= elapsedTime * 0.01f;
		g_COList[0].m_LookAtPoint.y -= elapsedTime * 0.01f;
	}

	if ( g_COList.size() < 2 )
	{
		return;
	}

	D3DXMATRIX matrix;
	D3DXMatrixTranslation( &matrix, g_COList[0].m_EyePoint.x, g_COList[0].m_EyePoint.y, g_COList[0].m_EyePoint.z );

	D3DXVECTOR3 box1minVertex;
	D3DXVECTOR3 box1maxVertex;

	D3DXVec3TransformCoord( &box1minVertex, &( g_COList[0].m_MinVertexPos ), &matrix );
	D3DXVec3TransformCoord( &box1maxVertex, &( g_COList[0].m_MaxVertexPos ), &matrix );

	for ( UINT i = 1; i < g_COList.size(); ++i )
	{
		D3DXMATRIX matrix;
		D3DXMatrixTranslation( &matrix, g_COList[i].m_EyePoint.x, g_COList[i].m_EyePoint.y, g_COList[i].m_EyePoint.z );

		D3DXVECTOR3 box2minVertex;
		D3DXVECTOR3 box2maxVertex;

		D3DXVec3TransformCoord( &box2minVertex, &( g_COList[i].m_MinVertexPos ), &matrix );
		D3DXVec3TransformCoord( &box2maxVertex, &( g_COList[i].m_MaxVertexPos ), &matrix );

		if ( TRUE == ( g_IsCollisionAABB = CheckCollisionAABB( &box1minVertex, &box1maxVertex, &box2minVertex, &box2maxVertex ) ) )
		{
			break;
		}
	}

	CollisionObject character = g_COList[0];

	for ( UINT i = 1; i < g_COList.size(); ++i )
	{
		CollisionObject colObject = g_COList[i];
		if ( TRUE == ( g_IsCollisionOBB = CheckCollisionOBB( &character, &colObject ) ) )
		{
			break;
		}
	}
}

VOID Render()
{
	if ( NULL == g_pd3dDevice )
	{
		return;
	}

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 30 , 0, 0 ), 1.0f, 0 );

	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		SetupMatrices();
		UINT	totalSize = g_COList.size();

		for ( auto iter : g_COList )
		{
			if ( iter.m_Mesh )
			{
				D3DXMATRIXA16 worldMatrix;
				D3DXMatrixIdentity( &worldMatrix );
				D3DXMatrixLookAtLH( &worldMatrix, &iter.m_EyePoint, &iter.m_LookAtPoint, &iter.m_UpVector );
				D3DXMatrixInverse( &worldMatrix, NULL, &worldMatrix );
				g_pd3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix );
				
				iter.m_Mesh->DrawSubset( 0 );
			}
		}
		g_pd3dDevice->EndScene();
	}

	RECT rt;
	SetRect( &rt, 10, 10, 0, 0 );
	if ( g_IsCollisionAABB )
	{
		g_pFont->DrawText( NULL, L"AABB : 충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}
	else
	{
		g_pFont->DrawText( NULL, L"AABB : 비충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}

	SetRect( &rt, 650, 10, 0, 0 );
	if ( g_IsCollisionOBB )
	{
		g_pFont->DrawText( NULL, L"OBB : 충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}
	else
	{
		g_pFont->DrawText( NULL, L"OBB : 비충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_DESTROY:
			Cleanup();
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

INT APIENTRY _tWinMain( _In_ HINSTANCE hInstance,
						_In_opt_ HINSTANCE hPrevInstance,
						_In_ LPTSTR    lpCmdLine,
						_In_ int       nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	UNREFERENCED_PARAMETER( nCmdShow );

	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ),
		CS_CLASSDC,
		MsgProc,
		0L,
		0L,
		GetModuleHandle( NULL ),
		NULL,
		NULL,
		NULL,
		NULL,
		L"Collision",
		NULL
	};

	// 윈도우 클래스 등록
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"Collision", L"Collision", WS_OVERLAPPEDWINDOW,
							  100, 100, 800, 800, GetDesktopWindow(), NULL, wc.hInstance, NULL );

	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		if ( SUCCEEDED( InitGeometry() ) )
		{
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			MSG msg;
			ZeroMemory( &msg, sizeof( msg ) );
			while ( msg.message != WM_QUIT )
			{
				if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				else
				{
					Update();
					Render();
				}
			}
		}
	}

	UnregisterClass( L"Collision", wc.hInstance );

	return 0;
}