//-----------------------------------------------------------------------------
// File:	uiAnimation.cpp
//
// Desc:	loads and renders *.md3 files
//
// Last modification: March 31th, 2003
//
// Credits: 
// This piece of code is inspired by the md3 viewer source of Nate Miller
//
// Copyright (c) 2001 - 2003 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------
#include "D3D9Compat.h"


#include "uiAnimation.h"
#include "../framework/D3D9TextRenderer.h"

//---------------------------------------------------
// names of animations
//
//---------------------------------------------------
const TCHAR* animString[40] = {
	L"BOTH_DEATH1",
	L"BOTH_DEAD1",
	L"BOTH_DEATH2",
	L"BOTH_DEAD2",
	L"BOTH_DEATH3",
	L"BOTH_DEAD3",

	L"TORSO_GESTURE",

	L"TORSO_ATTACK",
	L"TORSO_ATTACK2",

	L"TORSO_DROP",
	L"TORSO_RAISE",

	L"TORSO_STAND",
	L"TORSO_STAND2",

	L"LEGS_WALKCR",
	L"LEGS_WALK",
	L"LEGS_RUN",
	L"LEGS_BACK",
	L"LEGS_SWIM",

	L"LEGS_JUMP",
	L"LEGS_LAND",

	L"LEGS_JUMPB",
	L"LEGS_LANDB",

	L"LEGS_IDLE",
	L"LEGS_IDLECR",

	L"LEGS_TURN",

	L"MAX_ANIMATIONS"
};

//---------------------------------------------------------------
// CUIAnimation()
// 
// Desc: constructor
//---------------------------------------------------------------
UIAnimation::UIAnimation()
{
	iUpper = TORSO_STAND;
	iLower = LEGS_IDLE;
	upperiSCurrent = 1;
	m_dwPlayer = 0;
	m_pFont = NULL;
	m_pFontSmall = NULL;
	m_pTextSprite = NULL;   // Sprite for batching draw text calls
}

//---------------------------------------------------------------
// FinalCleanUp()
// 
// Desc: free space
//---------------------------------------------------------------
UIAnimation::~UIAnimation()
{
}

//---------------------------------------------------------------
// Init()
// 
// Desc: sets the animation state on the passed player
//---------------------------------------------------------------
void UIAnimation::OneInit(Q3Player* p)
{
	m_dwPlayer = p;
	p->SetLowerAnim(iLower);
	p->SetUpperAnim(iUpper);
}

//---------------------------------------------------------------
// Init()
// 
// Desc: sets the animation state on the passed player
//---------------------------------------------------------------
void UIAnimation::OnCreateDevice(IDirect3DDevice9* pd3dDevice)
{
	(void)pd3dDevice;
	// Text resources are owned by CharacterApp through Framework::D3D9TextRenderer.
}

//---------------------------------------------------------------
// OnResetDevice()
// 
// Desc: 
//---------------------------------------------------------------
void UIAnimation::OnResetDevice(IDirect3DDevice9* pd3dDevice)
{
	(void)pd3dDevice;
	// Text resources are owned by CharacterApp through Framework::D3D9TextRenderer.
}

//---------------------------------------------------------------
// OnLostDevice()
// 
// Desc: 
//---------------------------------------------------------------
void UIAnimation::OnLostDevice()
{
	// Text resources are owned by CharacterApp through Framework::D3D9TextRenderer.
}

//---------------------------------------------------------------
// OnDestroyDevice()
// 
// Desc: 
//---------------------------------------------------------------
void UIAnimation::OnDestroyDevice()
{
	// Text resources are owned by CharacterApp through Framework::D3D9TextRenderer.
}

//---------------------------------------------------------------
// Draw()
// 
// Desc: this is the main function, that draws the animation
//---------------------------------------------------------------
void UIAnimation::Draw(float width, float height, Framework::D3D9TextRenderer& textRenderer)
{
	// The caller owns textRenderer.Begin()/End(). This avoids nested ID3DXSprite batches.

	// Font colors.
	static const DWORD dwTitle = D3DCOLOR_ARGB(255, 200, 255, 200);
	static const DWORD dwTitleDead = D3DCOLOR_ARGB(255, 126, 126, 126);
	static const DWORD dwCurrent = D3DCOLOR_ARGB(255, 255, 0, 0);
	static const DWORD dwCurrentDead = D3DCOLOR_ARGB(255, 126, 0, 0);
	static const DWORD dwItem = D3DCOLOR_ARGB(255, 255, 255, 255);
	static const DWORD dwItemDead = D3DCOLOR_ARGB(255, 126, 126, 126);

	static const float size = 12.0f;

	DWORD dwCurrentColor = 0;
	float fXPosition = width;
	float fYPosition = height;

	//
	// Upper animation list.
	//
	dwCurrentColor = upperiSCurrent ? dwTitle : dwTitleDead;

	textRenderer.DrawLine(
		Framework::D3D9TextRenderer::FontSize::Normal,
		static_cast<int>(fXPosition),
		static_cast<int>(fYPosition),
		dwCurrentColor,
		L"Upper");

	fYPosition += size + 4.0f;

	for (int curr = BOTH_DEATH1; curr <= TORSO_STAND2; ++curr)
	{
		if (curr == iUpper)
			dwCurrentColor = upperiSCurrent ? dwCurrent : dwCurrentDead;
		else
			dwCurrentColor = upperiSCurrent ? dwItem : dwItemDead;

		textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			static_cast<int>(fXPosition),
			static_cast<int>(fYPosition),
			dwCurrentColor,
			animString[curr]);

		fYPosition += size;
	}

	//
	// Lower animation list.
	//
	fXPosition = width + size * 9.0f;
	fYPosition = height;

	dwCurrentColor = !upperiSCurrent ? dwTitle : dwTitleDead;

	textRenderer.DrawLine(
		Framework::D3D9TextRenderer::FontSize::Normal,
		static_cast<int>(fXPosition),
		static_cast<int>(fYPosition),
		dwCurrentColor,
		L"Lower");

	fYPosition += size + 4.0f;

	for (int curr = BOTH_DEATH1; curr <= LEGS_TURN; ++curr)
	{
		if (curr == TORSO_GESTURE)
			curr = LEGS_WALKCR;

		if (curr == iLower)
			dwCurrentColor = !upperiSCurrent ? dwCurrent : dwCurrentDead;
		else
			dwCurrentColor = !upperiSCurrent ? dwItem : dwItemDead;

		textRenderer.DrawLine(
			Framework::D3D9TextRenderer::FontSize::Small,
			static_cast<int>(fXPosition),
			static_cast<int>(fYPosition),
			dwCurrentColor,
			animString[curr]);

		fYPosition += size;
	}
}

//---------------------------------------------------------------
// AnimationSetNext()
// 
// Desc: sets the next animation
//---------------------------------------------------------------
void UIAnimation::AnimationSetNext(void)
{
	if (upperiSCurrent)
		++iUpper;
	else
		++iLower;

	CheckAnimations();
}

//---------------------------------------------------------------
// AnimationSetPrev()
// 
// Desc: sets the previous animation
//---------------------------------------------------------------
void UIAnimation::AnimationSetPrev(void)
{
	if (upperiSCurrent)
		--iUpper;
	else
		--iLower;

	CheckAnimations();
}

//---------------------------------------------------------------
// AnimationSectionToggle()
// 
// Desc: toggles between the upper animation and the lower animation set
//---------------------------------------------------------------
void UIAnimation::AnimationSectionToggle(void)
{
	upperiSCurrent = !upperiSCurrent;
}

//---------------------------------------------------------------
// CheckAnimation()
// 
// Desc: sets only useful animation combinations
//---------------------------------------------------------------
void UIAnimation::CheckAnimations(void)
{
	if (upperiSCurrent)
	{
		if (iUpper > TORSO_STAND2)
			iUpper = BOTH_DEATH1;

		if (iUpper < BOTH_DEATH1)
			iUpper = TORSO_STAND2;

		m_dwPlayer->SetUpperAnim(iUpper);
	}
	else
	{
		if (iLower > BOTH_DEAD3 && iLower < LEGS_WALKCR && iLower == TORSO_GESTURE)
			iLower = LEGS_WALKCR;

		if (iLower < LEGS_WALKCR && iLower > BOTH_DEAD3)
			iLower = BOTH_DEAD3;

		if (iLower < BOTH_DEATH1)
			iLower = LEGS_TURN;

		if (iLower > LEGS_TURN)
			iLower = BOTH_DEATH1;

		m_dwPlayer->SetLowerAnim(iLower);
	}
}
