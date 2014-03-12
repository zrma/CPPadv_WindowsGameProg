//////////////////////////////////////////////////////////////////////////
// http://blog.naver.com/poweryang1/110031563312
// http://blog.naver.com/winkey83/43694984 참고
//////////////////////////////////////////////////////////////////////////

#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "resource.h"

#pragma once

CModelViewerCamera          g_Camera;               // 카메라
ID3DXEffect*                g_pEffect = NULL;       // 쉐이더
ID3DXMesh*                  g_pMesh = NULL;         // 메쉬
IDirect3DTexture9*          g_pMeshTexture = NULL;  // 텍스쳐

//////////////////////////////////////////////////////////////////////////
// A shader compile flag that gives the compiler hints about how the shader will be used.
bool                        g_bEnablePreshader;

D3DXMATRIXA16               g_mCenterWorld;			// 월드 매트릭스

#define MAX_LIGHTS 3								// 최대 조명 개수 3개

//////////////////////////////////////////////////////////////////////////
// 래핑 된 클래스 - 정의 참고
CDXUTDirectionWidget		g_LightControl[MAX_LIGHTS];
//////////////////////////////////////////////////////////////////////////

float                       g_fLightScale;			// 빛 밝기
int                         g_nNumActiveLights;		// 켜진 조명 개수
int                         g_nActiveLight;			// 현재 활성화 된(선택 된) 조명

bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void InitApp();
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif


	//////////////////////////////////////////////////////////////////////////
	// DXUT가 제공하는 것들
	//////////////////////////////////////////////////////////////////////////
	// 윈도우 생성
	// Direct3D 디바이스 선택
	// Direct3D 디바이스 생성
	// 디바이스 이벤트 처리
	// 윈도우 이벤트 처리
	// 창 모드와 전체 화면 모드 사이의 전환

	//////////////////////////////////////////////////////////////////////////
	// 콜백 함수 등록
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// D3D 콜백 설정

	// 장치를 사용 할 수 있는지
	DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );

	// Direct3D 디바이스 생성
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
	
	// 초기화 또는 Lost시 리소스 재생성
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
	
	// 렌더
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	
	// 화면 모드(창 모드, 전체 화면 모드) 변경 시 처리
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
	
	// 리소스 해제
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// 일반 콜백 설정

	
	// 메시지 핸들링
    DXUTSetCallbackMsgProc( MsgProc );
	
	// DXUT가 렌더링 과정 중 매 프레임마다 사전 호출
	// http://blog.naver.com/poweryang1/110031577088 참고
    DXUTSetCallbackFrameMove( OnFrameMove );
	
	//////////////////////////////////////////////////////////////////////////

	InitApp();
	// 조명 초기화

	DXUTInit( false, false );
	// 명령어 행 전달 인자, 오류 발생 시 메시지 박스 등 출력 여부 초기 설정
	// http://cafe.naver.com/makkolli/55
	// http://cafe.naver.com/cafec/234056

    DXUTSetHotkeyHandling( true, true, false );
	//////////////////////////////////////////////////////////////////////////
	// 기본 키 입력을 처리 할 것인가
	//
	// Alt + Enter 전체창
	// ESC로 나가기
	// Pause로 멈추기
	//////////////////////////////////////////////////////////////////////////

	// 창 생성
	DXUTCreateWindow( L"BasicHLSL" );
	
	// 해당 width, height를 가진 윈도우로 생성할 것인가, 전체창으로 생성할 것인가
    DXUTCreateDevice( true, 800, 480 );
	
	// 무한 루프
    DXUTMainLoop();
	
	//////////////////////////////////////////////////////////////////////////
	// 사용자 레벨에서의 리소스 해제는 이곳에 적기
	//////////////////////////////////////////////////////////////////////////

	// DXUT의 종료 코드를 받아옴
    return DXUTGetExitCode();
}


//////////////////////////////////////////////////////////////////////////
// 프로그램 초기화
//////////////////////////////////////////////////////////////////////////
void InitApp()
{
    g_bEnablePreshader = true;

	//////////////////////////////////////////////////////////////////////////
	// 조명의 방향 벡터 초기화
    for( int i = 0; i < MAX_LIGHTS; i++ )
	{
		g_LightControl[i].SetLightDirection( D3DXVECTOR3( sinf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ),
			0, -cosf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ) ) );
	}

    g_nActiveLight = 0;
	// index가 0번

    g_nNumActiveLights = 1;
	// 1개 켜져 있음

    g_fLightScale = 1.0f;
	// 조명은 풀
}

//////////////////////////////////////////////////////////////////////////
// 장치를 사용 할 수 있는가?
//////////////////////////////////////////////////////////////////////////
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	// http://msdn.microsoft.com/en-us/library/bb172513(VS.85).aspx
	// 픽셀 쉐이더 버전 체크
	if ( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
	{
		return false;
	}

	// DXUT에서 D3D9 오브젝트를 가져와서
	IDirect3D9* pD3D = DXUTGetD3D9Object();

	//////////////////////////////////////////////////////////////////////////
	// 알파블렌딩 체크
	// http://ozlael.egloos.com/viewer/3362810
	//////////////////////////////////////////////////////////////////////////
	if ( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat,
		D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 디바이스 생성
//////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

	// 메쉬 로딩 및 최적화 - 해당 함수 참고
    V_RETURN( LoadMesh( pd3dDevice, L"tiny\\tiny.x", &g_pMesh ) );

    D3DXVECTOR3* pData;
    D3DXVECTOR3 vCenter;
    FLOAT fObjectRadius;

	// 메쉬에 락을 걸어서 버퍼 접근을 시작함
    V( g_pMesh->LockVertexBuffer( D3DLOCK_READONLY, ( LPVOID* )&pData ) );
	
	//////////////////////////////////////////////////////////////////////////
	// 메쉬 경계(충돌 체크를 위한) 산출
	V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(), D3DXGetFVFVertexSize( g_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
	
	// 메쉬 락 해제 - 접근 끝
    V( g_pMesh->UnlockVertexBuffer() );

    D3DXMatrixTranslation( &g_mCenterWorld, -vCenter.x, -vCenter.y, -vCenter.z );
    D3DXMATRIXA16 m;
    D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterWorld *= m;
    D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    g_mCenterWorld *= m;

	// 래핑 된 클래스 DXUTDirectionWidget을 이용해 장치 생성
    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );
	
	for ( int i = 0; i < MAX_LIGHTS; i++ )
	{
		// 카메라 회전 구 경계면 지름 설정
		g_LightControl[i].SetRadius( fObjectRadius );
	}

	// 쉐이더의 데이터를 메모리에 보존하지 않음으로써 쉐이더의 메모리 사용량을 50% 가량 줄인다.
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	// 메모리 최적화

#if defined( DEBUG ) || defined( _DEBUG )
    dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// InitApp() 에서 true로 미리 설정 되어 있음
	if ( !g_bEnablePreshader )
	{
		dwShaderFlags |= D3DXSHADER_NO_PRESHADER;
	}

    // D3DX Effect 파일 찾아보기
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"BasicHLSL.fx" ) );

    // 파일을 찾을 수 있을 때, 실제로 Effect 생성
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

    // 텍스쳐 파일 찾기
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"tiny\\tiny_skin.dds" ) );

	// 실제 텍스쳐 생성
	V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_pMeshTexture ) );

    D3DXCOLOR colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
    D3DXCOLOR colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );

	//////////////////////////////////////////////////////////////////////////
	// 여기부터
	V_RETURN( g_pEffect->SetValue( "g_MaterialAmbientColor", &colorMtrlAmbient, sizeof( D3DXCOLOR ) ) );
	V_RETURN( g_pEffect->SetValue( "g_MaterialDiffuseColor", &colorMtrlDiffuse, sizeof( D3DXCOLOR ) ) );
	V_RETURN( g_pEffect->SetTexture( "g_MeshTexture", g_pMeshTexture ) );
	// 여기까지 쉐이더 사용을 위한 설정
	//////////////////////////////////////////////////////////////////////////

	// 카메라 설정
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -15.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );

	// 카메라 벡터 설정
    g_Camera.SetViewParams( &vecEye, &vecAt );

	// 카메라 회전 구 경계면 지름 설정
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// 메쉬 로딩하고 최적화 하기
//////////////////////////////////////////////////////////////////////////
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh )
{
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
	// 메쉬 파일 찾기

    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh ) );
	// 로딩

    DWORD* rgdwAdjacency = NULL;

	//////////////////////////////////////////////////////////////////////////
    // 메쉬 안에 노멀 벡터가 없을 경우 노멀 벡터 생성
	if ( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh;
		V( pMesh->CloneMeshFVF( pMesh->GetOptions(), pMesh->GetFVF() | D3DFVF_NORMAL, pd3dDevice, &pTempMesh ) );
		V( D3DXComputeNormals( pTempMesh, NULL ) );

		SAFE_RELEASE( pMesh );
		pMesh = pTempMesh;
	}

    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;

	//////////////////////////////////////////////////////////////////////////
	// 성능 최적화를 위해서 메쉬의 버텍스들의 인접 정보 값을 가져오기
    V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) );

    V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) );
	//
	// 아웃풋 되어 있을 때에는 정리가 안 되어 있을 수 있으니 들어왔을 때 정리해서 캐시에 넣어둠
	//
	// 최적화 - 캐시 사용하기 옵션
	// http://coreafive.tistory.com/201 참조
	//////////////////////////////////////////////////////////////////////////

    delete []rgdwAdjacency;

    *ppMesh = pMesh;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// http://blog.naver.com/proonan29/130065459062
//
// D3DPOOL_DEFAULT 형태로 선언 된 리소스는, Alt + Tab 등으로 프로그램의 Focus가 바뀌거나,
// 창 모드 <-> 풀 모드 간 전환 등으로 LostDevice나 리소스 Reset이 발생하면 여기서 재생성함
//////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	if ( g_pEffect )
	{
		V_RETURN( g_pEffect->OnResetDevice() );
	}

	for ( int i = 0; i < MAX_LIGHTS; i++ )
	{
		g_LightControl[i].OnD3D9ResetDevice( pBackBufferSurfaceDesc );
	}

	// 투영 설정
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
	
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );
	//////////////////////////////////////////////////////////////////////////
	// 모델 회전
	// 줌
	// 카메라 회전
	//////////////////////////////////////////////////////////////////////////

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// DXUT가 렌더링 과정 중 매 프레임마다 사전 호출, 실제 렌더링을 하진 않음
// 각종 Object Update는 이 곳에서
//////////////////////////////////////////////////////////////////////////
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// 카메라 위치 계산
    g_Camera.FrameMove( fElapsedTime );
}

//////////////////////////////////////////////////////////////////////////
// 렌더
//////////////////////////////////////////////////////////////////////////
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXVECTOR3 vLightDir[MAX_LIGHTS];
    D3DXCOLOR vLightDiffuse[MAX_LIGHTS];
    UINT iPass, cPasses;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    // 초기화
	V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f, 0 ) );

    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        // 카메라로부터 행렬 얻어오기
		mWorld = g_mCenterWorld * ( *( g_Camera.GetWorldMatrix() ) );
        mProj = *( g_Camera.GetProjMatrix() );
		mView = *( g_Camera.GetViewMatrix() );

		mWorldViewProjection = mWorld * mView * mProj;

        // 조명 아이콘 그려주는 곳
        for( int i = 0; i < g_nNumActiveLights; i++ )
        {
            D3DXCOLOR arrowColor = ( i == g_nActiveLight ) ? D3DXCOLOR( 1, 1, 0, 1 ) : D3DXCOLOR( 1, 1, 1, 1 );

			// 색상, Viewing Matrix, Projection Matrix, Eye Vector로 렌더하기
            V( g_LightControl[i].OnRender9( arrowColor, &mView, &mProj, g_Camera.GetEyePt() ) );

			// 조명 방향 설정
            vLightDir[i] = g_LightControl[i].GetLightDirection();

			// 조명 색 설정
            vLightDiffuse[i] = g_fLightScale * D3DXCOLOR( 1, 1, 1, 1 );
        }

		//////////////////////////////////////////////////////////////////////////
		// 쉐이더 설정 시작
		//
		// 조명 방향
        V( g_pEffect->SetValue( "g_LightDir", vLightDir, sizeof( D3DXVECTOR3 ) * MAX_LIGHTS ) );

		// 조명 색상
        V( g_pEffect->SetValue( "g_LightDiffuse", vLightDiffuse, sizeof( D3DXVECTOR4 ) * MAX_LIGHTS ) );

		// 쉐이더에 매트릭스 건네줌 World * View * Projection Matrix
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );

		// 쉐이더에 매트릭스 건네줌 World Matrix
        V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );
		
		// 쉐이더에 시간 정보 건네줌
		V( g_pEffect->SetFloat( "g_fTime", ( float )fTime * 10 ) );

        D3DXCOLOR vWhite = D3DXCOLOR( 1, 1, 1, 1 );

		// 조명 및 색상
        V( g_pEffect->SetValue( "g_MaterialDiffuseColor", &vWhite, sizeof( D3DXCOLOR ) ) );
        
        V( g_pEffect->SetInt( "g_nNumLights", g_nNumActiveLights ) );

		// 조명에 따라서 .fx 파일에서 정의 된 대로 렌더하기 설정
		switch ( g_nNumActiveLights )
		{
			case 1:
				V( g_pEffect->SetTechnique( "RenderSceneWithTexture1Light" ) );
				break;
			case 2:
				V( g_pEffect->SetTechnique( "RenderSceneWithTexture2Light" ) );
				break;
			case 3:
				V( g_pEffect->SetTechnique( "RenderSceneWithTexture3Light" ) );
				break;
		}


        // 실제로 쉐이더 적용하기 시작
        V( g_pEffect->Begin( &cPasses, 0 ) );

		// 패스 횟수 만큼 그려줌
        for( iPass = 0; iPass < cPasses; iPass++ )
        {
			// http://blog.naver.com/unisocket/90017223793
			// 시작
            V( g_pEffect->BeginPass( iPass ) );

			// 쉐이더에서 반환하는 technique을 적용하여 메쉬를 렌더링 하기 시작
            V( g_pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
			// 끝
        }
        V( g_pEffect->End() );
		// 쉐이더 사용 끝

        V( pd3dDevice->EndScene() );
    }
}


//////////////////////////////////////////////////////////////////////////
// 메시지 발생 처리하는 콜백 함수
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    g_LightControl[g_nActiveLight].HandleMessages( hWnd, uMsg, wParam, lParam );
	// 선택 된 해당 번째의 조명 번호에 맞는 CDXUTDirectionWidget 객체에서 메시지 발생을 받아서 처리 함
	// 우 클릭일 때는 이쪽에서 처리

    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	// CModelViewerCamera 객체에서 메시지 발생을 받아서 처리
	// 좌 클릭일 때는 이쪽에서 처리

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// 창을 관리하는(디바이스를) Focus를 잃어버림
//////////////////////////////////////////////////////////////////////////
void CALLBACK OnLostDevice( void* pUserContext )
{
    CDXUTDirectionWidget::StaticOnD3D9LostDevice();

    if( g_pEffect )
        g_pEffect->OnLostDevice();
}

//////////////////////////////////////////////////////////////////////////
// 리소스 해제
//////////////////////////////////////////////////////////////////////////
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();
	// 잘 래핑 된 리소스 해제

    SAFE_RELEASE( g_pEffect );
	// 쉐이더

    SAFE_RELEASE( g_pMesh );
    SAFE_RELEASE( g_pMeshTexture );
}



