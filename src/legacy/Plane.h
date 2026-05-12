#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define MAXSHADERLEVELINMESH 12
#define MAXSHADERPROFILE 3

struct PlaneVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent;
	D3DXVECTOR3 binormal;
	float s, t;
	static const DWORD FVF;
};

class Plane
{
public:
	Plane();
	~Plane();

	HRESULT Create(float width, float depth);
	HRESULT RenderNoShader();
	HRESULT Render(D3DXMATRIX matViewProj);

	// Initializing and destroying device-dependent objects
	HRESULT InitDeviceObjects(IDirect3DDevice9* pDevice);
	HRESULT RestoreDeviceObjects();
	HRESULT InvalidateDeviceObjects();
	HRESULT DeleteDeviceObjects();

	void LoadShaders(IDirect3DDevice9* pDevice, const char* pcFileName, const char* pcShaderPath);
	void SetShaderProfile(int iProfile);

private:
	IDirect3DVertexShader9* pVertexShader[MAXSHADERLEVELINMESH];
	IDirect3DPixelShader9* pPixelShader[MAXSHADERLEVELINMESH];
	int iShaderProfile;

	IDirect3DDevice9* m_pDevice;
	IDirect3DTexture9* m_pTexture;
	float m_fWidth, m_fDepth;
	PlaneVertex m_Vertices[4];
	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DVertexDeclaration9* m_pDecl;
	IDirect3DVertexShader9* m_pShader;

};
