//-----------------------------------------------------------------------------
// File:	Utility.h
//
// Desc:	small little helpers
//
// Last modification: November 16, 2001
//
// Credits: 
//
// Copyright (c) 2001 - 2002 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __utility_h__
#define __utility_h__

#include "d3d9.h"
#include <D3DX9.h>
/*
HRESULT CreateVSFromCompiledFile (IDirect3DDevice9* m_pd3dDevice,
													 DWORD* dwDeclaration,
													 TCHAR* strVSPath,
													 DWORD* m_dwVS);

HRESULT CreatePSFromCompiledFile (IDirect3DDevice9* m_pd3dDevice,
													 TCHAR* strPSPath,
													 DWORD* dwPS);
*/
void ParseTextLine(char* strTextLine, char* strTok);
INT CheckNumber(const CHAR* str);
INT ParseNumber(const char* str);
INT CheckFile(const char* filename);

#endif