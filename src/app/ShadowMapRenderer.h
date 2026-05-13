#pragma once

#include "../framework/ComPtr.h"

#include <d3d9.h>
#include <d3dx9.h>

class Plane;
class Q3Player;

class ShadowMapRenderer final
{
public:
	static constexpr int Size = 2048;
	static constexpr D3DFORMAT Format = D3DFMT_R32F;
	static constexpr float NearPlane = 5.5f;
	static constexpr float FarPlane = 200.0f;

	bool CreateResetResources(IDirect3DDevice9* device);
	void DestroyResetResources();

	void Update(float timeSeconds);

	bool Render(
		IDirect3DDevice9* device,
		Q3Player* player,
		Plane* plane,
		const D3DXMATRIX& visibleViewProjection) const;

	void DrawPreview(IDirect3DDevice9* device) const;

	IDirect3DTexture9* Texture() const noexcept { return m_shadowMap.Get(); }
	const D3DXVECTOR3& LightViewPosition() const noexcept { return m_lightViewPosition; }
	const D3DXMATRIX& LightViewProjection() const noexcept { return m_lightViewProjection; }
	const D3DXMATRIX& ScaleBias() const noexcept { return m_scaleBias; }

	void TogglePreview() noexcept { m_showPreview = !m_showPreview; }
	bool IsPreviewVisible() const noexcept { return m_showPreview; }

private:
	Framework::ComPtr<IDirect3DTexture9> m_shadowMap;
	Framework::ComPtr<IDirect3DSurface9> m_shadowMapSurface;
	Framework::ComPtr<IDirect3DSurface9> m_shadowDepthSurface;

	bool m_showPreview = false;
	D3DXVECTOR3 m_lightViewPosition = D3DXVECTOR3(50.0f, 100.0f, 50.0f);
	D3DXMATRIX m_lightViewProjection = {};
	D3DXMATRIX m_scaleBias = {};
};
