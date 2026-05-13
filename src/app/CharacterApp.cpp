#include "CharacterApp.h"

#include "../framework/FrameworkLog.h"
#include "../framework/D3D9StateBlock.h"
#include "../legacy/logger.h"
#include "../legacy/D3D9Compat.h"

#include <d3dx9.h>

#include <algorithm>

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

Framework::ApplicationConfig CharacterApp::GetConfig() const
{
	LoadSettings();

	Framework::ApplicationConfig config;
	config.title = ToWideString(m_settings.windowTitle);
	config.width = m_settings.windowWidth;
	config.height = m_settings.windowHeight;
	config.windowed = m_settings.windowed;
	config.assetRoot = ToWideString(m_settings.assetRoot);
	config.logFile = ToWideString(m_settings.logFile);
	config.clearColor = m_settings.clearColor;
	return config;
}

void CharacterApp::LoadSettings() const
{
	if (m_settingsLoaded)
		return;

	m_settings = LoadCharacterAppSettings();
	m_settingsLoaded = true;
}

bool CharacterApp::Initialize()
{
	LoadSettings();

	// Switch the legacy logger to the configured file before the MD3/Q3 loaders start logging.
	logger.setLogFile(m_settings.logFile);

	Framework::FrameworkLog::WriteInfo("CharacterApp initialized");

	m_modelName = m_settings.modelName;
	m_skinName = m_settings.skinName;
	m_modelPath = m_settings.modelPath;
	m_weaponName = m_settings.weaponName;
	m_weaponSkinName = m_settings.weaponSkinName;
	m_weaponPath = m_settings.weaponPath;
	m_shaderRoot = m_settings.shaderRoot;
	m_shaderProfile = m_settings.shaderProfile;
	m_wireframe = m_settings.wireframe;
	m_weaponVisible = m_settings.showWeapon;
	m_moveLight = m_settings.moveLight;
	m_zoom = m_settings.zoom;
	m_trackball.SetDistance(m_zoom);
	m_fovDegrees = m_settings.fovDegrees;
	m_nearPlane = m_settings.nearPlane;
	m_farPlane = m_settings.farPlane;
	m_lightPosition = m_settings.lightPosition;

	// Init keyboard status
	std::fill(std::begin(m_keys), std::end(m_keys), 0);

	// Keep the temporary app-owned shader profile in sync with the legacy MD3 global.
	iShaderProfile = m_shaderProfile;

	// Init world matrix
	D3DXMatrixIdentity(&m_world);

	m_lightSphere = std::make_unique<D3DSphere>();
	m_player = std::make_unique<Q3Player>();
	m_animationUi = std::make_unique<UIAnimation>();
	m_plane = std::make_unique<Plane>();

	// Load model
	m_player->LoadPlayerGeometry(m_modelPath.c_str());
	m_player->LoadWeaponGeometry(m_weaponPath.c_str(), m_weaponName.c_str());
	m_animationUi->OneInit(m_player.get());	// user interface animations

	if (m_weaponVisible)
		m_player->md3Upper.LinkModel("tag_weapon", &m_player->md3Weapon);

	return true;
}

void CharacterApp::Shutdown()
{
	Framework::FrameworkLog::WriteInfo("CharacterApp shutdown");

}

bool CharacterApp::CreateDeviceResources()
{
	Framework::FrameworkLog::WriteInfo("CharacterApp CreateDeviceResources");

	// Initialize shared HUD/debug text resources.
	if (!m_textRenderer.Create(Device()))
		return false;

	IDirect3DDevice9* device = Device();
	if (!device)
		return false;

	m_animationUi->OnCreateDevice(Device());		// user interface animations
	// initialize sphere
	m_lightSphere->OnCreateDevice(Device());

	// shader loading
	m_player->LoadPlayerShaders(Device(), m_modelPath.c_str(), m_modelName.c_str(), m_skinName.c_str(), m_shaderRoot.c_str());
	m_player->LoadWeaponShaders(Device(), m_weaponPath.c_str(), m_weaponName.c_str(), m_weaponSkinName.c_str(), m_shaderRoot.c_str());

	m_player->LoadSkins(Device(), m_modelPath.c_str(), m_skinName.c_str());
	m_player->LoadWeaponSkins(Device(), m_weaponPath.c_str(), m_weaponSkinName.c_str());	// load skin of weapon

	m_plane->InitDeviceObjects(Device());
	m_plane->LoadShaders(Device(), "plane_default.sha", "");

	const std::string hdrEffectPath = m_shaderRoot + "/HDR.fx";
	if (!m_hdr.CreateDeviceResources(device, hdrEffectPath))
		return false;

	return true;
}

void CharacterApp::DestroyDeviceResources()
{
	Framework::FrameworkLog::WriteInfo("CharacterApp DestroyDeviceResources");

	// Release resources created in OnCreateDevice.
	m_textRenderer.Release();

	if (m_animationUi)
		m_animationUi->OnDestroyDevice();

	if (m_lightSphere)
		m_lightSphere->OnDestroyDevice();

	if (m_plane)
		m_plane->DeleteDeviceObjects();

	if (m_player)
	{
		// free model/weapon shaders
		m_player->FreePlayerShaders(Device());
		m_player->FreeWeaponShaders(Device());

		m_player->FreeSkins();
		m_player->FreeWeaponSkins();
	}

	m_hdr.DestroyDeviceResources();
}

bool CharacterApp::CreateResetResources()
{
	Framework::FrameworkLog::WriteInfo("CharacterApp CreateResetResources");

	IDirect3DDevice9* device = Device();
	if (!device)
		return false;

	device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	if (!m_textRenderer.CreateResetResources(device))
		return false;

	m_animationUi->OnResetDevice(device);
	m_lightSphere->OnResetDevice(device); // sphere

	// create dynamic vertex and index buffer
	m_player->CreateVertexNIndexBuffer(device);

	m_plane->Create(100.0f, 100.0f);
	m_plane->RestoreDeviceObjects();

	if (!m_shadowMapRenderer.CreateResetResources(device))
		return false;

	const UINT backBufferWidth = BackBufferWidth();
	const UINT backBufferHeight = BackBufferHeight();
	if (!m_hdr.CreateResetResources(device, backBufferWidth, backBufferHeight))
		return false;

	// Setup the camera's projection parameters
	const float fAspectRatio = BackBufferAspectRatio();

	// setup projection matrix
	D3DXMatrixPerspectiveFovLH(&m_projection, D3DXToRadian(m_fovDegrees),
		fAspectRatio, m_nearPlane, m_farPlane);

	// Configure model viewer controls.
	m_trackball.SetViewport(DeviceContext().Width(), DeviceContext().Height());
	m_trackball.SetDistance(m_zoom);

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	device->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	// constant values
	D3DXVECTOR4 comp(1.0f / 127.5f * 250.01f, 1.0f, 0.5f, 1.0f);
	device->SetVertexShaderConstantF(33, (float*)&comp, 1);

	return true;
}

void CharacterApp::DestroyResetResources()
{
	Framework::FrameworkLog::WriteInfo("CharacterApp DestroyResetResources");

	m_shadowMapRenderer.DestroyResetResources();
	m_hdr.DestroyResetResources();

	// Release resources created in CreateResetResources.
	// D3DXFont/D3DXSprite DestroyResetResources calls also belong here.
	m_textRenderer.DestroyResetResources();

	if (m_animationUi)
		m_animationUi->OnLostDevice();

	if (m_lightSphere)
		m_lightSphere->OnLostDevice();

	if (m_plane)
		m_plane->InvalidateDeviceObjects();

	if (m_player)
	{
		// delete vertex and index buffer for player	
		m_player->DeleteVertexNIndexBuffer();
	}

}

void CharacterApp::Update(float deltaSeconds)
{
	// Update animation, light, camera, and input-driven state.
	(void)deltaSeconds;

	IDirect3DDevice9* device = Device();
	if (!device)
		return;

	m_zoom = m_trackball.Distance();
	m_world = m_trackball.WorldMatrix();
	m_player->SetWorldMatrix(m_world);

	// setup view matrix
	D3DXVECTOR3 vUpVec(0, 1, 0);
	D3DXVECTOR3 vEyePt(0, 0, m_zoom);
	D3DXVECTOR3 vLookatPt(0, 0, 0);
	D3DXMatrixLookAtLH(&m_view, &vEyePt, &vLookatPt, &vUpVec);

	// invert the view position to get a right handed-model
	m_view._31 = -m_view._31;
	m_view._32 = -m_view._32;
	m_view._33 = -m_view._33;
	m_view._34 = -m_view._34;

	// precompute view/proj concatenation and deliver it to 
	// the player class
	D3DXMatrixMultiply(&m_viewProjection, &m_view, &m_projection);
	m_player->SetViewProjMatrix(m_viewProjection);

	// eye vector
	device->SetVertexShaderConstantF(24, (float*)&vEyePt, 1);

	// process keyboard input
	if (m_moveLight)
	{
		if (m_keys[VK_LEFT]) { m_keys[VK_RIGHT] = 0; m_lightPosition.x -= 100.0f * deltaSeconds; }
		if (m_keys[VK_RIGHT]) { m_keys[VK_LEFT] = 0;  m_lightPosition.x += 100.0f * deltaSeconds; }
		if (m_keys[VK_UP]) { m_keys[VK_DOWN] = 0;  m_lightPosition.y += 100.0f * deltaSeconds; }
		if (m_keys[VK_DOWN]) { m_keys[VK_UP] = 0;	   m_lightPosition.y -= 100.0f * deltaSeconds; }
		if (m_keys[VK_END]) { m_keys[VK_HOME] = 0;  m_lightPosition.z -= 100.0f * deltaSeconds; }
		if (m_keys[VK_HOME]) { m_keys[VK_END] = 0;   m_lightPosition.z += 100.0f * deltaSeconds; }

		m_lightSphere->SetLightPos(m_lightPosition);
	}

	// light direction
	device->SetVertexShaderConstantF(12, (float*)&m_lightPosition, 1);	// light direction

	if (m_keys['P']) { m_keys['P'] = 0; m_moveLight = !m_moveLight; }

	if (m_keys['G']) {
		m_keys['G'] = FALSE;
		if (m_weaponVisible) m_player->md3Upper.LinkModel("tag_weapon", nullptr);
		else m_player->md3Upper.LinkModel("tag_weapon", &m_player->md3Weapon);
		m_weaponVisible = !m_weaponVisible;
	}

	if (m_keys['W'])
	{
		m_keys['W'] = FALSE;
		m_wireframe = !m_wireframe;
	}

	// turn HDR on/off
	if (m_keys['H'])
	{
		m_keys['H'] = NULL;
		m_hdr.ToggleEnabled();
	}

	// turn shadow map preview on/off
	if (m_keys['M'])
	{
		m_keys['M'] = FALSE;
		m_shadowMapRenderer.TogglePreview();
	}

	if (m_keys['R'])
	{
		//m_keys['R'] = NULL;
		m_hdr.AddExposure(10.0f * deltaSeconds);
	}
	else if (m_keys['F'])
	{
		//m_keys['F'] = NULL;
		m_hdr.AddExposure(-10.0f * deltaSeconds);
	}
	// choose animations
	if (m_showAnimationUi)
	{
		if (m_keys['K']) { m_keys['K'] = FALSE; m_animationUi->AnimationSetNext(); }
		if (m_keys['I']) { m_keys['I'] = FALSE; m_animationUi->AnimationSetPrev(); }
		if (m_keys['J'] || m_keys['L']) {
			m_keys['L'] = FALSE;m_keys['J'] = FALSE;
			m_animationUi->AnimationSectionToggle();
		}
	}

	// set shader profile
	if (m_keys[VK_ADD] || m_keys[VK_OEM_PLUS])
	{
		m_keys[VK_ADD] = FALSE;
		m_keys[VK_OEM_PLUS] = FALSE;
		m_shaderProfile++;
		if (m_shaderProfile >= MAXSHADERPROFILE)
			m_shaderProfile = 0;
		iShaderProfile = m_shaderProfile;
	}

	// set shader profile
	if (m_keys[VK_SUBTRACT] || m_keys[VK_OEM_MINUS])
	{
		m_keys[VK_SUBTRACT] = FALSE;
		m_keys[VK_OEM_MINUS] = FALSE;
		m_shaderProfile--;
		if (m_shaderProfile < 0)
			m_shaderProfile = MAXSHADERPROFILE - 1;
		iShaderProfile = m_shaderProfile;
	}

	// toggle options help text
	if (m_keys['O'])
	{
		m_keys['O'] = FALSE;
		m_showAnimationUi = FALSE;
		m_showHelp = !m_showHelp;
	}


	// toggle animations user interface
	if (m_keys['A'])
	{
		m_keys['A'] = FALSE;
		m_showHelp = FALSE;
		m_showAnimationUi = !m_showAnimationUi;
	}

	m_appTimeSeconds += deltaSeconds;
	m_player->Update(m_appTimeSeconds);

	m_shadowMapRenderer.Update(m_appTimeSeconds);
	m_player->SetLightViewProjMatrix(m_shadowMapRenderer.LightViewProjection());
	m_player->SetScaleBiasMatrix(m_shadowMapRenderer.ScaleBias());
}

void CharacterApp::Render(Framework::RenderContext& context)
{
	IDirect3DDevice9* device = context.Device();
	if (!device)
		return;

	//
	// 1. Render shadow map.
	// RenderShadowMap owns shadow RT/depth switching and restores the previous target.
	//
	if (!m_shadowMapRenderer.Render(device, m_player.get(), m_plane.get(), m_viewProjection))
	{
		Framework::FrameworkLog::WriteError("RenderShadowMap failed");
	}

	//
	// 2. Save current backbuffer for HDR restore.
	//
	IDirect3DSurface9* oldBackBuffer = nullptr;
	if (FAILED(device->GetRenderTarget(0, &oldBackBuffer)))
		return;

	//
	// 3. Choose main scene target.
	//
	D3DXVECTOR4 hdrEnable(0.0f, 0.0f, 0.0f, 0.0f);
	if (!m_hdr.BeginScene(device, oldBackBuffer, hdrEnable))
	{
		SAFE_RELEASE(oldBackBuffer);
		Framework::FrameworkLog::WriteError("HDR BeginScene failed");
		return;
	}

	//
	// 4. Restore normal scene render state, then bind shadow/HDR constants.
	//
	context.SetDefault3DState();
	context.SetWireframe(m_wireframe);

	device->SetTexture(2, m_shadowMapRenderer.Texture());
	device->SetPixelShaderConstantF(29, reinterpret_cast<const float*>(&hdrEnable), 1);

	RenderMeshes();

	device->SetTexture(2, nullptr);

	//
	// 5. Post-process HDR back to backbuffer.
	//
	if (!m_hdr.RenderPostProcess(device, oldBackBuffer, BackBufferWidth(), BackBufferHeight()))
	{
		Framework::FrameworkLog::WriteError("HDR post-process failed");
	}

	SAFE_RELEASE(oldBackBuffer);

	//
	// 6. Debug/overlay.
	//
	if (m_moveLight && m_lightSphere)
		m_lightSphere->RenderSphere(device, m_viewProjection);

	if (m_shadowMapRenderer.IsPreviewVisible())
		m_shadowMapRenderer.DrawPreview(device);

	RenderText();
}

void CharacterApp::RenderMeshes()
{
	IDirect3DDevice9* device = Device();
	if (!device)
		return;

	D3DXMATRIX modelLightVP = m_world * m_shadowMapRenderer.LightViewProjection();
	D3DXMATRIX modelLightTex = modelLightVP * m_shadowMapRenderer.ScaleBias();

	D3DXMatrixTranspose(&modelLightVP, &modelLightVP);
	D3DXMatrixTranspose(&modelLightTex, &modelLightTex);

	device->SetVertexShaderConstantF(16, reinterpret_cast<const float*>(&modelLightVP), 4);
	device->SetVertexShaderConstantF(20, reinterpret_cast<const float*>(&modelLightTex), 4);
	D3DXVECTOR4 planeConstants(ShadowMapRenderer::FarPlane, ShadowMapRenderer::NearPlane, 0.0f, 0.0f);
	device->SetVertexShaderConstantF(
		34,
		reinterpret_cast<const float*>(&planeConstants),
		1);

	if (m_player)
	{
		//m_player->SetShaderProfile(m_shaderProfile);
		ScopedLegacyShaderProfile visibleProfile(m_shaderProfile);
		m_player->Draw(device);
	}

	D3DXMATRIX planeLightVP = m_shadowMapRenderer.LightViewProjection();
	D3DXMATRIX planeLightTex = planeLightVP * m_shadowMapRenderer.ScaleBias();

	D3DXMatrixTranspose(&planeLightVP, &planeLightVP);
	D3DXMatrixTranspose(&planeLightTex, &planeLightTex);

	device->SetVertexShaderConstantF(16, reinterpret_cast<const float*>(&planeLightVP), 4);
	device->SetVertexShaderConstantF(20, reinterpret_cast<const float*>(&planeLightTex), 4);
	device->SetVertexShaderConstantF(
		34,
		reinterpret_cast<const float*>(&planeConstants),
		1);

	if (m_plane)
	{
		m_plane->SetShaderProfile(0);
		m_plane->Render(m_viewProjection);
	}
}

void CharacterApp::RenderText()
{
	IDirect3DDevice9* device = Device();
	if (!device || !m_textRenderer.IsValid())
		return;

	Framework::D3D9ScopedStateBlock restoreState(device);

	const D3DCOLOR yellow = D3DCOLOR_ARGB(255, 255, 255, 0);
	const D3DCOLOR white = D3DCOLOR_ARGB(255, 255, 255, 255);

	WCHAR strShaderLevelStats[1024] = {};

	if (!m_textRenderer.Begin())
		return;

	//
	// Draw animation user interface.
	// CUIAnimation now borrows CharacterApp's D3D9TextRenderer, so the whole
	// HUD can be drawn inside one ID3DXSprite batch.
	//
	if (m_showAnimationUi)
	{
		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Normal,
			2,
			0,
			yellow,
			L"Animations (A - Back)");

		if (m_animationUi)
			m_animationUi->Draw(2, 15, m_textRenderer);

		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			2,
			250,
			white,
			L"J/L/K/I - Choose Animation");

		m_textRenderer.End();
		return;
	}

	//
	// Draw options.
	//
	if (m_showHelp)
	{
		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Normal,
			2,
			0,
			yellow,
			L"Options (O - Back)");

		swprintf_s(
			strShaderLevelStats,
			L"+/- on Num Pad\n"
			L"G\n"
			L"W\n"
			L"P - Up/Down/Left/Right\n"
			L"M -\n"
			L"R/F\n"
			L"H\n");

		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			2,
			15,
			white,
			strShaderLevelStats);

		swprintf_s(
			strShaderLevelStats,
			L"- Shader Profile #%d\n"
			L"- Toggle Weapon\n"
			L"- Toggle Wireframe Mode\n"
			L"- Toggle - Move Point Light\n"
			L"- Toggle Shadow Map Preview\n"
			L"- Increase/Decrease Exposure Level\n"
			L"- Toggle HDR rendering\n",
			m_shaderProfile);

		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			150,
			15,
			white,
			strShaderLevelStats);
	}
	else
	{
		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Normal,
			2,
			0,
			yellow,
			L"Character Engine");

		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			2,
			16,
			white,
			L"Standalone D3D9 Framework");

		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			2,
			32,
			white,
			L"A - Animations\nO - Options");
		WCHAR strExposure[256] = {};
		swprintf_s(strExposure, L"Exposure Level %.3f", m_hdr.ExposureLevel());
		m_textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			2,
			64,
			white,
			strExposure);
	}

	m_textRenderer.End();
}

void CharacterApp::OnKeyDown(uint32_t vk)
{
	if (vk < 256)
		m_keys[vk] = TRUE;
}

void CharacterApp::OnKeyUp(uint32_t vk)
{
	if (vk < 256)
		m_keys[vk] = FALSE;
}

void CharacterApp::OnMouseButtonDown(uint32_t button, int x, int y)
{
	m_trackball.OnMouseButtonDown(button, x, y);
}

void CharacterApp::OnMouseButtonUp(uint32_t button, int x, int y)
{
	m_trackball.OnMouseButtonUp(button, x, y);
}

void CharacterApp::OnMouseMove(int x, int y, int dx, int dy)
{
	m_trackball.OnMouseMove(x, y, dx, dy);
}
