//-----------------------------------------------------------------------------
// File: MD3Model.h
// Desc: CMD3Model declaration split from legacy MD3.h
//-----------------------------------------------------------------------------
#pragma once

#include "MD3Types.h"

class MD3Model
{
public:
	MD3Model();
	~MD3Model();

	int  LoadModelGeometry(const char* filename);
	void DeleteModelGeometry();
	void DumpGeometryInfo();

	void UpdateFrameTime(float fTime);

	void DrawSkeleton(MD3Model* md3Model,
		IDirect3DDevice9* m_pd3dDevice,
		IDirect3DIndexBuffer9* m_pIB,
		IDirect3DVertexBuffer9* m_pVB,
		D3DXMATRIX* viewProj,
		D3DXMATRIX* world,
		D3DXMATRIX* lightViewProj,
		D3DXMATRIX* scaleBias,
		IDirect3DVertexDeclaration9* m_pVertexDeclaration);

	int  LinkModel(const char* cTagName, MD3Model* mod);
	void UnLinkModel(const char* cTagName);

	void LoadSkins(IDirect3DDevice9* m_pd3dDevice, const char* filename, const char* imagepath);
	void DeleteSkins();
	void DumpSkinInfo();

	void LoadShaders(IDirect3DDevice9* m_pd3dDevice, const char* pcFileName, const char* pcShaderPath);
	void DeleteShaders(IDirect3DDevice9* m_pd3dDevice);
	void DumpShaderInfo();

	void CalculateNormals2(MD3MESH* MD3Mesh);

	void ComputeDuDv(const D3DXVECTOR3& v0_pos, const D3DXVECTOR2& v0_uv,
		const D3DXVECTOR3& v1_pos, const D3DXVECTOR2& v1_uv,
		const D3DXVECTOR3& v2_pos, const D3DXVECTOR2& v2_uv,
		TANGENTS& meshTangents);
	void AverageTriangles(int index, int tri_count, int frame, MD3MESH* MD3Mesh);
	void GenerateTangent(MD3MESH* MD3Mesh);



	int				iFrame;			// current frame to draw
	int				iNextFrame;		// next frame to draw
	int				iFps;			// frames per second

	int				iStartFrame;
	int				iEndFrame;

	MD3MESH* md3Meshes;
	MD3HEADER		modelHeader;




private:
	char			cModelName[512];		// name for the md3 file
	MD3BONEFRAME* md3BoneFrames;
	D3DTAG* d3dTag;		// Direct3D specific

	MD3Model** md3Links;

	float			fOldTime;
	float			fNewTime;

	IDirect3DTexture9* pMiniTextureCache[MAXTEXTURESPERMESH];
};
