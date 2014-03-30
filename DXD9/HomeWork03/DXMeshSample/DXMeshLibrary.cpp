#include "stdafx.h"
#include "DXMeshLibrary.h"
#include <mmsystem.h>
#include <strsafe.h>

HRESULT DXMeshLibrary::InitD3D( HWND hWnd )
{
	if ( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
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

	if ( FAILED( m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	return S_OK;
}

HRESULT DXMeshLibrary::InitGeometry()
{
	LPD3DXBUFFER pD3DXMtrlBuffer;
	if ( FAILED( D3DXLoadMeshFromX( L"./girl.x", D3DXMESH_SYSTEMMEM, m_pd3dDevice,
		NULL, &pD3DXMtrlBuffer, NULL, &m_dwNumMaterials, &m_pMesh ) ) )
	{
		MessageBox( NULL, L"Could not find girl.x", L"D3D Tutorial", MB_OK );
		return E_FAIL;
	}

	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();

	m_pMeshMaterials = new D3DMATERIAL9[m_dwNumMaterials];
	if ( m_pMeshMaterials == NULL )
	{
		return E_OUTOFMEMORY;
	}

	m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];
	if ( m_pMeshTextures == NULL )
	{
		return E_OUTOFMEMORY;
	}

	if ( !( m_pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		//가지고 있지 않다면 메쉬를 복제하고 D3DFVF_NORMAL을 추가한다.
		ID3DXMesh* pTempMesh = 0;
		m_pMesh->CloneMeshFVF( D3DXMESH_MANAGED, m_pMesh->GetFVF() | D3DFVF_NORMAL, m_pd3dDevice, &pTempMesh );

		// 법선을 계산한다.
		D3DXComputeNormals( pTempMesh, 0 );

		m_pMesh->Release(); // 기존메쉬를 제거한다
		m_pMesh = pTempMesh; // 기존메쉬를 법선이 계산된 메쉬로 지정한다.
	}

	for ( DWORD i = 0; i < m_dwNumMaterials; ++i )
	{
		m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

		m_pMeshTextures[i] = NULL;

		if ( d3dxMaterials[i].pTextureFilename != NULL &&
			 lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
		{
			if ( FAILED( D3DXCreateTextureFromFileA( m_pd3dDevice,
				d3dxMaterials[i].pTextureFilename, &m_pMeshTextures[i] ) ) )
			{
				const CHAR* strPrefix = "./Resource/";
				CHAR strTexture[MAX_PATH];
				strcpy_s( strTexture, MAX_PATH, strPrefix );
				strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );

				if ( FAILED( D3DXCreateTextureFromFileA( m_pd3dDevice, strTexture, &m_pMeshTextures[i] ) ) )
				{
					MessageBox( NULL, L"Could not find texture map", L"D3D Tutorial", MB_OK );
				}
			}
		}
	}

	pD3DXMtrlBuffer->Release();

	return S_OK;
}

void DXMeshLibrary::Cleanup()
{
	if ( m_pMeshMaterials != NULL )
		delete[] m_pMeshMaterials;

	if ( m_pMeshTextures )
	{
		for ( DWORD i = 0; i < m_dwNumMaterials; ++i )
		{
			if ( m_pMeshTextures[i] )
			{
				m_pMeshTextures[i]->Release();
			}
		}
	}

	if ( m_pMesh )
	{
		m_pMesh->Release();
	}

	if ( m_pd3dDevice != NULL )
	{
		m_pd3dDevice->Release();
	}

	if ( m_pD3D != NULL )
	{
		m_pD3D->Release();
	}
}

void DXMeshLibrary::SetupMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );
	m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXVECTOR3 vEyept( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 1.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyept, &vLookatPt, &vUpVec );
	m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 3, 1.2f, 1.0f, 100.0f );
	m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

void DXMeshLibrary::SetupLights()
{
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	m_pd3dDevice->SetMaterial( &mtrl );

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
	m_pd3dDevice->SetLight( 0, &light );
	m_pd3dDevice->LightEnable( 0, true );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xFFA0A0A0 );
}

void DXMeshLibrary::Render()
{
	if ( NULL == m_pd3dDevice )
	{
		return;
	}

	m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 30, 10, 10 ), 1.0f, 0 );

	if ( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
	{
		SetupLights();

		SetupMatrices();

		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

		D3DXMATRIXA16 thisMatrix;

		D3DXMatrixTranslation( &thisMatrix, 0.0f, -10.0f, 30.0f );
		m_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );
		D3DXMatrixRotationY( &thisMatrix, timeGetTime() / 1000.0f );
		m_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );
		D3DXMatrixScaling( &thisMatrix, 1.0f, 1.2f, 1.0f );
		m_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &thisMatrix );


		for ( DWORD i = 0; i < m_dwNumMaterials; ++i )
		{
			m_pd3dDevice->SetMaterial( &m_pMeshMaterials[i] );
			m_pd3dDevice->SetTexture( 0, m_pMeshTextures[i] );
			m_pMesh->DrawSubset( i );
		}

		m_pd3dDevice->EndScene();
	}

	m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

void DXMeshLibrary::Create( int width, int height )
{
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, StaticProc, 0L, 0L, GetModuleHandle( NULL ),
		NULL, LoadCursor( NULL, IDC_ARROW ), NULL, NULL, L"MeshLibrary", NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"MeshLibrary", L"HomeWork 03 Mesh Library",
							  WS_OVERLAPPEDWINDOW, 100, 100, 100 + width, 100 + height, 
							  NULL, NULL, wc.hInstance, NULL );

	SetPropW( hWnd, L"MeshLibrary", ( HANDLE )this );

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
	
	UnregisterClass( L"MeshLibrary", wc.hInstance );
}

LRESULT CALLBACK DXMeshLibrary::StaticProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	DXMeshLibrary* dxMesh = (DXMeshLibrary*)GetPropW( hWnd, L"MeshLibrary" );

	if ( dxMesh )
	{
		return dxMesh->MsgProc( hWnd, msg, wParam, lParam );
	}
	else
	{
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}
}

LRESULT CALLBACK DXMeshLibrary::MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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