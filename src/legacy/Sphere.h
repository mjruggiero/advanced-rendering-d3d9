//-----------------------------------------------------------------------------
// File: Sphere.h
//
// Desc: sphere class to mark the point, 
//       where the point light is positioned
//
// Last modification: November 16, 2001
//
// Copyright (c) 2001 - 2002 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------
#pragma once

#include <d3d9.h>
#include <d3dx9.h>

//-----------------------------------------------------------------------------
// Name: class D3DSphere
// Desc: Sphere class
//-----------------------------------------------------------------------------
class D3DSphere
{
	// sphere that marks position of point light
	ID3DXMesh* m_pSphere;
	IDirect3DVertexBuffer9* m_pSphereVB;
	IDirect3DIndexBuffer9* m_pSphereIB;
	D3DXVECTOR4				m_vLight;				// light position or direction
	DWORD					m_dwNumOfSphereFaces;
	DWORD					m_dwNumOfSphereVertices;
	//	DWORD					m_dwSphereShader;		// shader
	IDirect3DVertexShader9* pSphereShader;
	IDirect3DVertexDeclaration9* m_pVertexDeclaration;

public:
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice);
	HRESULT OnLostDevice();
	HRESULT OnDestroyDevice();

	HRESULT RenderSphere(IDirect3DDevice9* pd3dDevice, D3DXMATRIX m_matViewProj);

	void SetLightPos(D3DXVECTOR4& Light) { m_vLight = Light; }

	// Constructor / destructor
	D3DSphere();
	~D3DSphere();
};
