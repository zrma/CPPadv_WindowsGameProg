#pragma once
#include "windows.h"
#include <d3dx9.h>

class DXMeshLibrary
{
public:
	DXMeshLibrary()
		: m_pD3D( nullptr ), m_pd3dDevice( nullptr )
		, m_pMesh( nullptr ), m_pMeshMaterials( nullptr )
		, m_pMeshTextures( nullptr ), m_dwNumMaterials( 0 )
	{ }
	~DXMeshLibrary() { }

	void		Create( int width, int height );

private:
	HRESULT		InitD3D( HWND hWnd );
	HRESULT		InitGeometry();
	void		Cleanup();
	void		SetupMatrices();
	void		SetupLights();
	void		Render();

	static LRESULT CALLBACK StaticProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	LRESULT CALLBACK		MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	
	LPDIRECT3D9				m_pD3D;
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	LPD3DXMESH				m_pMesh;
	D3DMATERIAL9*			m_pMeshMaterials;
	LPDIRECT3DTEXTURE9*		m_pMeshTextures;
	DWORD					m_dwNumMaterials;
};