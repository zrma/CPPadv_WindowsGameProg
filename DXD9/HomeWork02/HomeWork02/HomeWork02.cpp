#include "stdafx.h"
#include "HomeWork02.h"
#include <mmsystem.h>
#include <d3dx9.h>
#include <strsafe.h>

LPDIRECT3D9				g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pd3dDevice = NULL;
LPD3DXMESH				g_pMesh = NULL;
D3DMATERIAL9*			g_pMeshMaterials = NULL;
LPDIRECT3DTEXTURE9*		g_pMeshTextures = NULL;
DWORD					g_dwNumMaterials = 0L;

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
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	return S_OK;
}

HRESULT InitGeometry()
{
	LPD3DXBUFFER pD3DXMtrlBuffer;
	if ( FAILED( D3DXLoadMeshFromX( L"./Resource/girl.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice,
		NULL, &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh ) ) )
	{
		MessageBox( NULL, L"Could not find girl.x", L"D3D Tutorial", MB_OK );
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();

	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if ( g_pMeshMaterials == NULL )
	{
		return E_OUTOFMEMORY;
	}

	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if ( g_pMeshTextures == NULL )
	{
		return E_OUTOFMEMORY;
	}

	if ( !( g_pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		//가지고 있지 않다면 메쉬를 복제하고 D3DFVF_NORMAL을 추가한다.
		ID3DXMesh* pTempMesh = 0;
		g_pMesh->CloneMeshFVF( D3DXMESH_MANAGED, g_pMesh->GetFVF() | D3DFVF_NORMAL, g_pd3dDevice, &pTempMesh );

		// 법선을 계산한다.
		D3DXComputeNormals( pTempMesh, 0 );

		g_pMesh->Release(); // 기존메쉬를 제거한다
		g_pMesh = pTempMesh; // 기존메쉬를 법선이 계산된 메쉬로 지정한다.
	}

	for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
	{
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;

		if ( d3dxMaterials[i].pTextureFilename != NULL &&
			 lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
		{
			if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename, &g_pMeshTextures[i] ) ) )
			{
				const CHAR* strPrefix = "./Resource/";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );

				if ( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice, strTexture, &g_pMeshTextures[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"D3D Tutorial", MB_OK );
				}
			}
		}
	}

	pD3DXMtrlBuffer->Release();

	return S_OK;
}

VOID Cleanup()
{
	if ( g_pMeshMaterials != NULL )
		delete[] g_pMeshMaterials;

	if ( g_pMeshTextures )
	{
		for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
		{
			if ( g_pMeshTextures[i] )
			{
				g_pMeshTextures[i]->Release();
			}
		}
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

VOID SetupMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXVECTOR3 vEyept( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyept, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 3, 1.2f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

VOID SetupLights()
{
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pd3dDevice->SetMaterial( &mtrl );

	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	vecDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	light.Range = 1000.0f;
	g_pd3dDevice->SetLight( 0, &light );
	g_pd3dDevice->LightEnable( 0, true );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xFFA0A0A0 );
}

VOID Render()
{
	if ( NULL == g_pd3dDevice )
	{
		return;
	}

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 30, 10, 10 ), 1.0f, 0 );

	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		SetupLights();

		SetupMatrices();

		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

		D3DXMATRIXA16 thisMatrix;

		D3DXMatrixTranslation( &thisMatrix, 0.0f, -10.0f, 30.0f );
		g_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );
		D3DXMatrixRotationY( &thisMatrix, timeGetTime() / 1000.0f );
		g_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );
		D3DXMatrixScaling( &thisMatrix, 1.0f, 1.2f, 1.0f );
		g_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );


		for ( DWORD i = 0; i < g_dwNumMaterials; ++i )
		{
			g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
			g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );
			g_pMesh->DrawSubset( i );
		}

		g_pd3dDevice->EndScene();
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

INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	UNREFERENCED_PARAMETER( hInst );

	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle( NULL ),
		NULL, LoadCursor( NULL, IDC_ARROW ), NULL, NULL, L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"D3D Tutorial", L"HomeWork 02 FBX to .X Mesh Load",
							  WS_OVERLAPPEDWINDOW, 100, 100, 900, 600, NULL, NULL, wc.hInstance, NULL );

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
					Render();
			}
		}
	}

	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}