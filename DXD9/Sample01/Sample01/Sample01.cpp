#include "stdafx.h"
#include "Sample01.h"
#include <mmsystem.h>
#include <d3dx9.h>
#include <strsafe.h>

LPDIRECT3D9				g_pD3D = nullptr;
LPDIRECT3DDEVICE9		g_pd3dDevice = nullptr;
LPDIRECT3DVERTEXBUFFER9	g_pVB = nullptr;

struct CUSTOMVERTEX
{
	FLOAT x , y , z;
	DWORD color;
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// struct CUSTOMVERTEX
// {
// 	FLOAT x , y , z , rhw;
// 	DWORD color;
// };
// #define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

HRESULT InitD3D( HWND hWnd )
{
	if ( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp , sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT , D3DDEVTYPE_HAL , hWnd , 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING , &d3dpp , &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE , D3DCULL_NONE );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	return S_OK;
}

HRESULT InitGeometry()
{
	CUSTOMVERTEX g_Vertices[] =
	{
		{ -1.0f , -1.0f , 0.0f , 0xffff0000 } ,
		{ 1.0f , -1.0f , 0.0f , 0xff0000ff } ,
		{ 0.0f , 1.0f , 0.0f , 0xffffffff }
	};

	if ( FAILED( g_pd3dDevice->CreateVertexBuffer( 3 * sizeof( CUSTOMVERTEX ) , 0 ,
		D3DFVF_CUSTOMVERTEX , D3DPOOL_DEFAULT , &g_pVB , NULL ) ) )
		return E_FAIL;

	VOID* pVertices;

	if ( FAILED( g_pVB->Lock( 0 , sizeof( g_Vertices ) , (void**)&pVertices , 0 ) ) )
		return E_FAIL;

	memcpy( pVertices , g_Vertices , sizeof( g_Vertices ) );
	g_pVB->Unlock();

	return S_OK;
}

/*
HRESULT InitVB()
{
	CUSTOMVERTEX Vertices[] =
	{
		{ 150.0f , 50.0f , 0.5f , 1.0f , 0xffff0000 } ,
		{ 250.0f , 250.0f , 0.5f , 1.0f , 0xff00ff00 } ,
		{ 50.0f , 250.0f , 0.5f , 1.0f , 0xff00ffff } ,
		{ 160.0f , 170.0f , 0.5f , 1.0f , 0xffff0000 } ,
		{ 200.0f , 250.0f , 0.5f , 1.0f , 0xff0000ff } ,
		{ 120.0f , 250.0f , 0.5f , 1.0f , 0xff00ff00 } ,
	};

	if ( FAILED( g_pd3dDevice->CreateVertexBuffer( 6 * sizeof( CUSTOMVERTEX ) ,
		0 , D3DFVF_CUSTOMVERTEX , D3DPOOL_DEFAULT , &g_pVB , NULL ) ) )
	{
		return E_FAIL;
	}

	VOID* pVertices;
	if ( FAILED( g_pVB->Lock( 0 , sizeof( Vertices ) , (void**)&pVertices , 0 ) ) )
		return E_FAIL;
	memcpy( pVertices , Vertices , sizeof( Vertices ) );
	g_pVB->Unlock();

	return S_OK;
}
*/

VOID Cleanup()
{
	if ( g_pVB != NULL )
		g_pVB->Release();

	if ( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if ( g_pD3D != NULL )
		g_pD3D->Release();
}

VOID SetupMatrices()
{
	D3DXMATRIXA16 matWorld;

	UINT iTime = timeGetTime() % 10000;
	FLOAT fAngle = iTime * ( 2.0f * D3DX_PI ) / 10000.0f;
	D3DXMatrixRotationY( &matWorld , fAngle );
	g_pd3dDevice->SetTransform( D3DTS_WORLD , &matWorld );

	D3DXVECTOR3 vEyePt( 0.0f , 3.0f , -5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f , 0.0f , 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f , 1.0f , 0.0f );

	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView , &vEyePt , &vLookatPt , &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW , &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj , D3DX_PI / 4 , 1.0f , 1.0f , 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION , &matProj );
}

VOID Render()
{
	if ( NULL == g_pd3dDevice )
		return;

	g_pd3dDevice->Clear( 0 , NULL , D3DCLEAR_TARGET , D3DCOLOR_XRGB( 30 , 10 , 10 ) , 1.0f , 0 );

	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		SetupMatrices();

		g_pd3dDevice->SetStreamSource( 0 , g_pVB , 0 , sizeof( CUSTOMVERTEX ) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP , 0 , 1 );

		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present( NULL , NULL , NULL , NULL );
}

// 4.
LRESULT WINAPI MsgProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	switch ( msg )
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd , msg , wParam , lParam );
}

INT WINAPI wWinMain( HINSTANCE hInst , HINSTANCE , LPWSTR , INT )
{
	UNREFERENCED_PARAMETER( hInst );

	//////////////////////////////////////////////////////////////////////////
	// 1. 
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ) , CS_CLASSDC , MsgProc , 0L , 0L , GetModuleHandle( NULL ) ,
		NULL , NULL , NULL , NULL , L"D3D Tutorial" , NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"D3D Tutorial" , L"D3D Tutorial 01 : CreateDevice" ,
		WS_OVERLAPPEDWINDOW , 100 , 100 , 300 , 300 , NULL , NULL , wc.hInstance , NULL );

	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		if ( SUCCEEDED( InitGeometry() ) )
		{
			ShowWindow( hWnd , SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			MSG msg;
			ZeroMemory( &msg , sizeof( msg ) );
			while ( msg.message != WM_QUIT )
			{
				if ( PeekMessage( &msg , NULL , 0U , 0U , PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				else
					Render();
			}
		}
	}

	UnregisterClass( L"D3D Tutorial" , wc.hInstance );
	return 0;
}