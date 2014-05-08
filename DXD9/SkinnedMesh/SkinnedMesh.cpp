#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"

#define MESHFILENAME L"tiny\\tiny.x"

struct D3DXFRAME_DERIVED: public D3DXFRAME
{
	D3DXMATRIXA16 m_CombinedTransformationMatrix;
};

struct D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9*		m_Textures = nullptr;
	LPD3DXMESH				m_OrigMesh = nullptr;
	LPD3DXATTRIBUTERANGE	m_AttributeTable = nullptr;
	DWORD					m_NumAttributeGroups = 0;
	DWORD					m_NumInfl = 0;
	LPD3DXBUFFER			m_BoneCombinationBuf = nullptr;
	D3DXMATRIX**			m_BoneMatrixPtrs = nullptr;
	D3DXMATRIX*				m_BoneOffsetMatrices = nullptr;
	DWORD					m_NumPaletteEntries = 0;
};

class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
	// Create
	STDMETHOD( CreateFrame )( THIS_ LPCSTR name, LPD3DXFRAME* newFrame );
	STDMETHOD( CreateMeshContainer )( THIS_ LPCSTR name, CONST D3DXMESHDATA* meshData,
									  CONST D3DXMATERIAL* materials, CONST D3DXEFFECTINSTANCE* effectInstances, DWORD numMaterials,
									  CONST DWORD* adjacency, LPD3DXSKININFO skinInfo, LPD3DXMESHCONTAINER* newMeshContainer );
	// Destroy
	STDMETHOD( DestroyFrame )( THIS_ LPD3DXFRAME frameToDelete );
	STDMETHOD( DestroyMeshContainer )( THIS_ LPD3DXMESHCONTAINER meshContainerBase );

	CAllocateHierarchy() {}
};

ID3DXSprite*                g_TextSprite = nullptr;
ID3DXEffect*                g_Effect = nullptr;
LPD3DXFRAME                 g_FrameRoot = nullptr;
ID3DXAnimationController*   g_AnimController = nullptr;
D3DXMATRIXA16*              g_BoneMatrices = nullptr;
UINT                        g_BoneMatricesMaxNumber = 0;

D3DXMATRIXA16               g_ViewMatrix;
D3DXMATRIXA16               g_ProjMatrix;
D3DXMATRIXA16               g_ProjMatrixTranspose;


bool	CALLBACK IsDeviceAcceptable( D3DCAPS9* caps, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, bool windowed, void* userContext );
HRESULT	CALLBACK OnCreateDevice( IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext );
HRESULT	CALLBACK OnResetDevice( IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext );
void	CALLBACK OnFrameMove( double time, float elapsedTime, void* userContext );
void	CALLBACK OnFrameRender( IDirect3DDevice9* d3dDevice, double time, float elapsedTime, void* userContext );
void	CALLBACK OnLostDevice( void* userContext );
void	CALLBACK OnDestroyDevice( void* userContext );

HRESULT		LoadMesh( IDirect3DDevice9* d3dDevice, WCHAR* fileName, ID3DXMesh** mesh );
void		DrawMeshContainer( IDirect3DDevice9* d3dDevice, LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase );
void		DrawFrame( IDirect3DDevice9* d3dDevice, LPD3DXFRAME frame );
HRESULT		SetupBoneMatrixPointersOnMesh( LPD3DXMESHCONTAINER meshContainer );
HRESULT		SetupBoneMatrixPointers( LPD3DXFRAME frame );
void		UpdateFrameMatrices( LPD3DXFRAME frameBase, LPD3DXMATRIX parentMatrix );
void		UpdateSkinningMethod( LPD3DXFRAME frameBase );
HRESULT		GenerateSkinnedMesh( IDirect3DDevice9* d3dDevice, D3DXMESHCONTAINER_DERIVED* meshContainer );
void		ReleaseAttributeTable( LPD3DXFRAME frameBase );


HRESULT AllocateName( LPCSTR name, LPSTR* newName )
{
	UINT length = 0;

	if ( name != nullptr )
	{
		length = (UINT)strlen( name ) + 1;
		*newName = new CHAR[length];
		if ( *newName == nullptr )
		{
			return E_OUTOFMEMORY;
		}
		memcpy( *newName, name, length * sizeof( CHAR ) );
	}
	else
	{
		*newName = nullptr;
	}

	return S_OK;
}

HRESULT CAllocateHierarchy::CreateFrame( LPCSTR name, LPD3DXFRAME* newFrame )
{
	HRESULT hr = S_OK;
	D3DXFRAME_DERIVED* frame = nullptr;

	*newFrame = nullptr;

	frame = (D3DXFRAME_DERIVED*)_aligned_malloc( sizeof( D3DXFRAME_DERIVED ), 16 );
	new (frame)D3DXFRAME_DERIVED();

	if ( frame == nullptr )
	{
		return ( hr = E_OUTOFMEMORY );
	}

	hr = AllocateName( name, &frame->Name );
	if ( FAILED( hr ) )
	{
		SAFE_DELETE( frame );
		return hr;
	}

	D3DXMatrixIdentity( &frame->TransformationMatrix );
	D3DXMatrixIdentity( &frame->m_CombinedTransformationMatrix );

	frame->pMeshContainer = nullptr;
	frame->pFrameSibling = nullptr;
	frame->pFrameFirstChild = nullptr;

	*newFrame = frame;
	frame = nullptr;

	return hr;
}


HRESULT CAllocateHierarchy::CreateMeshContainer( LPCSTR name, CONST D3DXMESHDATA *meshData, CONST D3DXMATERIAL *materials, 
												 CONST D3DXEFFECTINSTANCE *effectInstances, DWORD numMaterials, CONST DWORD *adjacency, 
												 LPD3DXSKININFO skinInfo, LPD3DXMESHCONTAINER *newMeshContainer )
{
	HRESULT hr = S_OK;
	LPDIRECT3DDEVICE9 d3dDevice = nullptr;
	D3DXMESHCONTAINER_DERIVED *meshContainer = nullptr;
	UINT numFaces = 0;
	UINT material = 0;
	UINT bone = 0;
	UINT bones = 0;

	LPD3DXMESH mesh = nullptr;
	*newMeshContainer = nullptr;

	if ( D3DXMESHTYPE_MESH != meshData->Type )
	{
		SAFE_RELEASE( d3dDevice );

		if ( nullptr != meshContainer )
		{
			DestroyMeshContainer( meshContainer );
		}

		return ( hr = E_FAIL );
	}

	mesh = meshData->pMesh;

	if ( 0 == mesh->GetFVF() )
	{
		SAFE_RELEASE( d3dDevice );

		if ( nullptr != meshContainer )
		{
			DestroyMeshContainer( meshContainer );
		}

		return ( hr = E_FAIL );
	}

	meshContainer = new D3DXMESHCONTAINER_DERIVED();
	if ( nullptr == meshContainer )
	{
		SAFE_RELEASE( d3dDevice );

		if ( nullptr != meshContainer )
		{
			DestroyMeshContainer( meshContainer );
		}

		return ( hr = E_OUTOFMEMORY );
	}
	memset( meshContainer, 0, sizeof( D3DXMESHCONTAINER_DERIVED ) );

	if ( FAILED( hr = AllocateName( name, &meshContainer->Name ) ) )
	{
		SAFE_RELEASE( d3dDevice );

		if ( nullptr != meshContainer )
		{
			DestroyMeshContainer( meshContainer );
		}

		return hr;
	}

	mesh->GetDevice( &d3dDevice );
	numFaces = mesh->GetNumFaces();

	if ( !( mesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		meshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

		if ( FAILED( hr = mesh->CloneMeshFVF( mesh->GetOptions(), mesh->GetFVF() | D3DFVF_NORMAL, d3dDevice, &meshContainer->MeshData.pMesh ) ) )
		{
			SAFE_RELEASE( d3dDevice );

			if ( nullptr != meshContainer )
			{
				DestroyMeshContainer( meshContainer );
			}

			return hr;
		}

		mesh = meshContainer->MeshData.pMesh;
		D3DXComputeNormals( mesh, NULL );
	}
	else
	{
		meshContainer->MeshData.pMesh = mesh;
		meshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

		mesh->AddRef();
	}

	meshContainer->NumMaterials = max( 1, numMaterials );
	meshContainer->pMaterials = new D3DXMATERIAL[meshContainer->NumMaterials];
	meshContainer->m_Textures = new LPDIRECT3DTEXTURE9[meshContainer->NumMaterials];
	meshContainer->pAdjacency = new DWORD[numFaces * 3];
	if ( ( nullptr == meshContainer->pAdjacency ) || ( nullptr == meshContainer->pMaterials ) )
	{
		SAFE_RELEASE( d3dDevice );

		if ( nullptr != meshContainer )
		{
			DestroyMeshContainer( meshContainer );
		}

		return ( hr = E_OUTOFMEMORY );
	}

	memcpy( meshContainer->pAdjacency, adjacency, sizeof(DWORD)* numFaces * 3 );
	memset( meshContainer->m_Textures, 0, sizeof(LPDIRECT3DTEXTURE9)* meshContainer->NumMaterials );

	if ( numMaterials > 0 )
	{
		memcpy( meshContainer->pMaterials, materials, sizeof(D3DXMATERIAL)* numMaterials );

		for ( material = 0; material < numMaterials; material++ )
		{
			if ( nullptr != meshContainer->pMaterials[material].pTextureFilename )
			{
				WCHAR strTexturePath[MAX_PATH];
				ZeroMemory( strTexturePath, sizeof( strTexturePath ) );

				WCHAR wszBuf[MAX_PATH];
				ZeroMemory( wszBuf, sizeof( wszBuf ) );

				MultiByteToWideChar( CP_ACP, 0, meshContainer->pMaterials[material].pTextureFilename, -1, wszBuf, MAX_PATH );
				wszBuf[MAX_PATH - 1] = L'\0';

				DXUTFindDXSDKMediaFileCch( strTexturePath, MAX_PATH, wszBuf );
				if ( FAILED( D3DXCreateTextureFromFile( d3dDevice, strTexturePath, &meshContainer->m_Textures[material] ) ) )
				{
					meshContainer->m_Textures[material] = nullptr;
				}

				meshContainer->pMaterials[material].pTextureFilename = nullptr;
			}
		}
	}
	else
	{
		meshContainer->pMaterials[0].pTextureFilename = nullptr;
		memset( &meshContainer->pMaterials[0].MatD3D, 0, sizeof( D3DMATERIAL9 ) );
		meshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		meshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		meshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		meshContainer->pMaterials[0].MatD3D.Specular = meshContainer->pMaterials[0].MatD3D.Diffuse;
	}

	if ( nullptr != skinInfo )
	{
		meshContainer->pSkinInfo = skinInfo;
		skinInfo->AddRef();

		meshContainer->m_OrigMesh = mesh;
		mesh->AddRef();

		bones = skinInfo->GetNumBones();
		meshContainer->m_BoneOffsetMatrices = new D3DXMATRIX[bones];
		if ( nullptr == meshContainer->m_BoneOffsetMatrices )
		{
			SAFE_RELEASE( d3dDevice );

			if ( nullptr != meshContainer )
			{
				DestroyMeshContainer( meshContainer );
			}

			return ( hr = E_OUTOFMEMORY );
		}

		for ( bone = 0; bone < bones; ++bone )
		{
			meshContainer->m_BoneOffsetMatrices[bone] = *( meshContainer->pSkinInfo->GetBoneOffsetMatrix( bone ) );
		}

		if ( FAILED( hr = GenerateSkinnedMesh( d3dDevice, meshContainer ) ) )
		{
			SAFE_RELEASE( d3dDevice );

			if ( nullptr != meshContainer )
			{
				DestroyMeshContainer( meshContainer );
			}

			return ( hr = E_FAIL );
		}
	}

	*newMeshContainer = meshContainer;

	SAFE_RELEASE( d3dDevice );
	return hr;
}


HRESULT CAllocateHierarchy::DestroyFrame( LPD3DXFRAME frameToDelete )
{
	SAFE_DELETE_ARRAY( frameToDelete->Name );
	
	frameToDelete->~D3DXFRAME();
	_aligned_free( frameToDelete );

	return S_OK;
}

HRESULT CAllocateHierarchy::DestroyMeshContainer( LPD3DXMESHCONTAINER meshContainerBase )
{
	D3DXMESHCONTAINER_DERIVED* meshContainer = (D3DXMESHCONTAINER_DERIVED*)meshContainerBase;

	SAFE_DELETE_ARRAY( meshContainer->Name );
	SAFE_DELETE_ARRAY( meshContainer->pAdjacency );
	SAFE_DELETE_ARRAY( meshContainer->pMaterials );
	SAFE_DELETE_ARRAY( meshContainer->m_BoneOffsetMatrices );

	if ( nullptr != meshContainer->m_Textures )
	{
		for ( UINT material = 0; material < meshContainer->NumMaterials; ++material )
		{
			SAFE_RELEASE( meshContainer->m_Textures[material] );
		}
	}

	SAFE_DELETE_ARRAY( meshContainer->m_Textures );
	SAFE_DELETE_ARRAY( meshContainer->m_BoneMatrixPtrs );
	SAFE_RELEASE( meshContainer->m_BoneCombinationBuf );
	SAFE_RELEASE( meshContainer->MeshData.pMesh );
	SAFE_RELEASE( meshContainer->pSkinInfo );
	SAFE_RELEASE( meshContainer->m_OrigMesh );
	SAFE_DELETE( meshContainer );

	return S_OK;
}

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
	DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTCreateWindow( L"Skinned Mesh" );
	DXUTInit( true, true );
	DXUTCreateDevice( true, 640, 480 );
	DXUTMainLoop();

	ReleaseAttributeTable( g_FrameRoot );
	SAFE_DELETE_ARRAY( g_BoneMatrices );

	return DXUTGetExitCode();
}

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* caps, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, bool windowed, void* userContext )
{
	IDirect3D9* d3d9Object = DXUTGetD3D9Object();

	if ( FAILED( d3d9Object->CheckDeviceFormat( caps->AdapterOrdinal, caps->DeviceType, adapterFormat,
		D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, backBufferFormat ) ) )
	{
		return false;
	}

	if ( caps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
	{
		return false;
	}

	return true;
}

HRESULT GenerateSkinnedMesh( IDirect3DDevice9* d3dDevice, D3DXMESHCONTAINER_DERIVED* meshContainer )
{
	HRESULT hr = S_OK;
	D3DCAPS9 d3dCaps;
	d3dDevice->GetDeviceCaps( &d3dCaps );

	if ( meshContainer->pSkinInfo == NULL )
	{
		return hr;
	}
	SAFE_RELEASE( meshContainer->MeshData.pMesh );
	SAFE_RELEASE( meshContainer->m_BoneCombinationBuf );

	UINT maxMatrices = 26;
	meshContainer->m_NumPaletteEntries = min( maxMatrices, meshContainer->pSkinInfo->GetNumBones() );

	DWORD flags = D3DXMESHOPT_VERTEXCACHE;

	if ( d3dCaps.VertexShaderVersion >= D3DVS_VERSION( 1, 1 ) )
	{
		flags |= D3DXMESH_MANAGED;
	}
	else
	{
		flags |= D3DXMESH_SYSTEMMEM;
	}

	SAFE_RELEASE( meshContainer->MeshData.pMesh );

	if ( FAILED( hr = meshContainer->pSkinInfo->ConvertToIndexedBlendedMesh( meshContainer->m_OrigMesh, flags,
		meshContainer->m_NumPaletteEntries, meshContainer->pAdjacency, NULL, NULL, NULL,
		&meshContainer->m_NumInfl, &meshContainer->m_NumAttributeGroups, &meshContainer->m_BoneCombinationBuf, &meshContainer->MeshData.pMesh ) ) )
	{
		return hr;
	}

	DWORD newFVF = ( meshContainer->MeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK ) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
	if ( newFVF != meshContainer->MeshData.pMesh->GetFVF() )
	{
		LPD3DXMESH pMesh = nullptr;
		if ( !FAILED( hr = meshContainer->MeshData.pMesh->CloneMeshFVF( meshContainer->MeshData.pMesh->GetOptions(), newFVF, d3dDevice, &pMesh ) ) )
		{
			meshContainer->MeshData.pMesh->Release();
			meshContainer->MeshData.pMesh = pMesh;
			pMesh = NULL;
		}
	}

	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
	ZeroMemory( decl, sizeof( decl ) );

	LPD3DVERTEXELEMENT9 declCur = nullptr;
	if ( FAILED( hr = meshContainer->MeshData.pMesh->GetDeclaration( decl ) ) )
	{
		return hr;
	}

	declCur = decl;

	while ( declCur->Stream != 0xff )
	{
		if ( ( declCur->Usage == D3DDECLUSAGE_BLENDINDICES ) && ( declCur->UsageIndex == 0 ) )
		{
			declCur->Type = D3DDECLTYPE_D3DCOLOR;
		}

		declCur++;
	}

	if ( FAILED( hr = meshContainer->MeshData.pMesh->UpdateSemantics( decl ) ) )
	{
		return hr;
	}

	if ( g_BoneMatricesMaxNumber < meshContainer->pSkinInfo->GetNumBones() )
	{
		g_BoneMatricesMaxNumber = meshContainer->pSkinInfo->GetNumBones();

		SAFE_DELETE_ARRAY( g_BoneMatrices );

		g_BoneMatrices = new D3DXMATRIXA16[g_BoneMatricesMaxNumber];
		if ( nullptr == g_BoneMatrices )
		{
			return hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}


HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext )
{
	HRESULT hr = S_OK;
	CAllocateHierarchy alloc;

	DWORD shaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	shaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	shaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	WCHAR str[MAX_PATH];
	ZeroMemory( str, sizeof( str ) );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"SkinnedMesh.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( d3dDevice, str, NULL, NULL, shaderFlags, NULL, &g_Effect, NULL ) );
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, MESHFILENAME ) );

	WCHAR strPath[MAX_PATH];
	ZeroMemory( strPath, sizeof( strPath ) );

	wcscpy_s( strPath, MAX_PATH, str );
	WCHAR* lastSlash = wcsrchr( strPath, L'\\' );

	if ( lastSlash )
	{
		*lastSlash = 0;
		++lastSlash;
	}
	else
	{
		wcscpy_s( strPath, MAX_PATH, L"." );
		lastSlash = str;
	}

	WCHAR strCWD[MAX_PATH];
	ZeroMemory( strCWD, sizeof( strCWD ) );

	GetCurrentDirectory( MAX_PATH, strCWD );
	SetCurrentDirectory( strPath );

	V_RETURN( D3DXLoadMeshHierarchyFromX( lastSlash, D3DXMESH_MANAGED, d3dDevice, &alloc, NULL, &g_FrameRoot, &g_AnimController ) );
	V_RETURN( SetupBoneMatrixPointers( g_FrameRoot ) );

	SetCurrentDirectory( strCWD );

	return S_OK;
}


HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* d3dDevice, const D3DSURFACE_DESC* backBufferSurfaceDesc, void* userContext )
{
	HRESULT hr = S_OK;

	if ( g_Effect )
	{
		V_RETURN( g_Effect->OnResetDevice() );
	}

	V_RETURN( D3DXCreateSprite( d3dDevice, &g_TextSprite ) );

	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	d3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
	d3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	d3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	d3dDevice->SetRenderState( D3DRS_AMBIENT, 0x33333333 );
	d3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	d3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	d3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	DWORD shaderFlags = 0;

#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DXSHADER_DEBUG;
#endif

#if defined(DEBUG_VS) || defined(DEBUG_PS)
	shaderFlags |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPVALIDATION;
#endif

	float aspect = (float)backBufferSurfaceDesc->Width / (float)backBufferSurfaceDesc->Height;

	D3DXMatrixPerspectiveFovLH( &g_ProjMatrix, D3DX_PI / 4, aspect, 1.0f, 2000.0f );
	d3dDevice->SetTransform( D3DTS_PROJECTION, &g_ProjMatrix );

	D3DXMatrixTranspose( &g_ProjMatrixTranspose, &g_ProjMatrix );

	return S_OK;
}

void CALLBACK OnFrameMove( double time, float elapsedTime, void* userContext )
{
	IDirect3DDevice9* d3dDevice = DXUTGetD3D9Device();

	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );

	D3DXMatrixTranslation( &matWorld, 0, 0, 0 );
	d3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXVECTOR3 eyePoint( 0, 0, -1000 );
	D3DXVECTOR3 lookAtPoint( 0, 0, 0 );
	D3DXVECTOR3 upVector( 0, 1, 0 );

	D3DXMatrixLookAtLH( &g_ViewMatrix, &eyePoint, &lookAtPoint, &upVector );
	d3dDevice->SetTransform( D3DTS_VIEW, &g_ViewMatrix );

	if ( nullptr != g_AnimController )
	{
		g_AnimController->AdvanceTime( elapsedTime, NULL );
	}

	UpdateFrameMatrices( g_FrameRoot, &matWorld );
}


void DrawMeshContainer( IDirect3DDevice9* d3dDevice, LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase )
{
	HRESULT hr = S_OK;
	D3DXMESHCONTAINER_DERIVED* meshContainer = (D3DXMESHCONTAINER_DERIVED*)meshContainerBase;
	D3DXFRAME_DERIVED* frame = (D3DXFRAME_DERIVED*)frameBase;
	
	LPD3DXBONECOMBINATION boneComb = nullptr;
	UINT matrixIndex = 0;

	D3DXMATRIXA16 matTemp;
	D3DXMatrixIdentity( &matTemp );

	D3DCAPS9 d3dCaps;
	d3dDevice->GetDeviceCaps( &d3dCaps );

	if ( nullptr != meshContainer->pSkinInfo )
	{
		boneComb = reinterpret_cast<LPD3DXBONECOMBINATION>( meshContainer->m_BoneCombinationBuf->GetBufferPointer() );

		for ( UINT attrib = 0; attrib < meshContainer->m_NumAttributeGroups; ++attrib )
		{
			for ( UINT paletteEntry = 0; paletteEntry < meshContainer->m_NumPaletteEntries; ++paletteEntry )
			{
				matrixIndex = boneComb[attrib].BoneId[paletteEntry];
				if ( UINT_MAX != matrixIndex )
				{
					D3DXMatrixMultiply( &matTemp, &meshContainer->m_BoneOffsetMatrices[matrixIndex], meshContainer->m_BoneMatrixPtrs[matrixIndex] );
					D3DXMatrixMultiply( &g_BoneMatrices[paletteEntry], &matTemp, &g_ViewMatrix );
				}
			}
			V( g_Effect->SetMatrixArray( "mWorldMatrixArray", g_BoneMatrices, meshContainer->m_NumPaletteEntries ) );

			D3DXCOLOR color1( meshContainer->pMaterials[boneComb[attrib].AttribId].MatD3D.Ambient );
			D3DXCOLOR color2( 0.25, 0.25, 0.25, 1.0 );
			D3DXCOLOR ambEmm;
			D3DXColorModulate( &ambEmm, &color1, &color2 );
			ambEmm += D3DXCOLOR( meshContainer->pMaterials[boneComb[attrib].AttribId].MatD3D.Emissive );

			V( g_Effect->SetVector( "MaterialDiffuse", (D3DXVECTOR4*)&( meshContainer->pMaterials[boneComb[attrib].AttribId].MatD3D.Diffuse ) ) );
			V( g_Effect->SetVector( "MaterialAmbient", (D3DXVECTOR4*)&ambEmm ) );

			V( d3dDevice->SetTexture( 0, meshContainer->m_Textures[boneComb[attrib].AttribId] ) );

			V( g_Effect->SetInt( "CurNumBones", meshContainer->m_NumInfl - 1 ) );

			UINT numPasses = 0;

			V( g_Effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE ) );

			for ( UINT iPass = 0; iPass < numPasses; ++iPass )
			{
				V( g_Effect->BeginPass( iPass ) );
				V( meshContainer->MeshData.pMesh->DrawSubset( attrib ) );
				V( g_Effect->EndPass() );
			}

			V( g_Effect->End() );

			V( d3dDevice->SetVertexShader( NULL ) );
		}
	}
	else
	{
		V( d3dDevice->SetTransform( D3DTS_WORLD, &frame->m_CombinedTransformationMatrix ) );

		for ( UINT material = 0; material < meshContainer->NumMaterials; ++material )
		{
			V( d3dDevice->SetMaterial( &meshContainer->pMaterials[material].MatD3D ) );
			V( d3dDevice->SetTexture( 0, meshContainer->m_Textures[material] ) );
			V( meshContainer->MeshData.pMesh->DrawSubset( material ) );
		}
	}
}

void DrawFrame( IDirect3DDevice9* d3dDevice, LPD3DXFRAME frame )
{
	LPD3DXMESHCONTAINER meshContainer = nullptr;

	meshContainer = frame->pMeshContainer;
	while ( nullptr != meshContainer )
	{
		DrawMeshContainer( d3dDevice, meshContainer, frame );
		meshContainer = meshContainer->pNextMeshContainer;
	}

	if ( nullptr != frame->pFrameSibling )
	{
		DrawFrame( d3dDevice, frame->pFrameSibling );
	}

	if ( frame->pFrameFirstChild != NULL )
	{
		DrawFrame( d3dDevice, frame->pFrameFirstChild );
	}
}


void CALLBACK OnFrameRender( IDirect3DDevice9* d3dDevice, double time, float elapsedTime, void* userContext )
{
	HRESULT hr = S_OK;

	d3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 66, 75, 121 ), 1.0f, 0L );

	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof( D3DLIGHT9 ) );

	D3DXVECTOR3 lightDirUnnormalizedVector( 0.0f, -1.0f, 1.0f );

	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;

	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &lightDirUnnormalizedVector );

	light.Position.x = 0.0f;
	light.Position.y = -1.0f;
	light.Position.z = 1.0f;

	light.Range = 1000.0f;

	V( d3dDevice->SetLight( 0, &light ) );
	V( d3dDevice->LightEnable( 0, TRUE ) );

	V( g_Effect->SetMatrix( "mViewProj", &g_ProjMatrix ) );

	D3DXVECTOR4 LightDirVector( 0.0f, 1.0f, -1.0f, 0.0f );
	D3DXVec4Normalize( &LightDirVector, &LightDirVector );

	V( d3dDevice->SetVertexShaderConstantF( 1, (float*)&LightDirVector, 1 ) );
	V( g_Effect->SetVector( "lhtDir", &LightDirVector ) );

	if ( SUCCEEDED( d3dDevice->BeginScene() ) )
	{
		DrawFrame( d3dDevice, g_FrameRoot );

		d3dDevice->EndScene();
	}
}


void CALLBACK OnLostDevice( void* userContext )
{
	if ( g_Effect )
	{
		g_Effect->OnLostDevice();
	}

	SAFE_RELEASE( g_TextSprite );
}


void CALLBACK OnDestroyDevice( void* userContext )
{
	SAFE_RELEASE( g_Effect );

	CAllocateHierarchy alloc;
	D3DXFrameDestroy( g_FrameRoot, &alloc );

	SAFE_RELEASE( g_AnimController );
}


HRESULT SetupBoneMatrixPointersOnMesh( LPD3DXMESHCONTAINER meshContainerBase )
{
	UINT bones = 0;
	D3DXFRAME_DERIVED* frame = nullptr;

	D3DXMESHCONTAINER_DERIVED* meshContainer = (D3DXMESHCONTAINER_DERIVED*)meshContainerBase;

	if ( nullptr != meshContainer->pSkinInfo )
	{
		bones = meshContainer->pSkinInfo->GetNumBones();
		meshContainer->m_BoneMatrixPtrs = new D3DXMATRIX*[bones];

		if ( nullptr == meshContainer->m_BoneMatrixPtrs )
		{
			return E_OUTOFMEMORY;
		}

		for ( UINT iBone = 0; iBone < bones; iBone++ )
		{
			if ( nullptr == ( frame = (D3DXFRAME_DERIVED*)D3DXFrameFind( g_FrameRoot, meshContainer->pSkinInfo->GetBoneName( iBone ) ) ) )
			{
				return E_FAIL;
			}

			meshContainer->m_BoneMatrixPtrs[iBone] = &frame->m_CombinedTransformationMatrix;
		}
	}

	return S_OK;
}


HRESULT SetupBoneMatrixPointers( LPD3DXFRAME frame )
{
	HRESULT hr = S_OK;

	if ( nullptr != frame->pMeshContainer )
	{
		if ( FAILED( hr = SetupBoneMatrixPointersOnMesh( frame->pMeshContainer ) ) )
		{
			return hr;
		}
	}

	if ( nullptr != frame->pFrameSibling )
	{
		if ( FAILED( hr = SetupBoneMatrixPointers( frame->pFrameSibling ) ) )
		{
			return hr;
		}
	}

	if ( nullptr != frame->pFrameFirstChild )
	{
		if ( FAILED( hr = SetupBoneMatrixPointers( frame->pFrameFirstChild ) ) )
		{
			return hr;
		}
	}

	return S_OK;
}


void UpdateFrameMatrices( LPD3DXFRAME frameBase, LPD3DXMATRIX parentMatrix )
{
	D3DXFRAME_DERIVED* frame = (D3DXFRAME_DERIVED*)frameBase;

	if ( nullptr != parentMatrix )
	{
		D3DXMatrixMultiply( &frame->m_CombinedTransformationMatrix, &frame->TransformationMatrix, parentMatrix );
	}
	else
	{
		frame->m_CombinedTransformationMatrix = frame->TransformationMatrix;
	}

	if ( nullptr != frame->pFrameSibling )
	{
		UpdateFrameMatrices( frame->pFrameSibling, parentMatrix );
	}

	if ( nullptr != frame->pFrameFirstChild )
	{
		UpdateFrameMatrices( frame->pFrameFirstChild, &frame->m_CombinedTransformationMatrix );
	}
}


void UpdateSkinningMethod( LPD3DXFRAME frameBase )
{
	D3DXFRAME_DERIVED* frame = (D3DXFRAME_DERIVED*)frameBase;
	D3DXMESHCONTAINER_DERIVED* meshContainer = nullptr;

	meshContainer = (D3DXMESHCONTAINER_DERIVED*)frame->pMeshContainer;

	while ( nullptr != meshContainer )
	{
		GenerateSkinnedMesh( DXUTGetD3D9Device(), meshContainer );

		meshContainer = (D3DXMESHCONTAINER_DERIVED*)meshContainer->pNextMeshContainer;
	}

	if ( nullptr != frame->pFrameSibling )
	{
		UpdateSkinningMethod( frame->pFrameSibling );
	}

	if ( nullptr != frame->pFrameFirstChild )
	{
		UpdateSkinningMethod( frame->pFrameFirstChild );
	}
}


void ReleaseAttributeTable( LPD3DXFRAME frameBase )
{
	D3DXFRAME_DERIVED* frame = (D3DXFRAME_DERIVED*)frameBase;
	D3DXMESHCONTAINER_DERIVED* meshContainer = nullptr;

	meshContainer = (D3DXMESHCONTAINER_DERIVED*)frame->pMeshContainer;

	while ( nullptr != meshContainer )
	{
		SAFE_DELETE_ARRAY( meshContainer->m_AttributeTable );

		meshContainer = (D3DXMESHCONTAINER_DERIVED*)meshContainer->pNextMeshContainer;
	}

	if ( nullptr != frame->pFrameSibling )
	{
		ReleaseAttributeTable( frame->pFrameSibling );
	}

	if ( nullptr != frame->pFrameFirstChild )
	{
		ReleaseAttributeTable( frame->pFrameFirstChild );
	}
}
