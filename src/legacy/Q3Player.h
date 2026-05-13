//-----------------------------------------------------------------------------
// File: Q3Player.h
// Desc: Q3Player declaration split from legacy MD3.h
//-----------------------------------------------------------------------------
#pragma once

#include "MD3Model.h"

class Q3Player
{
public:
	Q3Player();
	~Q3Player();

	void LoadPlayerGeometry(const char* path);
	void DeletePlayerGeometry();

	void LoadSkins(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cSkin);
	void FreeSkins();

	void LoadPlayerShaders(
		IDirect3DDevice9* m_pd3dDevice,
		const char* cPath,
		const char* cName,
		const char* cSkin,
		const char* shaderRootPath = "shaders",
		const char* profileRootPath = nullptr);
	void FreePlayerShaders(IDirect3DDevice9* m_pd3dDevice);
	void LoadPlayerShaderProfile(const char* pcFileName);

	HRESULT CreateVertexNIndexBuffer(IDirect3DDevice9* m_pd3dDevice);
	void DeleteVertexNIndexBuffer();

	void Draw(IDirect3DDevice9* m_pd3dDevice);

	void Update(float time);
	void LoadAnim(const char* filename);
	void SetLowerAnim(int iAnimNumber);
	void SetUpperAnim(int iAnimNumber);
	void DumpAnimInfo();

	void LoadWeaponGeometry(const char* cPath, const char* cName);
	void DeleteWeaponGeometry();

	void LoadWeaponSkins(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cSkin);
	void FreeWeaponSkins();

	void LoadWeaponShaders(
		IDirect3DDevice9* m_pd3dDevice,
		const char* cPath,
		const char* cName,
		const char* cSkin,
		const char* shaderRootPath = "shaders",
		const char* profileRootPath = nullptr);
	void FreeWeaponShaders(IDirect3DDevice9* m_pd3dDevice);
	void LoadWeaponShaderProfile(const char* pcFileName);

	void SetWorldMatrix(D3DXMATRIX& matW) { matWorld = matW; }
	void SetViewProjMatrix(D3DXMATRIX matVP) { matViewProj = matVP; }
	void SetLightViewProjMatrix(const D3DXMATRIX& matLightVP) { matLightViewProj = matLightVP; }
	void SetScaleBiasMatrix(const D3DXMATRIX& matScaleBiasIn) { matScaleBias = matScaleBiasIn; }

	MD3Model				md3Lower, md3Upper, md3Head, md3Weapon;

	IDirect3DVertexDeclaration9* m_pVertexDeclaration;

private:
	MD3ANIM					playerAnim[25];

	int						playerAnimLower;
	int						playerAnimUpper;

	IDirect3DIndexBuffer9* m_pIB;
	IDirect3DVertexBuffer9* m_pVB;

	D3DXMATRIX matViewProj;
	D3DXMATRIX matWorld;
	D3DXMATRIX matLightViewProj;
	D3DXMATRIX matScaleBias;
};
