#include "ShadowMapRenderer.h"

#include "../framework/D3D9StateBlock.h"
#include "../framework/FrameworkLog.h"
#include "../legacy/D3D9Compat.h"
#include "../legacy/Plane.h"
#include "../legacy/Q3Player.h"

#include <cmath>

extern int iShaderProfile;

namespace
{
	class ScopedLegacyShaderProfile
	{
	public:
		explicit ScopedLegacyShaderProfile(int profile)
			: m_previous(iShaderProfile)
		{
			iShaderProfile = profile;
		}

		~ScopedLegacyShaderProfile()
		{
			iShaderProfile = m_previous;
		}

	private:
		int m_previous = 0;
	};
}

bool ShadowMapRenderer::CreateResetResources(IDirect3DDevice9* device)
{
	if (!device)
		return false;

	HRESULT hr = device->CreateTexture(
		Size,
		Size,
		1,
		D3DUSAGE_RENDERTARGET,
		Format,
		D3DPOOL_DEFAULT,
		m_shadowMap.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create shadow map texture");
		return false;
	}

	hr = m_shadowMap->GetSurfaceLevel(0, m_shadowMapSurface.Put());
	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to get shadow map surface");
		return false;
	}

	hr = device->CreateDepthStencilSurface(
		Size,
		Size,
		D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		m_shadowDepthSurface.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create shadow depth surface");
		return false;
	}

	return true;
}

void ShadowMapRenderer::DestroyResetResources()
{
	m_shadowDepthSurface.Reset();
	m_shadowMapSurface.Reset();
	m_shadowMap.Reset();
}

void ShadowMapRenderer::Update(float timeSeconds)
{
	m_lightViewPosition = D3DXVECTOR3(
		50.0f * sinf(timeSeconds),
		100.0f,
		50.0f * cosf(timeSeconds));

	D3DXVECTOR3 lightLookAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 lightUp(0.0f, 1.0f, 0.0f);

	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;

	D3DXMatrixLookAtLH(
		&lightView,
		&m_lightViewPosition,
		&lightLookAt,
		&lightUp);

	D3DXMatrixPerspectiveFovLH(
		&lightProjection,
		D3DX_PI / 2.0f,
		1.0f,
		NearPlane,
		FarPlane);

	m_lightViewProjection = lightView * lightProjection;

	const float offsetX = 0.5f + (0.5f / static_cast<float>(Size));
	const float offsetY = 0.5f + (0.5f / static_cast<float>(Size));

	m_scaleBias = D3DXMATRIX(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		offsetX, offsetY, 0.5f, 1.0f);
}

bool ShadowMapRenderer::Render(
	IDirect3DDevice9* device,
	Q3Player* player,
	Plane* plane,
	const D3DXMATRIX& visibleViewProjection) const
{
	if (!device || !m_shadowMapSurface || !m_shadowDepthSurface)
		return false;

	IDirect3DSurface9* oldRenderTarget = nullptr;
	IDirect3DSurface9* oldDepthSurface = nullptr;
	D3DVIEWPORT9 oldViewport = {};

	if (FAILED(device->GetRenderTarget(0, &oldRenderTarget)))
		return false;

	if (FAILED(device->GetDepthStencilSurface(&oldDepthSurface)))
	{
		SAFE_RELEASE(oldRenderTarget);
		return false;
	}

	device->GetViewport(&oldViewport);

	HRESULT hr = device->SetRenderTarget(0, m_shadowMapSurface.Get());
	if (FAILED(hr))
	{
		SAFE_RELEASE(oldDepthSurface);
		SAFE_RELEASE(oldRenderTarget);
		return false;
	}

	hr = device->SetDepthStencilSurface(m_shadowDepthSurface.Get());
	if (FAILED(hr))
	{
		device->SetRenderTarget(0, oldRenderTarget);
		device->SetViewport(&oldViewport);
		SAFE_RELEASE(oldDepthSurface);
		SAFE_RELEASE(oldRenderTarget);
		return false;
	}

	D3DVIEWPORT9 shadowViewport = {};
	shadowViewport.X = 0;
	shadowViewport.Y = 0;
	shadowViewport.Width = Size;
	shadowViewport.Height = Size;
	shadowViewport.MinZ = 0.0f;
	shadowViewport.MaxZ = 1.0f;
	device->SetViewport(&shadowViewport);

	device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		0xffffffff,
		1.0f,
		0);

	D3DXVECTOR4 planeConstants(FarPlane, NearPlane, 0.0f, 0.0f);
	device->SetVertexShaderConstantF(
		34,
		reinterpret_cast<const float*>(&planeConstants),
		1);

	if (player)
	{
		ScopedLegacyShaderProfile shadowProfile(3);
		player->Draw(device);
	}

	if (plane)
	{
		D3DXMATRIX lightVP = m_lightViewProjection;
		D3DXMatrixTranspose(&lightVP, &lightVP);

		device->SetVertexShaderConstantF(16, reinterpret_cast<const float*>(&lightVP), 4);

		plane->SetShaderProfile(2);
		plane->Render(visibleViewProjection);
	}

	device->SetDepthStencilSurface(oldDepthSurface);
	device->SetRenderTarget(0, oldRenderTarget);
	device->SetViewport(&oldViewport);

	SAFE_RELEASE(oldDepthSurface);
	SAFE_RELEASE(oldRenderTarget);

	return true;
}

void ShadowMapRenderer::DrawPreview(IDirect3DDevice9* device) const
{
	if (!device || !m_shadowMap)
		return;

	Framework::D3D9ScopedStateBlock restoreState(device);

	struct TVertex
	{
		float x, y, z, rhw;
		float u, v;
	};

	constexpr DWORD FVF = D3DFVF_XYZRHW | D3DFVF_TEX1;
	const float scale = 256.0f;

	const TVertex vertices[4] =
	{
		{ 0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f },
		{ scale, 0.0f,  0.0f, 1.0f, 1.0f, 0.0f },
		{ scale, scale, 0.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f,  scale, 0.0f, 1.0f, 0.0f, 1.0f },
	};

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	device->SetVertexShader(nullptr);
	device->SetPixelShader(nullptr);
	device->SetVertexDeclaration(nullptr);

	device->SetTexture(0, m_shadowMap.Get());
	device->SetTexture(1, nullptr);
	device->SetTexture(2, nullptr);

	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	device->SetFVF(FVF);
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TVertex));
}
