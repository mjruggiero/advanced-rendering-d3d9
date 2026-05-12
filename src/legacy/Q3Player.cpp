//-----------------------------------------------------------------------------
// File: Q3Player.cpp
// Desc: Q3Player implementation split from legacy MD3.cpp
//-----------------------------------------------------------------------------
#include "D3D9Compat.h"
#include "Q3Player.h"

#include "logger.h"
#include "Utility.h"

//-------------------------------------------------------
// Q3Player
//-------------------------------------------------------
Q3Player::Q3Player()
{
	playerAnimLower = LEGS_IDLE;
	playerAnimUpper = TORSO_STAND;

	m_pVB = NULL;
	m_pIB = NULL;
	m_pVertexDeclaration = NULL;

	D3DXMatrixIdentity(&matViewProj);
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixIdentity(&matLightViewProj);
	D3DXMatrixIdentity(&matScaleBias);
}

//-------------------------------------------------------
// ~Q3Player
//-------------------------------------------------------
Q3Player::~Q3Player()
{
}

//-----------------------------------------------------------
// LoadAnim
//
// Desc: Load animation.cfg for Q3Player
//----------------------------------------------------------
void Q3Player::LoadAnim(const char* filename)
{
	LOGFUNC("LoadAnim()");

	FILE* AnimFile;
	CHAR	pcTextLine[1024];
	CHAR    strToken[1024];
	char	cOneChar;
	int		i;
	int		a = 0, t = 0;
	int		iSex, iHeadOffset, iFootSteps;
	INT		iFirstFrame, iNumFrames;
	INT		iPreviousFirstFrame = 0;
	INT		iPreviousNumFrames = 0;

	if (CheckFile(filename) == 0)
		return;

	AnimFile = fopen(filename, "rt");

	while (!feof(AnimFile))
	{
		cOneChar = fgetc(AnimFile);

		i = 0;
		pcTextLine[0] = 0;
		// 10 - new line
		// 32 - space
		// 255 - eof (-1)
		while (cOneChar != 10 && cOneChar != 255 && cOneChar != -1)
		{
			pcTextLine[i] = cOneChar; i++;
			cOneChar = fgetc(AnimFile);
		}
		pcTextLine[i] = '\0';

		t = 0;
		iSex = 0;
		iHeadOffset = 0;
		iFootSteps = 0;
		while (pcTextLine && pcTextLine[0])
		{
			// returns in strToken a word of a textline
			// returns in pcTextLine the next word of this textline
			// Reads only words consisting of chars, numbers, points and underscores
			ParseTextLine(pcTextLine, strToken);

			if (strToken && strToken[0] != 0)
			{
				if (strstr(strToken, "sex")) iSex = 1;				// not used here
				if (strstr(strToken, "headoffset")) iHeadOffset = 1;	// not used here
				if (strstr(strToken, "footsteps")) iFootSteps = 1;	// not used here

				if (CheckNumber(strToken) && !iSex && !iHeadOffset && !iFootSteps)
				{
					if (t == 0) { iFirstFrame = atoi(strToken);t++; }
					else if (t == 1) { iNumFrames = atoi(strToken); t++; }
					else if (t == 2) { playerAnim[a].iLoopingFrame = atoi(strToken); t++; }
					else if (t == 3) { playerAnim[a].iFramesPerSecond = atoi(strToken); t++; }

					/*
										if NumberOfLoopFrames == 0
											amnt = animationsEnd - AnimationStart - NumberLoopFrames;
										else
											amnt = 0;
									   if (NextFrame >= AnimStart + iNumFrames || NextFrame >= MAX_ANIMATIONS)
										   NextFrame = AnimStart + anmt;
					*/
					if (t == 4)
					{
						if ((iPreviousFirstFrame + iPreviousNumFrames) != iFirstFrame)
						{
							CHAR strData[256];
							sprintf(strData, "Bug in animation.cfg: Start Frame %d + Number of Frames %d is not the same number as the next Frame %d",
								iPreviousFirstFrame, iPreviousNumFrames, iFirstFrame);
							LOG(std::string(strData), Logger::LOG_DATA);

							//							iFirstFrame = iPreviousFirstFrame + iPreviousNumFrames;
						}
						iPreviousFirstFrame = playerAnim[a].iFirstFrame = iFirstFrame;
						iPreviousNumFrames = playerAnim[a].iNumFrames = iNumFrames;

						a++;
					}
				}
			}
		}
	}

	// find the offset
	int	 iOffset = playerAnim[LEGS_WALKCR].iFirstFrame - playerAnim[TORSO_GESTURE].iFirstFrame;

	// remove it from the lower animations
	for (a = LEGS_WALKCR; a < MAX_ANIMATIONS; a++)
		playerAnim[a].iFirstFrame -= iOffset;

	for (a = 0; a < MAX_ANIMATIONS; a++)
		if (playerAnim[a].iNumFrames > 0) playerAnim[a].iNumFrames -= 1;

	fclose(AnimFile);
}

#ifdef _DEBUG

//-------------------------------------------------------------------
// DumpAnimInfo
//
// Desc: Dump all Anim info
//-------------------------------------------------------------------
void Q3Player::DumpAnimInfo()
{
	LOGFUNC("DumpAnimInfo()");

	//
	// write into logfile
	//
	LOG("First Frame - Number of Frames - Looping Frame - Frames per Second ----------", Logger::LOG_DATA);

	for (int i = 0; i < MAX_ANIMATIONS; i++)
	{
		CHAR strData[256];
		sprintf(strData, "%3d;               %3d;               %3d;               %3d",
			playerAnim[i].iFirstFrame, playerAnim[i].iNumFrames + 1, playerAnim[i].iLoopingFrame, playerAnim[i].iFramesPerSecond);
		LOG(std::string(strData), Logger::LOG_DATA);
	}
}

#endif
//-----------------------------------------------------------------
// SetLowerAnim
//
// Desc: Set current animation for the lower part of the model
//-------------------------------------------------------------------
void Q3Player::SetLowerAnim(int iAnimNumber)
{
	playerAnimLower = iAnimNumber;

	if (!md3AnimationIsValid(iAnimNumber))
		return;

	if (!md3AnimationIsDeath(iAnimNumber) && !md3AnimationIsLower(iAnimNumber))
		return;

	md3Lower.iFps = playerAnim[iAnimNumber].iFramesPerSecond;
	md3Lower.iNextFrame = playerAnim[iAnimNumber].iFirstFrame;
	md3Lower.iStartFrame = playerAnim[iAnimNumber].iFirstFrame;
	md3Lower.iEndFrame = playerAnim[iAnimNumber].iFirstFrame + playerAnim[iAnimNumber].iNumFrames;

	// If a death animation, upper animation must match
	if (md3AnimationIsDeath(iAnimNumber) && playerAnimUpper != iAnimNumber)
		SetUpperAnim(iAnimNumber);

	// If we were in a death animation and we changed, set the upper to idle
	if (!md3AnimationIsDeath(iAnimNumber) && md3AnimationIsDeath(playerAnimUpper))
		SetUpperAnim(TORSO_STAND);
}

//-----------------------------------------------------------------
// SetUpperAnim
//
// Desc: Set current animation for the upper part of the model
//-------------------------------------------------------------------
void Q3Player::SetUpperAnim(int iAnimNumber)
{

	playerAnimUpper = iAnimNumber;

	if (!md3AnimationIsValid(iAnimNumber))
		return;

	if (!md3AnimationIsDeath(iAnimNumber) && !md3AnimationIsUpper(iAnimNumber))
		return;

	// set animation
	md3Upper.iFps = playerAnim[iAnimNumber].iFramesPerSecond;
	md3Upper.iNextFrame = playerAnim[iAnimNumber].iFirstFrame;
	md3Upper.iStartFrame = playerAnim[iAnimNumber].iFirstFrame;
	md3Upper.iEndFrame = playerAnim[iAnimNumber].iFirstFrame + playerAnim[iAnimNumber].iNumFrames;

	// If a death animation, lower animation must match
	if (md3AnimationIsDeath(iAnimNumber) && playerAnimLower != iAnimNumber)
		SetLowerAnim(iAnimNumber);

	// If we were in a death animation and we changed, set the lower to idle
	if (!md3AnimationIsDeath(iAnimNumber) && md3AnimationIsDeath(playerAnimLower))
		SetLowerAnim(LEGS_IDLE);
}


//-----------------------------------------------------------
// LoadPlayerShaderProfile
//
// Desc: Load *.profile for Player
//----------------------------------------------------------
void Q3Player::LoadPlayerShaderProfile(const char* pcFileName)
{
	LOGFUNC("LoadPlayerShaderProfile()");

	FILE* ShaderProfileFile;
	CHAR	cMeshName[16];
	CHAR    cShaderProfile[32];
	CHAR    strToken[1024];
	CHAR	pcTextLine[1024];
	BOOL	bMesh, bTag, bShader, bShaderLevel, bShaderProfile;
	int iShaderLevel, iShaderProfile;

	//
	// open *.profile file
	//
	if (CheckFile(pcFileName) == 0)
		return;

	ShaderProfileFile = fopen(pcFileName, "rt");

	while (!feof(ShaderProfileFile))
	{
		bMesh = FALSE;
		bTag = FALSE;
		bShader = FALSE;
		bShaderLevel = FALSE;
		bShaderProfile = FALSE;

		// scans a whole text line
		fscanf(ShaderProfileFile, "%s", pcTextLine);

		while (pcTextLine && pcTextLine[0])
		{
			// returns in strToken a word of a textline
			// returns in pcTextLine the next word of this textline
			// Reads only words consisting of chars, numbers, points and underscores
			ParseTextLine(pcTextLine, strToken);

			// read out the # of the shaderprofile and put it into
			// iShaderProfile
			if (strstr(_strlwr(strToken), "shaderprofile"))
			{
				strcpy(cShaderProfile, strToken);
				iShaderProfile = ParseNumber(strToken);
				bShaderProfile = 1;
			}

			// get mesh name
			else if (bMesh == 0 && bShaderLevel == 0 && bShaderProfile == 0)
			{
				strcpy(cMeshName, strToken);
				bMesh = 1;
			}

			// get shaderlevel
			else if ((strstr(strToken, "shaderlevel")) && bShaderLevel == 0 && bMesh == 1 && bShaderProfile == 0)
			{
				iShaderLevel = ParseNumber(strToken);
				bShaderLevel = 1;
			}

			//
			// write into log file
			//
			if (bShaderProfile == 1)
			{
				CHAR strData[64];
				sprintf(strData, "Load Shaderprofile #%d",
					iShaderProfile);
				LOG(std::string(strData), Logger::LOG_DATA);
			}

			if (bShaderLevel)
			{
				for (int i = 0; i < md3Upper.modelHeader.iMeshNum; i++)
				{
					if (strstr(md3Upper.md3Meshes[i].meshHeader.cName, cMeshName))
					{
						md3Upper.md3Meshes[i].iChoosedShaderLevel[iShaderProfile] = iShaderLevel;
						CHAR strData[64];
						sprintf(strData, "... for Mesh %s with Shader Level #%d", cMeshName, iShaderLevel);
						LOG(std::string(strData), Logger::LOG_DATA);
						break;
					}
				}

				for (int i = 0; i < md3Lower.modelHeader.iMeshNum; i++)
				{
					if (strstr(md3Lower.md3Meshes[i].meshHeader.cName, cMeshName))
					{
						md3Lower.md3Meshes[i].iChoosedShaderLevel[iShaderProfile] = iShaderLevel;
						CHAR strData[64];
						sprintf(strData, "... for Mesh %s with Shader Level #%d", cMeshName, iShaderLevel);
						LOG(std::string(strData), Logger::LOG_DATA);
						break;
					}
				}

				for (int i = 0; i < md3Head.modelHeader.iMeshNum; i++)
				{
					if (strstr(md3Head.md3Meshes[i].meshHeader.cName, cMeshName))
					{
						md3Head.md3Meshes[i].iChoosedShaderLevel[iShaderProfile] = iShaderLevel;
						CHAR strData[64];
						sprintf(strData, "... for Mesh %s with Shader Level #%d", cMeshName, iShaderLevel);
						LOG(std::string(strData), Logger::LOG_DATA);
						break;
					}
				}

			}
		}
	}
	fclose(ShaderProfileFile);
}

//-----------------------------------------------------------
// LoadWeaponShaderProfile
//
// Desc: Load *.profile for weapon
//----------------------------------------------------------
void Q3Player::LoadWeaponShaderProfile(char* pcFileName)
{
	LOGFUNC("LoadWeaponShaderProfile()");

	FILE* ShaderProfileFile;
	CHAR	cMeshName[16];
	CHAR    cShaderProfile[32];
	CHAR    strToken[1024];
	CHAR	pcTextLine[1024];
	BOOL	bMesh, bTag, bShader, bShaderLevel, bShaderProfile;
	int iShaderLevel, iShaderProfile;

	//
	// open *.profile file
	//
	if (CheckFile(pcFileName) == 0)
		return;

	ShaderProfileFile = fopen(pcFileName, "rt");

	while (!feof(ShaderProfileFile))
	{
		bMesh = FALSE;
		bTag = FALSE;
		bShader = FALSE;
		bShaderLevel = FALSE;
		bShaderProfile = FALSE;

		// scans a whole text line
		fscanf(ShaderProfileFile, "%s", pcTextLine);

		while (pcTextLine && pcTextLine[0])
		{
			// returns in strToken a word of a textline
			// returns in pcTextLine the next word of this textline
			// Reads only words consisting of chars, numbers, points and underscores
			ParseTextLine(pcTextLine, strToken);

			// read out the # of the shaderprofile and put it into
			// iShaderProfile
			if (strstr(_strlwr(strToken), "shaderprofile"))
			{
				strcpy(cShaderProfile, strToken);
				iShaderProfile = ParseNumber(strToken);
				bShaderProfile = 1;
			}

			// get mesh name
			else if (bMesh == 0 && bShaderLevel == 0 && bShaderProfile == 0)
			{
				strcpy(cMeshName, strToken);
				bMesh = 1;
			}

			// get shaderlevel
			else if ((strstr(strToken, "shaderlevel")) && bShaderLevel == 0 && bMesh == 1 && bShaderProfile == 0)
			{
				iShaderLevel = ParseNumber(strToken);
				bShaderLevel = 1;
			}

			//
			// write into log file
			//		
			if (bShaderProfile == 1)
			{
				CHAR strData[64];
				sprintf(strData, "Load Shaderprofile #%d", iShaderProfile);
				LOG(std::string(strData), Logger::LOG_DATA);
			}

			if (bShaderLevel)
			{
				for (int i = 0; i < md3Weapon.modelHeader.iMeshNum; i++)
				{
					if (strstr(md3Weapon.md3Meshes[i].meshHeader.cName, cMeshName))
					{
						md3Weapon.md3Meshes[i].iChoosedShaderLevel[iShaderProfile] = iShaderLevel;
						CHAR strData[64];
						sprintf(strData, "... for Mesh %s with Shader Level #%d", cMeshName, iShaderLevel);
						LOG(std::string(strData), Logger::LOG_DATA);
						break;
					}
				}
			}
		}
	}
	fclose(ShaderProfileFile);
}
//-------------------------------------------------------------------
// LoadPlayerGeometry
//
// Desc: Load Q3Player (md3 files, animation file)
//-------------------------------------------------------------------
void Q3Player::LoadPlayerGeometry(const char* cPath)
{
	char cLowerModelFilePath[1024], cUpperModelFilePath[1024], cHeadModelFilePath[1024];
	char cAnimFilePath[1024];

	sprintf(cLowerModelFilePath, "%s\\lower.md3", cPath);
	sprintf(cUpperModelFilePath, "%s\\upper.md3", cPath);
	sprintf(cHeadModelFilePath, "%s\\head.md3", cPath);

	sprintf(cAnimFilePath, "%s\\animation.cfg", cPath);

	md3Lower.LoadModelGeometry(cLowerModelFilePath);
	md3Upper.LoadModelGeometry(cUpperModelFilePath);
	md3Head.LoadModelGeometry(cHeadModelFilePath);

	LoadAnim(cAnimFilePath);

	md3Lower.iStartFrame = 0; md3Lower.iEndFrame = 0;
	md3Upper.iStartFrame = 0; md3Upper.iEndFrame = 0;
	md3Head.iStartFrame = 0; md3Head.iEndFrame = 0;

	md3Lower.LinkModel("tag_torso", &md3Upper);
	md3Upper.LinkModel("tag_head", &md3Head);

#ifdef _DEBUG
	md3Lower.DumpGeometryInfo();
	md3Upper.DumpGeometryInfo();
	md3Head.DumpGeometryInfo();
	DumpAnimInfo();
#endif
}

//-------------------------------------------------------------------
// LoadWeaponGeometry
//
// Desc: Load weapon 
//-------------------------------------------------------------------
void Q3Player::LoadWeaponGeometry(const char* cPath, const char* cName)
{
	char cWeaponModelFilePath[1024];

	sprintf(cWeaponModelFilePath, "%s\\%s.md3", cPath, cName);

	md3Weapon.LoadModelGeometry(cWeaponModelFilePath);

#ifdef _DEBUG
	md3Weapon.DumpGeometryInfo();
#endif
}


//-------------------------------------------------------------------------------
// LoadSkins
//
// Desc: load skins for the player
//-------------------------------------------------------------------------------
void Q3Player::LoadSkins(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cSkin)
{
	char cLowerSkinFilePath[1024], cUpperSkinFilePath[1024], cHeadSkinFilePath[1024];

	sprintf(cLowerSkinFilePath, "%s\\lower_%s.skin", cPath, cSkin);
	sprintf(cUpperSkinFilePath, "%s\\upper_%s.skin", cPath, cSkin);
	sprintf(cHeadSkinFilePath, "%s\\head_%s.skin", cPath, cSkin);

	md3Lower.LoadSkins(m_pd3dDevice, cLowerSkinFilePath, cPath);
	md3Upper.LoadSkins(m_pd3dDevice, cUpperSkinFilePath, cPath);
	md3Head.LoadSkins(m_pd3dDevice, cHeadSkinFilePath, cPath);

#ifdef _DEBUG
	md3Lower.DumpSkinInfo();
	md3Upper.DumpSkinInfo();
	md3Head.DumpSkinInfo();
#endif
}

//-------------------------------------------------------------------------------
// LoadWeaponSkins
//
// Desc: load skins for the player
//-------------------------------------------------------------------------------
void Q3Player::LoadWeaponSkins(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cSkin)
{
	char cWeaponSkinFilePath[1024];

	sprintf(cWeaponSkinFilePath, "%s\\weapon_%s.skin", cPath, cSkin);
	md3Weapon.LoadSkins(m_pd3dDevice, cWeaponSkinFilePath, cPath);
}

//-------------------------------------------------------------------------------
// LoadWeaponShader
//
// Desc: load shader for the weapon
//-------------------------------------------------------------------------------
void Q3Player::LoadWeaponShaders(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cName, const char* cSkin, const char* shaderRootPath)
{
	char cWeaponShaderFilePath[1024];
	char cShaderProfilePath[1024];

	sprintf(cShaderProfilePath, "%s\\%s_shader.profiles", cPath, cName);

	sprintf(cWeaponShaderFilePath, "%s\\weapon_%s.sha", cPath, cSkin);
	md3Weapon.LoadShaders(m_pd3dDevice, cWeaponShaderFilePath, shaderRootPath);

	LoadWeaponShaderProfile(cShaderProfilePath);

#ifdef _DEBUG
	md3Weapon.DumpShaderInfo();
#endif
}


//-------------------------------------------------------------------------------
// LoadPlayerShaders
//
// Desc: load shaders for the player
//-------------------------------------------------------------------------------
void Q3Player::LoadPlayerShaders(IDirect3DDevice9* m_pd3dDevice, const char* cPath, const char* cName, const char* cSkin, const char* shaderRootPath)
{
	char cLowerShaderFilePath[1024], cUpperShaderFilePath[1024], cHeadShaderFilePath[1024];
	char cShaderProfilePath[1024];

	sprintf(cShaderProfilePath, "%s\\%s_shader.profiles", cPath, cName);

	sprintf(cLowerShaderFilePath, "%s\\lower_%s.sha", cPath, cSkin);
	sprintf(cUpperShaderFilePath, "%s\\upper_%s.sha", cPath, cSkin);
	sprintf(cHeadShaderFilePath, "%s\\head_%s.sha", cPath, cSkin);

	md3Lower.LoadShaders(m_pd3dDevice, cLowerShaderFilePath, shaderRootPath);
	md3Upper.LoadShaders(m_pd3dDevice, cUpperShaderFilePath, shaderRootPath);
	md3Head.LoadShaders(m_pd3dDevice, cHeadShaderFilePath, shaderRootPath);

	LoadPlayerShaderProfile(cShaderProfilePath);
	D3DVERTEXELEMENT9 decl[] =
	{
		// stream, offset, type, method, semantic type (for example normal), ?
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
		D3DDECL_END()
	};

	m_pd3dDevice->CreateVertexDeclaration(decl, &m_pVertexDeclaration);

#ifdef _DEBUG
	md3Lower.DumpShaderInfo();
	md3Upper.DumpShaderInfo();
	md3Head.DumpShaderInfo();
#endif

}

//---------------------------------------------------------
// FreeShaders
//
// Desc: frees memory used by the textures
//----------------------------------------------------------
void Q3Player::FreePlayerShaders(IDirect3DDevice9* m_pd3dDevice)
{
	md3Lower.DeleteShaders(m_pd3dDevice);
	md3Upper.DeleteShaders(m_pd3dDevice);
	md3Head.DeleteShaders(m_pd3dDevice);

	SAFE_RELEASE(m_pVertexDeclaration);
}

//---------------------------------------------------------
// FreeSkins
//
// Desc: frees memory used by the textures
//----------------------------------------------------------
void Q3Player::FreeSkins()
{
	md3Lower.DeleteSkins();
	md3Upper.DeleteSkins();
	md3Head.DeleteSkins();
}

//---------------------------------------------------------
// FreeWeaponSkins
//
// Desc: frees memory used by the textures
//----------------------------------------------------------
void Q3Player::FreeWeaponSkins()
{
	md3Weapon.DeleteSkins();
}

//---------------------------------------------------------
// FreeWeaponShader
//
// Desc: frees memory used by the shaders
//----------------------------------------------------------
void Q3Player::FreeWeaponShaders(IDirect3DDevice9* m_pd3dDevice)
{
	md3Weapon.DeleteShaders(m_pd3dDevice);
}



//-----------------------------------------------------------
// DeletePlayerGeometry
//
// Desc: frees memory used by the player geometry
//-----------------------------------------------------------	
void Q3Player::DeletePlayerGeometry()
{
	md3Lower.DeleteModelGeometry();
	md3Upper.DeleteModelGeometry();
	md3Head.DeleteModelGeometry();
}

//-----------------------------------------------------------
// DeleteWeaponGeometry
//
// Desc: frees memory used by the player geometry
//-----------------------------------------------------------	
void Q3Player::DeleteWeaponGeometry()
{
	md3Weapon.DeleteModelGeometry();
}


//-----------------------------------------------------------------------------
// Draw
//
// Desc: Draw all models for curent Q3Player
//
//------------------------------------------------------------------------------
void Q3Player::Draw(IDirect3DDevice9* m_pDevice)
{
	md3Lower.DrawSkeleton(
		&md3Lower,
		m_pDevice,
		m_pIB,
		m_pVB,
		&matViewProj,
		&matWorld,
		&matLightViewProj,
		&matScaleBias,
		m_pVertexDeclaration);

	m_pDevice->SetVertexShader(NULL);
	m_pDevice->SetPixelShader(NULL);
}

//---------------------------------------------------------------------------------
// Update
//
// Desc: Update time for all models for current Q3Player
//---------------------------------------------------------------------------------
void Q3Player::Update(float time)
{
	md3Lower.UpdateFrameTime(time);
	md3Upper.UpdateFrameTime(time);
	md3Head.UpdateFrameTime(time);
}

//---------------------------------------------------------------------------------
// CreateVertexBuffer
//
// Desc: creates the vertex buffer, that will be filled 
//		 later by LoadPlayerGeometry()
//---------------------------------------------------------------------------------
HRESULT Q3Player::CreateVertexNIndexBuffer(IDirect3DDevice9* m_pd3dDevice)
{
	LOGFUNC("CreateVertexNIndexBuffer()");

	HRESULT hr;

	const int iNumVertices = 4096;

	hr = m_pd3dDevice->CreateVertexBuffer(iNumVertices * sizeof(MD3VERTEXBUFFERSTRUCT),
		D3DUSAGE_NPATCHES | D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_DEFAULT, &m_pVB, NULL);
	CHAR strData[10];
	_itoa(iNumVertices, strData, 10);

	if (FAILED(hr))
	{
		LOG("Vertex Buffer for " + std::string(strData) + " Vertices failed", Logger::LOG_ERR);
		OutputDebugString(L"Failed to create Vertex Buffer\n");
		OutputDebugString(L"\n");
		return hr;
	}
	else
		LOG("Created Vertex Buffer for " + std::string(strData) + " Vertices", Logger::LOG_DATA);

	const int iNumIndices = 8192;

	// create index buffer
	hr = m_pd3dDevice->CreateIndexBuffer(iNumIndices,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &m_pIB, NULL);

	_itoa(iNumIndices, strData, 10);

	if (FAILED(hr))
	{
		LOG("Index Buffer for " + std::string(strData) + " Indices failed", Logger::LOG_ERR);
		OutputDebugString(L"Failed to create Vertex Buffer\n");
		OutputDebugString(L"\n");
		return hr;
	}
	else
		LOG("Created Index Buffer for " + std::string(strData) + " Indices", Logger::LOG_DATA);

	return S_OK;
}

//---------------------------------------------------------------------------------
// DeleteVertexNIndexBuffer
//
// Desc: deletes the vertex buffer
//---------------------------------------------------------------------------------
void Q3Player::DeleteVertexNIndexBuffer()
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
}
