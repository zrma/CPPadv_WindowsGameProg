// Collision.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Collision.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <timeapi.h>

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

struct CollisionObject
{
	void SetPosition( float x, float y, float z )
	{
		m_EyePoint = { x, y, z };
		m_LookAtPoint = { x, y, z + 1.0f };
		
		m_MinVertexPos += { x, y, z };
		m_MaxVertexPos += { x, y, z };

		for ( UINT i = 0; i < 8; ++i )
		{
			m_AABBVertex[i] += m_EyePoint;
		}
	}

	void MovePosition( float x, float y, float z )
	{
		m_EyePoint += { x, y, z };
		m_LookAtPoint += { x, y, z };

		m_MinVertexPos += { x, y, z };
		m_MaxVertexPos += { x, y, z };

		for ( UINT i = 0; i < 8; ++i )
		{
			m_AABBVertex[i] += { x, y, z };
		}
	}

	void RotateAxisX( float angle )
	{
		D3DXMATRIXA16 matrix;
		D3DXMatrixRotationX( &matrix, angle );

		D3DXVECTOR3 view = m_LookAtPoint - m_EyePoint;

		D3DXVec3TransformCoord( &view, &view, &matrix );
		m_LookAtPoint = m_EyePoint + view;

		D3DXVec3TransformCoord( &m_AxisDir[0], &m_AxisDir[0], &matrix );
		D3DXVec3Normalize( &m_AxisDir[0], &m_AxisDir[0] );
		D3DXVec3TransformCoord( &m_AxisDir[1], &m_AxisDir[1], &matrix );
		D3DXVec3Normalize( &m_AxisDir[1], &m_AxisDir[1] );
		D3DXVec3TransformCoord( &m_AxisDir[2], &m_AxisDir[2], &matrix );
		D3DXVec3Normalize( &m_AxisDir[2], &m_AxisDir[2] );

		for ( UINT i = 0; i < 8; ++i )
		{
			m_AABBVertex[i] -= m_EyePoint;
			D3DXVec3TransformCoord( &m_AABBVertex[i], &m_AABBVertex[i], &matrix );
			m_AABBVertex[i] += m_EyePoint;
		}
		SetAABBVertex();

		SetMeshCollisionBox();
	}

	void RotateAxisY( float angle )
	{
		D3DXMATRIXA16 matrix;
		D3DXMatrixRotationY( &matrix, angle );

		D3DXVECTOR3 view = m_LookAtPoint - m_EyePoint;

		D3DXVec3TransformCoord( &view, &view, &matrix );
		m_LookAtPoint = m_EyePoint + view;

		D3DXVec3TransformCoord( &m_AxisDir[0], &m_AxisDir[0], &matrix );
		D3DXVec3Normalize( &m_AxisDir[0], &m_AxisDir[0] );
		D3DXVec3TransformCoord( &m_AxisDir[1], &m_AxisDir[1], &matrix );
		D3DXVec3Normalize( &m_AxisDir[1], &m_AxisDir[1] );
		D3DXVec3TransformCoord( &m_AxisDir[2], &m_AxisDir[2], &matrix );
		D3DXVec3Normalize( &m_AxisDir[2], &m_AxisDir[2] );

		for ( UINT i = 0; i < 8; ++i )
		{
			m_AABBVertex[i] -= m_EyePoint;
			D3DXVec3TransformCoord( &m_AABBVertex[i], &m_AABBVertex[i], &matrix );
			m_AABBVertex[i] += m_EyePoint;
		}
		SetAABBVertex();

		SetMeshCollisionBox();
	}

	void RotateAxisZ( float angle )
	{
		D3DXMATRIXA16 matrix;
		D3DXMatrixRotationZ( &matrix, angle );

		D3DXVECTOR3 view = m_LookAtPoint - m_EyePoint;

		D3DXVec3TransformCoord( &view, &view, &matrix );
		m_LookAtPoint = m_EyePoint + view;

		D3DXVec3TransformCoord( &m_AxisDir[0], &m_AxisDir[0], &matrix );
		D3DXVec3Normalize( &m_AxisDir[0], &m_AxisDir[0] );
		D3DXVec3TransformCoord( &m_AxisDir[1], &m_AxisDir[1], &matrix );
		D3DXVec3Normalize( &m_AxisDir[1], &m_AxisDir[1] );
		D3DXVec3TransformCoord( &m_AxisDir[2], &m_AxisDir[2], &matrix );
		D3DXVec3Normalize( &m_AxisDir[2], &m_AxisDir[2] );

		for ( UINT i = 0; i < 8; ++i )
		{
			m_AABBVertex[i] -= m_EyePoint;
			D3DXVec3TransformCoord( &m_AABBVertex[i], &m_AABBVertex[i], &matrix );
			m_AABBVertex[i] += m_EyePoint;
		}
		SetAABBVertex();

		SetMeshCollisionBox();
	}

	void SetAABBVertex()
	{
		m_MinVertexPos = m_MaxVertexPos = m_AABBVertex[0];
		for ( UINT i = 1; i < 8; ++i )
		{
			m_MinVertexPos.x = min( m_MinVertexPos.x, m_AABBVertex[i].x );
			m_MaxVertexPos.x = max( m_MaxVertexPos.x, m_AABBVertex[i].x );
			m_MinVertexPos.y = min( m_MinVertexPos.y, m_AABBVertex[i].y );
			m_MaxVertexPos.y = max( m_MaxVertexPos.y, m_AABBVertex[i].y );
			m_MinVertexPos.z = min( m_MinVertexPos.z, m_AABBVertex[i].z );
			m_MaxVertexPos.z = max( m_MaxVertexPos.z, m_AABBVertex[i].z );
		}
	}

	void SetMeshCollisionBox()
	{
		if ( m_MeshCBox )
		{
			m_MeshCBox->Release();
		}
		D3DXCreateBox( g_pd3dDevice, 
					   m_MaxVertexPos.x - m_MinVertexPos.x, 
					   m_MaxVertexPos.y - m_MinVertexPos.y,
					   m_MaxVertexPos.z - m_MinVertexPos.z,
					   &m_MeshCBox , NULL );
	}

	LPD3DXMESH		m_Mesh = nullptr;
	LPD3DXMESH		m_MeshCBox = nullptr;
	
	D3DXVECTOR3		m_MinVertexPos = { 0, 0, 0 };
	D3DXVECTOR3		m_MaxVertexPos = { 1, 1, 1 };

	D3DXVECTOR3		m_AABBVertex[8];

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
	{
		return TRUE;
	}
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

	colObject->m_AxisLen[0] = ( colObject->m_MaxVertexPos.x - colObject->m_MinVertexPos.x ) / 2;
	colObject->m_AxisLen[1] = ( colObject->m_MaxVertexPos.y - colObject->m_MinVertexPos.y ) / 2;
	colObject->m_AxisLen[2] = ( colObject->m_MaxVertexPos.z - colObject->m_MinVertexPos.z ) / 2;

	colObject->m_AxisDir[0] = { 1.0f, 0, 0 };
	colObject->m_AxisDir[1] = { 0, 1.0f, 0 };
	colObject->m_AxisDir[2] = { 0, 0, 1.0f };

	colObject->m_AABBVertex[0] = { colObject->m_MinVertexPos.x, colObject->m_MinVertexPos.y, colObject->m_MinVertexPos.z };
	colObject->m_AABBVertex[1] = { colObject->m_MinVertexPos.x, colObject->m_MinVertexPos.y, colObject->m_MaxVertexPos.z };
	colObject->m_AABBVertex[2] = { colObject->m_MinVertexPos.x, colObject->m_MaxVertexPos.y, colObject->m_MinVertexPos.z };
	colObject->m_AABBVertex[3] = { colObject->m_MinVertexPos.x, colObject->m_MaxVertexPos.y, colObject->m_MaxVertexPos.z };
	colObject->m_AABBVertex[4] = { colObject->m_MaxVertexPos.x, colObject->m_MinVertexPos.y, colObject->m_MinVertexPos.z };
	colObject->m_AABBVertex[5] = { colObject->m_MaxVertexPos.x, colObject->m_MinVertexPos.y, colObject->m_MaxVertexPos.z };
	colObject->m_AABBVertex[6] = { colObject->m_MaxVertexPos.x, colObject->m_MaxVertexPos.y, colObject->m_MinVertexPos.z };
	colObject->m_AABBVertex[7] = { colObject->m_MaxVertexPos.x, colObject->m_MaxVertexPos.y, colObject->m_MaxVertexPos.z };

	colObject->SetAABBVertex();

	return S_OK;
}

HRESULT InitGeometry()
{
	CollisionObject character;
	HRESULT hr = S_FALSE;
	if ( S_OK != ( hr = CreateBoxMesh( &character, 1.0f, 1.0f, 3.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	g_COList.push_back( character );

	CollisionObject	colObject1;
	if ( S_OK != ( hr = CreateBoxMesh( &colObject1, 2.0f, 2.0f, 3.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	colObject1.SetPosition( 5.0f, 0.0f, 5.0f );
	colObject1.RotateAxisY( -60.0f );
	g_COList.push_back( colObject1 );

	CollisionObject	colObject2;
	if ( S_OK != ( hr = CreateBoxMesh( &colObject2, 3.0f, 3.0f, 3.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	colObject2.SetPosition( -5.0f, 0.0f, 0.0f );
	// colObject2.RotateAxisY( 40.0f );
	g_COList.push_back( colObject2 );

	CollisionObject	colObject3;
	if ( S_OK != ( hr = CreateBoxMesh( &colObject3, 4.0f, 2.0f, 1.0f ) ) )
	{
		MessageBox( NULL, L"Create Mesh Failed", L"Collision.exe", MB_OK );
		return hr;
	}
	colObject3.SetPosition( 0.0f, 0.0f, -4.0f );
	g_COList.push_back( colObject3 );

	return S_OK;
}

VOID SetupMatrices()
{
	D3DXMATRIXA16 worldMatrix;
	D3DXMatrixIdentity( &worldMatrix );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix );

	D3DXVECTOR3 eyePoint( 0.0f, 30.0f, 0.0f );
	D3DXVECTOR3 lookAtPoint( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 upVector( 0.0f, 0.0f, 1.0f );
	D3DXMATRIXA16 viewMatrix;
	D3DXMatrixLookAtLH( &viewMatrix, &eyePoint, &lookAtPoint, &upVector );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &viewMatrix );

	D3DXMATRIXA16 projMatrix;
	D3DXMatrixPerspectiveFovLH( &projMatrix, D3DX_PI / 6, 1.0f, 1.0f, 100.0f );
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

		if ( toBeDelete.m_MeshCBox )
		{
			toBeDelete.m_MeshCBox->Release();
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
		g_COList[0].MovePosition( 0, 0, elapsedTime * 0.01f );
	}
	if ( GetAsyncKeyState( VK_S ) )
	{
		g_COList[0].MovePosition( 0, 0, -( elapsedTime * 0.01f ) );
	}
	if ( GetAsyncKeyState( VK_A ) )
	{
		g_COList[0].MovePosition( -( elapsedTime * 0.01f ), 0, 0 );
	}
	if ( GetAsyncKeyState( VK_D ) )
	{
		g_COList[0].MovePosition( elapsedTime * 0.01f , 0, 0 );
	}
	if ( GetAsyncKeyState( VK_C ) )
	{
		g_COList[0].MovePosition( 0, elapsedTime * 0.01f, 0 );
	}
	if ( GetAsyncKeyState( VK_V ) )
	{
		g_COList[0].MovePosition( 0, -( elapsedTime * 0.01f ), 0 );
	}
	if ( GetAsyncKeyState( VK_Q ) )
	{
		g_COList[0].RotateAxisY( -( elapsedTime * 0.01f ) );
	}
	if ( GetAsyncKeyState( VK_E ) )
	{
		g_COList[0].RotateAxisY( elapsedTime * 0.01f );
	}
	if ( GetAsyncKeyState( VK_R ) )
	{
		g_COList[0].RotateAxisX( elapsedTime * 0.01f ); 
	}
	if ( GetAsyncKeyState( VK_F ) )
	{
		g_COList[0].RotateAxisX( -( elapsedTime * 0.01f ) );
	}
	if ( GetAsyncKeyState( VK_T ) )
	{
		g_COList[0].RotateAxisZ( elapsedTime * 0.01f );
	}
	if ( GetAsyncKeyState( VK_G ) )
	{
		g_COList[0].RotateAxisZ( -( elapsedTime * 0.01f ) ); 
	}

	if ( g_COList.size() < 2 )
	{
		return;
	}

	for ( UINT i = 1; i < g_COList.size(); ++i )
	{
		if ( TRUE == ( g_IsCollisionAABB = CheckCollisionAABB( 
			&( g_COList[0].m_MinVertexPos ), &( g_COList[0].m_MaxVertexPos ),
			&( g_COList[i].m_MinVertexPos ), &( g_COList[i].m_MaxVertexPos ) ) ) )
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

	if ( g_COList.size() > 2 )
	{
		g_COList[2].RotateAxisX( elapsedTime * 0.002f );
		g_COList[2].RotateAxisZ( elapsedTime * 0.002f );
		g_COList[3].RotateAxisY( elapsedTime * 0.001f );
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

			if ( iter.m_MeshCBox )
			{
				D3DXMATRIXA16 worldMatrix;
				D3DXMatrixIdentity( &worldMatrix );
				D3DXMatrixTranslation( &worldMatrix, iter.m_EyePoint.x, iter.m_EyePoint.y, iter.m_EyePoint.z );
				g_pd3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix );
				iter.m_MeshCBox->DrawSubset( 0 );
			}
		}
		g_pd3dDevice->EndScene();
	}

	if ( NULL == g_pFont )
	{
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		return;
	}

	RECT rt;
	SetRect( &rt, 10, 10, 0, 0 );
	g_pFont->DrawText( NULL, L"조작 : W A S D C V(이동) Q E R F T G(회전)", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	
	SetRect( &rt, 10, 30, 0, 0 );
	if ( g_IsCollisionAABB )
	{
		g_pFont->DrawText( NULL, L"AABB : 충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}
	else
	{
		g_pFont->DrawText( NULL, L"AABB : 비충돌", -1, &rt, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}

	SetRect( &rt, 650, 30, 0, 0 );
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

	AllocConsole();
	FILE* pFile;
	freopen_s( &pFile, "CONOUT$", "wb", stdout );

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

	FreeConsole();

	UnregisterClass( L"Collision", wc.hInstance );

	return 0;
}