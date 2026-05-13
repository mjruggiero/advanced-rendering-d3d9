#pragma once

#include "../framework/ComPtr.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <string>

class HdrRenderer final
{
public:
	bool CreateDeviceResources(IDirect3DDevice9* device, const std::string& effectPath);
	void DestroyDeviceResources();

	bool CreateResetResources(IDirect3DDevice9* device, UINT backBufferWidth, UINT backBufferHeight);
	void DestroyResetResources();

	bool BeginScene(IDirect3DDevice9* device, IDirect3DSurface9* backBuffer, D3DXVECTOR4& hdrEnable) const;
	bool RenderPostProcess(IDirect3DDevice9* device, IDirect3DSurface9* backBuffer, UINT backBufferWidth, UINT backBufferHeight);

	bool IsEnabled() const noexcept { return m_enabled; }
	void ToggleEnabled() noexcept { m_enabled = !m_enabled; }

	float ExposureLevel() const noexcept { return m_exposureLevel; }
	void AddExposure(float amount) noexcept;

private:
	struct FullscreenVertex
	{
		float x = 0.0f;
		float y = 0.0f;
	};

	bool RenderScalePass(IDirect3DDevice9* device);
	bool RenderBloomPass(IDirect3DDevice9* device, UINT backBufferWidth, UINT backBufferHeight);
	bool RenderScreenPass(IDirect3DDevice9* device);

	bool m_enabled = true;
	bool m_pingPongOnFirstTarget = true;
	float m_exposureLevel = 4.0f;

	Framework::ComPtr<IDirect3DVertexDeclaration9> m_fullscreenDecl;
	Framework::ComPtr<ID3DXEffect> m_effect;
	Framework::ComPtr<IDirect3DTexture9> m_sceneTexture;
	Framework::ComPtr<IDirect3DSurface9> m_sceneSurface;
	Framework::ComPtr<IDirect3DTexture9> m_bloomTextureA;
	Framework::ComPtr<IDirect3DSurface9> m_bloomSurfaceA;
	Framework::ComPtr<IDirect3DTexture9> m_bloomTextureB;
	Framework::ComPtr<IDirect3DSurface9> m_bloomSurfaceB;
	Framework::ComPtr<IDirect3DVertexBuffer9> m_fullscreenVertexBuffer;
};
