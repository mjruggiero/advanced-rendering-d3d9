//-----------------------------------------------------------------------------
// File:	MD3.h
//
// Desc:	loads and renders *.md3 files
//
// Last modification: March 31th, 2003
//
// Credits: 
// I would like to thank the following people for providing source 
// or information, that helped me putting together this program:
// - The people behind the mentalvortex page that 
// 	 described the format the first time
// - Lonerunner for his md3 viewer at 
//		http://lonerunner.cfxweb.net/
// - Bart Sekura for his Direct3D md3 viewer 
// - Nate Miller for his md3 viewer
// - Wrath for his explanations of the md3 format at
//		http://www.planetquake.com/polycount/cottages/wrath/
//
// Copyright (c) 2001 - 2003 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------
#pragma once

#include "ShaderProfileLimits.h"

#include <D3DX9.h>
#include "D3D9.h"

#define MAXTEXTURESPERMESH 6
#define MAXSHADERLEVELINMESH 12

extern int iShaderProfile;


//------------------------------------------------------------
// the length of MD3HEADER, MD3BONEFRAME, MD3TAG, MD3MESHFILE 
// and MD3VERTEX is fixed in the md3 file
//------------------------------------------------------------
typedef struct
{
	char id[4];			// id of file, always "IDP3"
	int iVersion;		// version number
	char cFileName[68];
	int iBoneFrameNum;	// Number of animation key frames in the whole model=every mesh
	int iTagNum;		// Number of tags
	int iMeshNum;		// Number of meshes
	int iMaxTextureNum;	// maximum number of unique textures used in a md3 model
	int iHeaderSize;	// size of header
	int iTagStart;		// starting position of tag frame structures
	int iMeshStart;		// starting position of mesh structures
	int iFileSize;
} MD3HEADER;

typedef struct
{
	float	mins[3];		// first corner of the bounding box
	float	maxs[3];		// second corner of the bounding box 
	float	position[3];	// position/origin of the bounding box 
	float	scale;			// radius of bounding sphere
	char	name[16];		// name of frame ASCII character string, NULL terminated
} MD3BONEFRAME;

typedef struct
{
	char	name[64];
	float	position[3];	// position of tag relative to the model that contains the tag
	float	rotation[3][3];	// the direction the tag is facing
} MD3TAG;

// storage for Direct3D specific tag informations
typedef struct
{
	char	name[64];
	D3DXMATRIX matTag;
} D3DTAG;

typedef struct
{
	char cId[4];
	char cName[68];
	int iMeshFrameNum;  // number of frames in mesh
	int iTextureNum;	// number of textures=skins in this mesh
	int iVertexNum;		// number of vertices in this mesh
	int iTriangleNum;	// number of triangles
	int iTriangleStart;	// starting position of triangle data, relative to the start of MD3Mesh
	int iHeaderSize;	// Headersize = starting position of texture data
	int iTecVecStart;	// starting position of the texture vector data
	int iVertexStart;	// starting position of the vertex data
	int iMeshSize;
} MD3MESHHEADER;

typedef struct
{
	char	name[68];
} MD3SKIN;

typedef struct
{
	int		index[3];
} MD3TRIANGLE;

typedef struct
{
	float	texvec[2];
} MD3TEXCOORD;

typedef struct
{
	signed	 short	sVector[3];
	unsigned char	cNormal[2];
} MD3VERTEX;

// extended version of MD3VERTEX
typedef struct
{
	D3DXVECTOR3		vVector;
	D3DXVECTOR3		vVertexNormal;
} MD3VERTEXEX;

typedef struct
{
	D3DXVECTOR3 vTangent;
	D3DXVECTOR3 vBinormal;
}TANGENTS;

typedef struct
{
	MD3MESHHEADER		meshHeader;
	MD3SKIN* meshSkins;
	MD3TRIANGLE* meshTriangles;
	MD3TEXCOORD* meshTexCoord;
	MD3VERTEXEX* meshVertices;
	TANGENTS* meshTangents;
	int					iNumTextures;
	IDirect3DTexture9* pTexturesInterfaces[MAXTEXTURESPERMESH];
	IDirect3DVertexShader9* pVertexShader[MAXSHADERLEVELINMESH];
	IDirect3DPixelShader9* pPixelShader[MAXSHADERLEVELINMESH];
	int					iChoosedShaderLevel[MAXSHADERPROFILE];
} MD3MESH;

typedef struct
{
	int		iFirstFrame, iNumFrames, iLoopingFrame, iFramesPerSecond;
} MD3ANIM;


struct MD3VERTEXBUFFERSTRUCT
{
	D3DXVECTOR3 vPosition;
	D3DXVECTOR3 vVertexNormal;
	D3DXVECTOR2 vTexCoord;
	D3DXVECTOR3 vTangent;
};


typedef enum {
	BOTH_DEATH1,
	BOTH_DEAD1,
	BOTH_DEATH2,
	BOTH_DEAD2,
	BOTH_DEATH3,
	BOTH_DEAD3,

	TORSO_GESTURE,

	TORSO_ATTACK,
	TORSO_ATTACK2,

	TORSO_DROP,
	TORSO_RAISE,

	TORSO_STAND,
	TORSO_STAND2,

	LEGS_WALKCR,
	LEGS_WALK,
	LEGS_RUN,
	LEGS_BACK,
	LEGS_SWIM,

	LEGS_JUMP,
	LEGS_LAND,

	LEGS_JUMPB,
	LEGS_LANDB,

	LEGS_IDLE,
	LEGS_IDLECR,

	LEGS_TURN,

	MAX_ANIMATIONS
} ANIMNUMBER;


// Useful functions
static inline bool md3AnimationIsValid(INT anim)
{
	return (anim >= BOTH_DEATH1 && anim < MAX_ANIMATIONS);
}

static inline bool md3AnimationIsDeath(INT anim)
{
	return (anim >= BOTH_DEATH1 && anim <= BOTH_DEAD3);
}

static inline bool md3AnimationIsLower(INT anim)
{
	return (anim >= LEGS_WALKCR && anim <= LEGS_TURN);
}

static inline bool md3AnimationIsUpper(INT anim)
{
	return (anim >= TORSO_GESTURE && anim <= TORSO_STAND2);
}
