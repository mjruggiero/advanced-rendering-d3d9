/*
   Copyright (C) 2001 Nate Miller nathanm@uci.edu

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   See gpl.txt for more information regarding the GNU General Public License.
*/
#pragma once

//#include "font.h"
//#include "md3Player.h"
#include "md3.h"

namespace Framework
{
	class D3D9TextRenderer;
}

class UIAnimation
{
public:
	UIAnimation();
	~UIAnimation();

	void OneInit(Q3Player* QPlayer);
	void OnCreateDevice(IDirect3DDevice9* pd3dDevice);
	void OnResetDevice(IDirect3DDevice9* pd3dDevice);
	void OnLostDevice();
	void OnDestroyDevice();
	void FinalCleanUp();

	void Draw(float width, float height, Framework::D3D9TextRenderer& textRenderer);

	void AnimationSetNext(void);
	void AnimationSetPrev(void);
	void AnimationSectionToggle(void);

	ID3DXFont* m_pFont;
	ID3DXFont* m_pFontSmall;
	ID3DXSprite* m_pTextSprite;

protected:
	void CheckAnimations(void);

	bool upperiSCurrent;
	int iUpper;
	int iLower;
	Q3Player* m_dwPlayer;
};
