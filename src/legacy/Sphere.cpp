//-----------------------------------------------------------------------------
// File: Sphere.cpp
//
// Desc: sphere class to mark the point, 
//       where the point light is positioned
//
// Last modification: March 31th, 2003
//
// Copyright (c) 2001 - 2003 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------
#include "D3D9Compat.h"

#include "Sphere.h"
#include "Utility.h"

//-----------------------------------------------------------------------------
// Name: CD3DSphere()
// Desc: Sphere class constructor
//-----------------------------------------------------------------------------
D3DSphere::D3DSphere( )
{
    pSphereShader	= NULL;
    m_pSphere			= NULL;
	m_pSphereVB			= NULL;
	m_pSphereIB			= NULL;
	m_vLight			= D3DXVECTOR4(0,0,0,0);
}

//-----------------------------------------------------------------------------
// Name: ~CD3DSphere()
// Desc: Sphere class destructor
//-----------------------------------------------------------------------------
D3DSphere::~D3DSphere()
{
}

//-----------------------------------------------------------------------------
// Name: OnCreateDevice()
// Desc: Initializes device-dependent objects, including the vertex buffer used
//       for rendering text and the texture map which stores the font image.
//-----------------------------------------------------------------------------
HRESULT D3DSphere::OnCreateDevice(IDirect3DDevice9* pd3dDevice)
{
	// create the sphere that marks the point light
    if (FAILED( D3DXCreateSphere(pd3dDevice, 1.0f, 30, 30, &m_pSphere, NULL) ) )
        return E_FAIL;

	m_pSphere->GetVertexBuffer( &m_pSphereVB );
	m_pSphere->GetIndexBuffer( &m_pSphereIB );		
	m_dwNumOfSphereFaces = m_pSphere->GetNumFaces();
	m_dwNumOfSphereVertices = m_pSphere->GetNumVertices();	

	D3DVERTEXELEMENT9 decl[]=
	{
		// stream, offset, type, method, semantic type (for example normal), ?
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END()
	};

	LPD3DXBUFFER pCode = nullptr;
    HRESULT hr;

	V_RETURN(pd3dDevice->CreateVertexDeclaration(decl, &m_pVertexDeclaration));
	V_RETURN(D3DXAssembleShaderFromFile(L"shaders/sphere.vsh", NULL, NULL, NULL, &pCode, NULL));
	V_RETURN(pd3dDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &pSphereShader));
	SAFE_RELEASE(pCode);

   return S_OK;
}

//-----------------------------------------------------------------------------
// Name: OnResetDevice()
// Desc:
//-----------------------------------------------------------------------------
HRESULT D3DSphere::OnResetDevice(IDirect3DDevice9* pd3dDevice)
{

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: OnLostDevice()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT D3DSphere::OnLostDevice()
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: OnDestroyDevice()
// Desc: Destroys all device-dependent objects
//-----------------------------------------------------------------------------
HRESULT D3DSphere::OnDestroyDevice()
{
    SAFE_RELEASE( m_pSphere );
	SAFE_RELEASE (m_pSphereVB);
	SAFE_RELEASE (m_pSphereIB);
	SAFE_RELEASE (pSphereShader);
	SAFE_RELEASE (m_pVertexDeclaration);

    return S_OK;
}

//------------------------------------------------------------------------------
// Name:	RenderSphere
// Desc:	renders the sphere that marks the point light
//------------------------------------------------------------------------------
HRESULT D3DSphere::RenderSphere(IDirect3DDevice9* pd3dDevice, D3DXMATRIX m_matViewProj)
{
    D3DXMATRIX world;
    D3DXMatrixTranslation(&world, m_vLight.x, 
								  m_vLight.y, 
								  m_vLight.z);

    D3DXMATRIX temp,clip;
    D3DXMatrixMultiply(&temp,&world,&m_matViewProj);
    D3DXMatrixTranspose(&clip,&temp);
    
	pd3dDevice->SetVertexShaderConstantF(0,(float*)&clip,4);
	pd3dDevice->SetVertexShaderConstantF(4,(float*)&D3DXVECTOR4(1,1,0,0)[0],1);

	pd3dDevice->SetVertexDeclaration(m_pVertexDeclaration);

	pd3dDevice->SetTexture(0, NULL);
	pd3dDevice->SetPixelShader(NULL);

	pd3dDevice->SetVertexShader(pSphereShader);
    
	pd3dDevice->SetStreamSource(0,m_pSphereVB, 0, 6 * sizeof(float));
	pd3dDevice->SetIndices(m_pSphereIB);

    pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				0,
				0,
				m_dwNumOfSphereVertices,
				0,
				m_dwNumOfSphereFaces);

   return S_OK;
}
