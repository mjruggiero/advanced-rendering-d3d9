#include "HdrRenderer.h"

#include "../framework/FrameworkLog.h"

#include <algorithm>

namespace
{
	D3DVERTEXELEMENT9 FullscreenVertexDecl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	void LogEffectError(const std::string& prefix, ID3DXBuffer* errorBuffer)
	{
		if (errorBuffer)
		{
			Framework::FrameworkLog::WriteError(
				prefix + ": " + static_cast<const char*>(errorBuffer->GetBufferPointer()));
			errorBuffer->Release();
			return;
		}

		Framework::FrameworkLog::WriteError(prefix);
	}
}

bool HdrRenderer::CreateDeviceResources(IDirect3DDevice9* device, const std::string& effectPath)
{
	if (!device)
		return false;

	HRESULT hr = device->CreateVertexDeclaration(FullscreenVertexDecl, m_fullscreenDecl.Put());
	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create HDR fullscreen vertex declaration");
		return false;
	}

	ID3DXBuffer* errorBuffer = nullptr;
	hr = D3DXCreateEffectFromFileA(
		device,
		effectPath.c_str(),
		nullptr,
		nullptr,
		D3DXSHADER_DEBUG,
		nullptr,
		m_effect.Put(),
		&errorBuffer);

	if (FAILED(hr))
	{
		LogEffectError("Failed to create HDR effect: " + effectPath, errorBuffer);
		return false;
	}

	if (errorBuffer)
		errorBuffer->Release();

	return true;
}

void HdrRenderer::DestroyDeviceResources()
{
	m_effect.Reset();
	m_fullscreenDecl.Reset();
}

bool HdrRenderer::CreateResetResources(IDirect3DDevice9* device, UINT backBufferWidth, UINT backBufferHeight)
{
	if (!device)
		return false;

	HRESULT hr = device->CreateTexture(
		backBufferWidth,
		backBufferHeight,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		m_sceneTexture.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create HDR full-resolution render target");
		return false;
	}

	hr = m_sceneTexture->GetSurfaceLevel(0, m_sceneSurface.Put());
	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to get HDR full-resolution render target surface");
		return false;
	}

	const UINT bloomWidth = std::max<UINT>(1, backBufferWidth / 4);
	const UINT bloomHeight = std::max<UINT>(1, backBufferHeight / 4);

	hr = device->CreateTexture(
		bloomWidth,
		bloomHeight,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		m_bloomTextureA.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create HDR bloom render target 2");
		return false;
	}

	hr = m_bloomTextureA->GetSurfaceLevel(0, m_bloomSurfaceA.Put());
	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to get HDR bloom surface 2");
		return false;
	}

	hr = device->CreateTexture(
		bloomWidth,
		bloomHeight,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		m_bloomTextureB.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create HDR bloom render target 3");
		return false;
	}

	hr = m_bloomTextureB->GetSurfaceLevel(0, m_bloomSurfaceB.Put());
	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to get HDR bloom surface 3");
		return false;
	}

	hr = device->CreateVertexBuffer(
		4 * sizeof(FullscreenVertex),
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_DEFAULT,
		m_fullscreenVertexBuffer.Put(),
		nullptr);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to create HDR fullscreen image vertex buffer");
		return false;
	}

	FullscreenVertex* vertices = nullptr;
	hr = m_fullscreenVertexBuffer->Lock(
		0,
		4 * sizeof(FullscreenVertex),
		reinterpret_cast<void**>(&vertices),
		0);

	if (FAILED(hr))
	{
		Framework::FrameworkLog::WriteError("Failed to lock HDR fullscreen image vertex buffer");
		return false;
	}

	vertices[0] = { -1.0f, -1.0f };
	vertices[1] = { -1.0f,  1.0f };
	vertices[2] = {  1.0f, -1.0f };
	vertices[3] = {  1.0f,  1.0f };

	m_fullscreenVertexBuffer->Unlock();

	if (m_effect)
		m_effect->OnResetDevice();

	m_pingPongOnFirstTarget = true;
	return true;
}

void HdrRenderer::DestroyResetResources()
{
	if (m_effect)
		m_effect->OnLostDevice();

	m_fullscreenVertexBuffer.Reset();
	m_bloomSurfaceB.Reset();
	m_bloomTextureB.Reset();
	m_bloomSurfaceA.Reset();
	m_bloomTextureA.Reset();
	m_sceneSurface.Reset();
	m_sceneTexture.Reset();
}

bool HdrRenderer::BeginScene(IDirect3DDevice9* device, IDirect3DSurface9* backBuffer, D3DXVECTOR4& hdrEnable) const
{
	if (!device || !backBuffer)
		return false;

	IDirect3DSurface9* target = backBuffer;
	hdrEnable = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

	if (m_enabled)
	{
		if (!m_sceneSurface)
			return false;

		target = m_sceneSurface.Get();
		hdrEnable = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.0f);
	}

	HRESULT hr = device->SetRenderTarget(0, target);
	if (FAILED(hr))
		return false;

	device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		0x000000ff,
		1.0f,
		0);

	return true;
}

bool HdrRenderer::RenderPostProcess(IDirect3DDevice9* device, IDirect3DSurface9* backBuffer, UINT backBufferWidth, UINT backBufferHeight)
{
	if (!m_enabled)
		return true;

	if (!device || !backBuffer)
		return false;

	if (!RenderScalePass(device))
		return false;

	if (!RenderBloomPass(device, backBufferWidth, backBufferHeight))
		return false;

	if (FAILED(device->SetRenderTarget(0, backBuffer)))
		return false;

	return RenderScreenPass(device);
}

void HdrRenderer::AddExposure(float amount) noexcept
{
	m_exposureLevel = std::clamp(m_exposureLevel + amount, 1.0f, 32.0f);
}

bool HdrRenderer::RenderScalePass(IDirect3DDevice9* device)
{
	if (!device || !m_effect || !m_fullscreenDecl || !m_fullscreenVertexBuffer || !m_sceneTexture || !m_bloomSurfaceA)
		return false;

	device->SetRenderTarget(0, m_bloomSurfaceA.Get());

	m_effect->SetTechnique("ScaleBuffer");
	device->SetVertexDeclaration(m_fullscreenDecl.Get());
	m_effect->SetTexture("RenderMap", m_sceneTexture.Get());

	m_effect->Begin(nullptr, 0);
	m_effect->BeginPass(0);
	device->SetStreamSource(0, m_fullscreenVertexBuffer.Get(), 0, sizeof(FullscreenVertex));
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_effect->EndPass();
	m_effect->End();

	return true;
}

bool HdrRenderer::RenderBloomPass(IDirect3DDevice9* device, UINT backBufferWidth, UINT backBufferHeight)
{
	if (!device || !m_effect || !m_fullscreenDecl || !m_fullscreenVertexBuffer || !m_bloomTextureA || !m_bloomSurfaceA || !m_bloomTextureB || !m_bloomSurfaceB)
		return false;

	m_effect->SetTechnique("Bloom");
	device->SetVertexDeclaration(m_fullscreenDecl.Get());

	const float fPixelSizeX = -1.0f / (static_cast<float>(backBufferWidth) / 4.0f);
	const float fPixelSizeY = 1.0f / (static_cast<float>(backBufferHeight) / 4.0f);
	D3DXVECTOR4 pixelSizes(fPixelSizeX, fPixelSizeY, 1.0f, 1.0f);
	m_effect->SetVector("pixelSize", &pixelSizes);

	device->SetStreamSource(0, m_fullscreenVertexBuffer.Get(), 0, sizeof(FullscreenVertex));

	constexpr int BloomIterations = 8;
	for (int pass = 0; pass < BloomIterations; ++pass)
	{
		if (m_pingPongOnFirstTarget)
		{
			m_effect->SetTexture("RenderMap", m_bloomTextureA.Get());
			device->SetRenderTarget(0, m_bloomSurfaceB.Get());
			m_pingPongOnFirstTarget = false;
		}
		else
		{
			m_effect->SetTexture("RenderMap", m_bloomTextureB.Get());
			device->SetRenderTarget(0, m_bloomSurfaceA.Get());
			m_pingPongOnFirstTarget = true;
		}

		m_effect->SetFloat("fIteration", static_cast<float>(pass));

		m_effect->Begin(nullptr, 0);
		m_effect->BeginPass(0);
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		m_effect->EndPass();
		m_effect->End();
	}

	return true;
}

bool HdrRenderer::RenderScreenPass(IDirect3DDevice9* device)
{
	if (!device || !m_effect || !m_fullscreenDecl || !m_fullscreenVertexBuffer || !m_sceneTexture || !m_bloomTextureA)
		return false;

	m_effect->SetTechnique("Screenblit");
	device->SetVertexDeclaration(m_fullscreenDecl.Get());

	m_effect->SetTexture("FullResMap", m_sceneTexture.Get());
	m_effect->SetTexture("RenderMap", m_bloomTextureA.Get());
	m_effect->SetFloat("ExposureLevel", m_exposureLevel);

	m_effect->Begin(nullptr, 0);
	m_effect->BeginPass(0);
	device->SetStreamSource(0, m_fullscreenVertexBuffer.Get(), 0, sizeof(FullscreenVertex));
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_effect->EndPass();
	m_effect->End();

	return true;
}
