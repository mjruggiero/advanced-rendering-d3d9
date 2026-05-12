
#include "Plane.h"

#include "D3D9Compat.h"
#include "Logger.h"
#include "Utility.h"

#include <cstdio>
#include <cstring>
#include <string>

const DWORD PlaneVertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;

Plane::Plane()
{
	m_pDevice = NULL;
	m_fWidth = 0.0f;
	m_fDepth = 0.0f;
	m_pVB = NULL;
	m_pDecl = NULL;
	m_pShader = NULL;
	iShaderProfile = 0;
	for (int i = 0; i < MAXSHADERLEVELINMESH; ++i)
	{
		pVertexShader[i] = 0;
		pPixelShader[i] = 0;
	}
}

Plane::~Plane()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

HRESULT Plane::Create(float width, float depth)
{
	m_fWidth = width;
	m_fDepth = depth;

	m_Vertices[0].position.x = -width;
	m_Vertices[0].position.y = -25.0f;
	m_Vertices[0].position.z = depth;
	m_Vertices[0].normal.x = 0.0f;
	m_Vertices[0].normal.y = 1.0f;
	m_Vertices[0].normal.z = 0.0f;
	m_Vertices[0].tangent.x = 1.0f;
	m_Vertices[0].tangent.y = 0.0f;
	m_Vertices[0].tangent.z = 0.0f;
	m_Vertices[0].binormal.x = 0.0f;
	m_Vertices[0].binormal.y = 0.0f;
	m_Vertices[0].binormal.z = 1.0f;
	m_Vertices[0].s = 0.0f;
	m_Vertices[0].t = 0.0f;
	//m_Vertices[0].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);

	m_Vertices[3].position.x = width;
	m_Vertices[3].position.y = -25.0f;
	m_Vertices[3].position.z = depth;
	m_Vertices[3].normal.x = 0.0f;
	m_Vertices[3].normal.y = 1.0f;
	m_Vertices[3].normal.z = 0.0f;
	m_Vertices[3].tangent.x = 1.0f;
	m_Vertices[3].tangent.y = 0.0f;
	m_Vertices[3].tangent.z = 0.0f;
	m_Vertices[3].binormal.x = 0.0f;
	m_Vertices[3].binormal.y = 0.0f;
	m_Vertices[3].binormal.z = 1.0f;
	m_Vertices[3].s = 2.0f;
	m_Vertices[3].t = 0.0f;
	//m_Vertices[3].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);

	m_Vertices[2].position.x = width;
	m_Vertices[2].position.y = -25.0f;
	m_Vertices[2].position.z = -depth;
	m_Vertices[2].normal.x = 0.0f;
	m_Vertices[2].normal.y = 1.0f;
	m_Vertices[2].normal.z = 0.0f;
	m_Vertices[2].tangent.x = 1.0f;
	m_Vertices[2].tangent.y = 0.0f;
	m_Vertices[2].tangent.z = 0.0f;
	m_Vertices[2].binormal.x = 0.0f;
	m_Vertices[2].binormal.y = 0.0f;
	m_Vertices[2].binormal.z = 1.0f;
	m_Vertices[2].s = 2.0f;
	m_Vertices[2].t = 2.0f;
	//m_Vertices[2].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);

	m_Vertices[1].position.x = -width;
	m_Vertices[1].position.y = -25.0f;
	m_Vertices[1].position.z = -depth;
	m_Vertices[1].normal.x = 0.0f;
	m_Vertices[1].normal.y = 1.0f;
	m_Vertices[1].normal.z = 0.0f;
	m_Vertices[1].tangent.x = 1.0f;
	m_Vertices[1].tangent.y = 0.0f;
	m_Vertices[1].tangent.z = 0.0f;
	m_Vertices[1].binormal.x = 0.0f;
	m_Vertices[1].binormal.y = 0.0f;
	m_Vertices[1].binormal.z = 1.0f;
	m_Vertices[1].s = 0.0f;
	m_Vertices[1].t = 2.0f;
	//m_Vertices[1].diffuse = D3DCOLOR_ARGB(255, 255, 255, 255);

	return S_OK;
}

void Plane::LoadShaders(IDirect3DDevice9* pDevice, const char* pcFileName, const char* pcShaderPath)
{
	FILE* shaderFile = fopen(pcFileName, "rt");
	if (!shaderFile)
	{
		LOG("Could not open plane shader list: " + std::string(pcFileName), Logger::LOG_ERR);
		return;
	}

	char shaderName[256] = {};
	char token[1024] = {};
	char shaderPath[1024] = {};
	char textLine[1024] = {};

	while (fscanf(shaderFile, "%1023s", textLine) == 1)
	{
		BOOL foundShader = FALSE;
		BOOL foundShaderLevel = FALSE;
		int shaderLevel = 0;

		while (textLine[0])
		{
			ParseTextLine(textLine, token);

			char loweredToken[1024] = {};
			strcpy(loweredToken, token);
			_strlwr(loweredToken);

			if ((strstr(loweredToken, ".vsh") || strstr(loweredToken, ".psh")) && !foundShader)
			{
				strcpy(shaderName, token);
				foundShader = TRUE;
			}
			else if (strstr(loweredToken, "shaderlevel") && !foundShaderLevel && foundShader)
			{
				shaderLevel = ParseNumber(token);
				foundShaderLevel = TRUE;
			}
		}

		if (!foundShader)
			continue;

		if (shaderLevel < 0 || shaderLevel >= MAXSHADERLEVELINMESH)
		{
			LOG("Invalid plane shader level for shader: " + std::string(shaderName), Logger::LOG_ERR);
			continue;
		}

		const char* shaderRoot = (pcShaderPath && pcShaderPath[0]) ? pcShaderPath : "shaders";
		sprintf(shaderPath, "%s\\%s", shaderRoot, shaderName);

		DWORD shaderFlags = 0;

#if defined(_DEBUG) || defined(DEBUG)
		shaderFlags |= D3DXSHADER_DEBUG;
#endif

		ID3DXBuffer* code = nullptr;
		HRESULT hr = D3DXAssembleShaderFromFileA(
			shaderPath,
			nullptr,
			nullptr,
			shaderFlags,
			&code,
			nullptr);

		if (FAILED(hr) || !code)
		{
			LOG("Could not assemble plane shader: " + std::string(shaderPath), Logger::LOG_ERR);
			SAFE_RELEASE(code);
			continue;
		}

		if (strstr(shaderPath, ".vsh"))
		{
			SAFE_RELEASE(pVertexShader[shaderLevel]);

			hr = pDevice->CreateVertexShader(
				static_cast<DWORD*>(code->GetBufferPointer()),
				&pVertexShader[shaderLevel]);

			if (FAILED(hr))
			{
				LOG("Could not create plane vertex shader: " + std::string(shaderPath), Logger::LOG_ERR);
				pVertexShader[shaderLevel] = nullptr;
			}
			else
			{
				LOG("Created plane vertex shader: " + std::string(shaderPath), Logger::LOG_DATA);
			}
		}
		else if (strstr(shaderPath, ".psh"))
		{
			SAFE_RELEASE(pPixelShader[shaderLevel]);

			hr = pDevice->CreatePixelShader(
				static_cast<DWORD*>(code->GetBufferPointer()),
				&pPixelShader[shaderLevel]);

			if (FAILED(hr))
			{
				LOG("Could not create plane pixel shader: " + std::string(shaderPath), Logger::LOG_ERR);
				pPixelShader[shaderLevel] = nullptr;
			}
			else
			{
				LOG("Created plane pixel shader: " + std::string(shaderPath), Logger::LOG_DATA);
			}
		}

		SAFE_RELEASE(code);
	}

	fclose(shaderFile);
}

VOID Plane::SetShaderProfile(int iProfile)
{
	iShaderProfile = iProfile;
}

HRESULT Plane::RenderNoShader()
{
	m_pDevice->SetVertexDeclaration(m_pDecl);
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(PlaneVertex));
	m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	return S_OK;
}

HRESULT Plane::Render(D3DXMATRIX matViewProj)
{
	D3DXMATRIX matClip;
	D3DXMatrixTranspose(&matClip, &matViewProj);

	m_pDevice->SetTexture(0, m_pTexture);
	m_pDevice->SetTexture(1, NULL);
	//m_pDevice->SetVertexShader(m_pShader);
	m_pDevice->SetVertexShader(pVertexShader[iShaderProfile]);
	//m_pDevice->SetPixelShader(NULL);
	m_pDevice->SetPixelShader(pPixelShader[iShaderProfile]);

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pDevice->SetVertexShaderConstantF(0, (float*)&matWorld, 4);
	m_pDevice->SetVertexShaderConstantF(8, (float*)&matClip, 4);
	m_pDevice->SetVertexShaderConstantF(4, (float*)&D3DXVECTOR4(1, 1, 1, 0)[0], 1);

	m_pDevice->SetVertexDeclaration(m_pDecl);

	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(PlaneVertex));

	m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	return S_OK;
}

HRESULT Plane::InitDeviceObjects(LPDIRECT3DDEVICE9 pDevice)
{
	m_pDevice = pDevice;
	return S_OK;
}

HRESULT Plane::RestoreDeviceObjects()
{
	// create vertex declaration
	D3DVERTEXELEMENT9 decl[] =
	{
		// stream, offset, type, method, semantic type (for example normal), ?
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
		{0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
		{0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	m_pDevice->CreateVertexDeclaration(decl, &m_pDecl);

	// create and fill vertex buffer
	m_pDevice->CreateVertexBuffer(4 * sizeof(PlaneVertex), D3DUSAGE_WRITEONLY,
		PlaneVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL);

	PlaneVertex* pData = NULL;

	m_pVB->Lock(0, 0, (void**)&pData, 0);

	memcpy(pData, m_Vertices, 4 * sizeof(PlaneVertex));

	m_pVB->Unlock();

	ID3DXBuffer* code = nullptr;

	DWORD shaderFlags = 0;

#if defined(_DEBUG) || defined(DEBUG)
	shaderFlags |= D3DXSHADER_DEBUG;
#endif

	HRESULT hr = D3DXAssembleShaderFromFileA(
		"shaders/sphere.vsh",
		nullptr,
		nullptr,
		shaderFlags,
		&code,
		nullptr);

	if (FAILED(hr) || !code)
	{
		LOG("Could not assemble plane fallback vertex shader: shaders/sphere.vsh", Logger::LOG_ERR);
		SAFE_RELEASE(code);
		return E_FAIL;
	}

	hr = m_pDevice->CreateVertexShader(
		static_cast<DWORD*>(code->GetBufferPointer()),
		&m_pShader);

	SAFE_RELEASE(code);

	if (FAILED(hr))
	{
		LOG("Could not create plane fallback vertex shader", Logger::LOG_ERR);
		return E_FAIL;
	}

	hr = D3DXCreateTextureFromFileA(m_pDevice, "../media/WallSandstone.tga", &m_pTexture);
	if (FAILED(hr))
	{
		LOG("Could not load plane texture: WallSandstone.tga", Logger::LOG_ERR);
		m_pTexture = nullptr;
	}

	return S_OK;
}

HRESULT Plane::InvalidateDeviceObjects()
{
	SAFE_RELEASE(m_pDecl);
	SAFE_RELEASE(m_pTexture);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pShader);
	return S_OK;
}

HRESULT Plane::DeleteDeviceObjects()
{
	for (int i = 0; i < MAXSHADERLEVELINMESH; ++i)
	{
		SAFE_RELEASE(pVertexShader[i]);
		SAFE_RELEASE(pPixelShader[i]);
	}
	m_pDevice = NULL;
	m_pDecl = NULL;
	m_pVB = NULL;
	m_pShader = NULL;
	return S_OK;
}
