#pragma once

#include "../framework/Application.h"
#include "../framework/D3D9TextRenderer.h"
#include "../framework/TrackballController.h"
#include "CharacterAppSettings.h"

#include <memory>
#include <string>

#include "../legacy/MD3.h"
#include "../legacy/Plane.h"
#include "../legacy/Sphere.h"
#include "../legacy/uiAnimation.h"

class CharacterApp final : public Framework::Application
{
protected:
	Framework::ApplicationConfig GetConfig() const override;

	bool Initialize() override;
	void Shutdown() override;

	bool CreateDeviceResources() override;
	void DestroyDeviceResources() override;
	bool CreateResetResources() override;
	void DestroyResetResources() override;

	void Update(float deltaSeconds) override;
	void Render(Framework::RenderContext& context) override;
	void RenderMeshes();
	bool RenderShadowMap();
	bool RenderScalePass();
	bool RenderBloomPass();
	bool RenderScreenPass();

	void DrawShadowMapPreview();

	void OnKeyDown(uint32_t vk) override;
	void OnKeyUp(uint32_t vk) override;
	void OnMouseButtonDown(uint32_t button, int x, int y) override;
	void OnMouseButtonUp(uint32_t button, int x, int y) override;
	void OnMouseMove(int x, int y, int dx, int dy) override;

	void RenderText();
	void LoadSettings() const;

private:
	bool m_wireframe = false;
	float m_appTimeSeconds = 0.0f;

	mutable bool m_settingsLoaded = false;
	mutable CharacterAppSettings m_settings;

	Framework::D3D9TextRenderer m_textRenderer;

	std::string m_modelPath = "../media/dragon";
	std::string m_modelName = "dragon";
	std::string m_skinName = "default";

	std::string m_weaponPath = "../media/railgun";
	std::string m_weaponName = "railgun";
	std::string m_weaponSkinName = "default";
	std::string m_shaderRoot = "shaders";

	float m_fovDegrees = 70.0f;
	float m_nearPlane = 1.0f;
	float m_farPlane = 1000.0f;

	int m_shaderProfile = 2;
	BYTE m_keys[256] = {};

	std::unique_ptr<D3DSphere> m_lightSphere;
	std::unique_ptr<UIAnimation> m_animationUi;
	std::unique_ptr<Q3Player> m_player;
	std::unique_ptr<Plane> m_plane;

	bool m_showHelp = false;
	bool m_showAnimationUi = false;
	bool m_moveLight = false;
	bool m_weaponVisible = false;

	D3DXVECTOR4 m_lightPosition = D3DXVECTOR4(0, -10, 40, 1);
	D3DXMATRIX m_projection = {};
	D3DXMATRIX m_view = {};
	D3DXMATRIX m_world = {};
	D3DXMATRIX m_viewProjection = {};

	// Model viewer controls.
	Framework::TrackballController m_trackball;
	float m_zoom = -120.0f;

	static constexpr int ShadowMapSize = 2048;
	static constexpr D3DFORMAT ShadowMapFormat = D3DFMT_R32F;
	static constexpr float ShadowNearPlane = 5.5f;
	static constexpr float ShadowFarPlane = 200.0f;

	bool m_showShadowMap = false;
	IDirect3DTexture9* m_shadowMap = nullptr;
	IDirect3DSurface9* m_shadowMapSurface = nullptr;
	IDirect3DSurface9* m_shadowDepthSurface = nullptr;

	// HDR stuff
	float m_fExposureLevel = 4.0f;
	bool m_bHDR = true;
	bool m_bOne = true;
	IDirect3DVertexDeclaration9* m_pDecl = nullptr;
	ID3DXEffect* m_pEffect = nullptr;
	IDirect3DTexture9* m_pRenderTarget = nullptr;
	IDirect3DSurface9* m_pSurface = nullptr;
	IDirect3DTexture9* m_pRenderTarget2 = nullptr;
	IDirect3DSurface9* m_pSurface2 = nullptr;
	IDirect3DTexture9* m_pRenderTarget3 = nullptr;
	IDirect3DSurface9* m_pSurface3 = nullptr;
	IDirect3DVertexBuffer9* m_pImageVB = nullptr;

	D3DXVECTOR3 m_lightViewPosition = D3DXVECTOR3(50.0f, 100.0f, 50.0f);
	D3DXMATRIX m_lightViewProjection = {};
	D3DXMATRIX m_scaleBias = {};
};
