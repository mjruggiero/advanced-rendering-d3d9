//-----------------------------------------------------------------------------
// File:	MD3.cpp
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
#include "D3D9Compat.h"
#include "MD3Model.h"
#include "MD3ShaderLoader.h"
#include "MD3SkinLoader.h"

#include "logger.h"
#include "Utility.h"

int iShaderProfile = 2;

//---------------------------------------------------------------
// MD3Model
// 
// Constructor
//---------------------------------------------------------------
MD3Model::MD3Model()
{
	iFrame = 0;
	iNextFrame = 0;
	iFps = 0;
	iStartFrame = 0;
	iEndFrame = 0;
	fOldTime = 0;
	fNewTime = 0;

	cModelName[0] = '\0';
	md3BoneFrames = nullptr;
	d3dTag = nullptr;
	md3Links = nullptr;
	md3Meshes = nullptr;
	modelHeader = {};

	for (int i = 0; i < MAXTEXTURESPERMESH; ++i)
		pMiniTextureCache[i] = nullptr;
}

//---------------------------------------------------------------
// MD3Model
// 
// Destructor
//---------------------------------------------------------------
MD3Model::~MD3Model()
{
}

//-----------------------------------------------------------------
// DeleteModelGeometry()
//
// Desc: delete the geometry of the model
//-----------------------------------------------------------------
void MD3Model::DeleteModelGeometry()
{
	if (md3BoneFrames)
	{
		free(md3BoneFrames);
		md3BoneFrames = nullptr;
	}

	if (d3dTag)
	{
		free(d3dTag);
		d3dTag = nullptr;
	}

	if (md3Links)
	{
		free(md3Links);
		md3Links = nullptr;
	}

	if (md3Meshes)
	{
		for (int i = 0; i < modelHeader.iMeshNum; i++)
		{
			free(md3Meshes[i].meshSkins);
			free(md3Meshes[i].meshTriangles);
			free(md3Meshes[i].meshTexCoord);
			free(md3Meshes[i].meshVertices);
			free(md3Meshes[i].meshTangents);
		}

		free(md3Meshes);
		md3Meshes = nullptr;
	}

	modelHeader = {};
}

//-------------------------------------------------------------------
// LoadModel
//
// Desc: return:
//  1 : all is Ok
// -1 : can't find file
// -2 : bad header for md3 file
//--------------------------------------------------------------------
int MD3Model::LoadModelGeometry(const char* filename)
{
	FILE* md3file;
	int				i, j;

	// check if file exist
	if (CheckFile(filename) == 0)
		return -1;

	// open file
	md3file = fopen(filename, "rb");

	// copy name
	strcpy(cModelName, filename);

	//
	//	read header
	//
	fread(&modelHeader, 1, sizeof(MD3HEADER), md3file);

	static char ver[4];
	sprintf(ver, "%c%c%c%c", modelHeader.id[0], modelHeader.id[1], modelHeader.id[2], modelHeader.id[3]);

	if (strcmp(ver, "IDP3") || modelHeader.iVersion != 15)
	{
		fclose(md3file);
		return -2;
	}

	//
	//	read boneframes
	//
	md3BoneFrames = (MD3BONEFRAME*)malloc(sizeof(MD3BONEFRAME) * modelHeader.iBoneFrameNum);
	fread(md3BoneFrames, sizeof(MD3BONEFRAME), modelHeader.iBoneFrameNum, md3file);

	//
	//	read tags
	//
	MD3TAG* tempTag;
	tempTag = (MD3TAG*)malloc(sizeof(MD3TAG) * modelHeader.iBoneFrameNum * modelHeader.iTagNum);
	d3dTag = (D3DTAG*)malloc(sizeof(D3DTAG) * modelHeader.iBoneFrameNum * modelHeader.iTagNum);
	fread(tempTag, sizeof(MD3TAG), modelHeader.iBoneFrameNum * modelHeader.iTagNum, md3file);

	for (i = 0; i < modelHeader.iBoneFrameNum * modelHeader.iTagNum; i++)
	{
		strcpy(d3dTag[i].name, tempTag[i].name);
		d3dTag[i].matTag(0, 0) = tempTag[i].rotation[0][0];
		d3dTag[i].matTag(0, 1) = tempTag[i].rotation[0][1];
		d3dTag[i].matTag(0, 2) = tempTag[i].rotation[0][2];
		d3dTag[i].matTag(0, 3) = 0.0f;
		d3dTag[i].matTag(1, 0) = tempTag[i].rotation[1][0];
		d3dTag[i].matTag(1, 1) = tempTag[i].rotation[1][1];
		d3dTag[i].matTag(1, 2) = tempTag[i].rotation[1][2];
		d3dTag[i].matTag(1, 3) = 0.0f;
		d3dTag[i].matTag(2, 0) = tempTag[i].rotation[2][0];
		d3dTag[i].matTag(2, 1) = tempTag[i].rotation[2][1];
		d3dTag[i].matTag(2, 2) = tempTag[i].rotation[2][2];
		d3dTag[i].matTag(2, 3) = 0.0f;
		d3dTag[i].matTag(3, 0) = tempTag[i].position[0];
		d3dTag[i].matTag(3, 1) = tempTag[i].position[1];
		d3dTag[i].matTag(3, 2) = tempTag[i].position[2];
		d3dTag[i].matTag(3, 3) = 1.0f;
	}
	free(tempTag);

	//
	//	init links
	//
	md3Links = (MD3Model**)malloc(sizeof(MD3Model*) * modelHeader.iTagNum);

	for (i = 0; i < modelHeader.iTagNum; i++)
		md3Links[i] = NULL;

	//
	//	read meshes
	//
	md3Meshes = (MD3MESH*)malloc(sizeof(MD3MESH) * modelHeader.iMeshNum);

	LONG lMeshOffset = ftell(md3file);

	for (i = 0; i < modelHeader.iMeshNum; i++)
	{
		fseek(md3file, lMeshOffset, SEEK_SET);
		fread(&md3Meshes[i].meshHeader, sizeof(MD3MESHHEADER), 1, md3file);

		// ------------------------------------------
		md3Meshes[i].meshSkins = (MD3SKIN*)malloc(sizeof(MD3SKIN) * md3Meshes[i].meshHeader.iTextureNum);
		fread(md3Meshes[i].meshSkins, sizeof(MD3SKIN), md3Meshes[i].meshHeader.iTextureNum, md3file);

		// ------------------------------------------
		fseek(md3file, lMeshOffset + md3Meshes[i].meshHeader.iTriangleStart, SEEK_SET);
		md3Meshes[i].meshTriangles = (MD3TRIANGLE*)malloc(sizeof(MD3TRIANGLE) * md3Meshes[i].meshHeader.iTriangleNum);
		fread(md3Meshes[i].meshTriangles, sizeof(MD3TRIANGLE), md3Meshes[i].meshHeader.iTriangleNum, md3file);

		// ------------------------------------------			
		fseek(md3file, lMeshOffset + md3Meshes[i].meshHeader.iTecVecStart, SEEK_SET);
		md3Meshes[i].meshTexCoord = (MD3TEXCOORD*)malloc(sizeof(MD3TEXCOORD) * md3Meshes[i].meshHeader.iVertexNum);
		fread(md3Meshes[i].meshTexCoord, sizeof(MD3TEXCOORD), md3Meshes[i].meshHeader.iVertexNum, md3file);

		// ------------------------------------------			
		fseek(md3file, lMeshOffset + md3Meshes[i].meshHeader.iVertexStart, SEEK_SET);
		md3Meshes[i].meshVertices = (MD3VERTEXEX*)malloc(sizeof(MD3VERTEXEX) * md3Meshes[i].meshHeader.iVertexNum * md3Meshes[i].meshHeader.iMeshFrameNum);

		MD3VERTEX* meshTempVertices;
		meshTempVertices = (MD3VERTEX*)malloc(sizeof(MD3VERTEX) * md3Meshes[i].meshHeader.iVertexNum * md3Meshes[i].meshHeader.iMeshFrameNum);
		fread(meshTempVertices, sizeof(MD3VERTEX), md3Meshes[i].meshHeader.iMeshFrameNum * md3Meshes[i].meshHeader.iVertexNum, md3file);

		for (j = 0; j < md3Meshes[i].meshHeader.iVertexNum * md3Meshes[i].meshHeader.iMeshFrameNum; j++)
		{
			// The /64 is a constanct scale factor that is used to convert the md3
			// model verts from shorts to floats.
			md3Meshes[i].meshVertices[j].vVector[0] = (float)meshTempVertices[j].sVector[0] / 64;
			md3Meshes[i].meshVertices[j].vVector[1] = (float)meshTempVertices[j].sVector[1] / 64;
			md3Meshes[i].meshVertices[j].vVector[2] = (float)meshTempVertices[j].sVector[2] / 64;

			/*
			The normal information in the md3 file is stored as two unsigned chars to save space.
			These chars contain the spherical coordinates of the normal.

			The first byte is longitude and the second byte is latitude. You may find
			this information in mathlib.c in the NormalToLatLong() function
			in the Quake3 tools source code.
			*/
			//			md3Meshes[ i ].meshVertices [ j ].vNormal = fCalcNormals[meshTempVertices[j].cNormal[0]] [meshTempVertices[j].cNormal[1]];
		}

		//-------------------------------------------------------------------
		//
		// Calculate Normals and Tangents
		// store them in files for faster startup
		//
		//-------------------------------------------------------------------
		FILE* File;
		DWORD			dwNumberOfVertices;
		CHAR			cMeshName[68];

		dwNumberOfVertices = md3Meshes[i].meshHeader.iMeshFrameNum * md3Meshes[i].meshHeader.iVertexNum;

		// allocate memory for tangents
		md3Meshes[i].meshTangents = (TANGENTS*)malloc(sizeof(TANGENTS) * dwNumberOfVertices);

		// Calculate Normals
		// store the normals in a file with the extension *.nor
		// read normals from this file to speed things up
		int z = 0;

		// read in for example hunter\ when cModelName stores hunter\head.md3
		while (cModelName[z] != '\0')
		{
			cMeshName[z] = cModelName[z];

			if (cModelName[z] == '\\')
			{
				++z;
				break;
			}

			++z;
		}

		cMeshName[z] = '\0';

		// adds to hunter\ for example legs_l: hunter\legs_l
		for (unsigned int y = 0; y < strlen(md3Meshes[i].meshHeader.cName); y++)
			cMeshName[z++] = md3Meshes[i].meshHeader.cName[y];

		// saves mesh name in an extra array and adds the terminating \0  
		char cNormalName[68];
		memcpy(cNormalName, cMeshName, sizeof(cNormalName));
		cNormalName[z] = '\0';

		// add the appendix .n
		strcat(cNormalName, ".n");

		// check if file exist
		if (CheckFile(cNormalName) == 0)
		{
			CalculateNormals2(&md3Meshes[i]);

			// open file
			File = fopen(cNormalName, "w+b");

			for (unsigned int y = 0; y < dwNumberOfVertices; y++)
				fwrite(&md3Meshes[i].meshVertices[y].vVertexNormal, sizeof(D3DXVECTOR3), 1, File);

			// close file
			fclose(File);
		}
		else
		{
			// open file
			File = fopen(cNormalName, "rb");

			for (unsigned int y = 0; y < dwNumberOfVertices; y++)
				fread(&md3Meshes[i].meshVertices[y].vVertexNormal, sizeof(D3DXVECTOR3), 1, File);

			// close file
			fclose(File);
		}

		// Calculate tangent
		// store the tangent in a file with the extension *.tan
		// read tangents from this file to speed things up
		// check if file exist
		char cTangentName[68];
		memcpy(cTangentName, cMeshName, sizeof(cTangentName));
		cTangentName[z] = '\0';

		// add the appendix .t
		strcat(cTangentName, ".t");

		// check if file exist
		if (CheckFile(cTangentName) == 0)
		{
			GenerateTangent(&md3Meshes[i]);

			// open file
			File = fopen(cTangentName, "w+b");

			// write tangents into file
			for (unsigned int y = 0; y < dwNumberOfVertices; y++)
				fwrite(&md3Meshes[i].meshTangents[y].vTangent, sizeof(D3DXVECTOR3), 1, File);

			// close file
			fclose(File);
		}
		else
		{
			// open file
			File = fopen(cTangentName, "rb");

			// read tangents
			for (unsigned int y = 0; y < dwNumberOfVertices; y++)
				fread(&md3Meshes[i].meshTangents[y].vTangent, sizeof(D3DXVECTOR3), 1, File);

			// close file
			fclose(File);
		}

		free(meshTempVertices);

		lMeshOffset += md3Meshes[i].meshHeader.iMeshSize;
		md3Meshes[i].iNumTextures = NULL;

		for (j = 0; j < MAXSHADERPROFILE; j++)
		{
			md3Meshes[i].iChoosedShaderLevel[j] = NULL;
		}

		for (j = 0; j < MAXSHADERLEVELINMESH; j++)
		{
			md3Meshes[i].pVertexShader[j] = 0;
			md3Meshes[i].pPixelShader[j] = 0;
		}

		for (j = 0; j < MAXTEXTURESPERMESH; j++)
		{
			md3Meshes[i].pTexturesInterfaces[j] = 0;
		}
	}

	// close file
	fclose(md3file);

	// warning: the last frame for the quake3 model is header.numBoneFrames - 1
	modelHeader.iBoneFrameNum -= 1;

	// set the start, end frame
	iStartFrame = 0;
	iEndFrame = modelHeader.iBoneFrameNum;

	return 1;
}


//-------------------------------------------------------------------
// UpdateFrameTime
// 
// Desc: Update time and frame for curent model
//
// iStartFrame	- start frame
// iEndFrame	- end frame
// iFrame		- curent frame to draw
// iNextFrame	- nextframe to draw (only if interpolated)
//-------------------------------------------------------------------
void MD3Model::UpdateFrameTime(float time)
{
	fNewTime = time;

	if (fNewTime - fOldTime > 1. / iFps)
	{
		iFrame = iNextFrame;
		iNextFrame += 1;

		// Loop
		if (iNextFrame > iEndFrame) iNextFrame = iStartFrame;

		fOldTime = fNewTime;
	}
}


//---------------------------------------------------------------------
// DrawSkeleton
//
// 1. Sets all mesh data of a model into the "display buffer"
//    with DrawIndexedPrimitive()
// 2. Interpolate the models animation and position, when this model 
//	  has a link to a "higher" model. 
//	  That means: lower has a link to upper and upper to head, but 
//    head has no link to another higher part of the value. So it 
//    is not interpolated.
//
//---------------------------------------------------------------------
void MD3Model::DrawSkeleton(MD3Model* md3Model,
	IDirect3DDevice9* m_pDevice,
	IDirect3DIndexBuffer9* m_pIB,
	IDirect3DVertexBuffer9* m_pVB,
	D3DXMATRIX* matViewProj,
	D3DXMATRIX* matWorld,
	D3DXMATRIX* lightViewProj,
	D3DXMATRIX* scaleBias,
	IDirect3DVertexDeclaration9* m_pVertexDeclaration) 
{
	//-------------------------------------
	// draw one of the up to four md3 models 
	// 
	// 1. lower.md3
	// 2. upper.md3
	// 3. optional: railgun.md3
	// 4. head.md3
	//-------------------------------------
	D3DXMATRIX matClip, matTemp;
	int i, j;	// counter
	int iCurrMesh, iCurrOffsetVertex, iNextCurrOffsetVertex;

	// interpolation factor
	float fPol = md3Model->iFps * (md3Model->fNewTime - md3Model->fOldTime);

	// interpolation constant
	D3DXVECTOR4 intpol(fPol, 1.0f, 0.5f, 1.0f);
	m_pDevice->SetVertexShaderConstantF(37, (float*)&intpol, 1);

	for (iCurrMesh = 0; iCurrMesh < md3Model->modelHeader.iMeshNum; iCurrMesh++)
	{
		MD3VERTEXBUFFERSTRUCT* pVertexBuffer;
		m_pVB->Lock(0, 0, (void**)&pVertexBuffer, D3DLOCK_DISCARD);

		WORD* pIndices;	  // fill index buffer
		m_pIB->Lock(0, 0, (void**)&pIndices, D3DLOCK_DISCARD);
		DWORD dwIndexBufferCounter = 0;

		iCurrOffsetVertex = md3Model->iFrame * md3Model->md3Meshes[iCurrMesh].meshHeader.iVertexNum;
		iNextCurrOffsetVertex = md3Model->iNextFrame * md3Model->md3Meshes[iCurrMesh].meshHeader.iVertexNum;

		// fill index buffer
		for (i = 0; i < md3Model->md3Meshes[iCurrMesh].meshHeader.iTriangleNum; i++)
			for (j = 0; j < 3; j++)
				pIndices[dwIndexBufferCounter++] = (WORD)md3Model->md3Meshes[iCurrMesh].meshTriangles[i].index[j];

		for (i = 0; i < md3Model->md3Meshes[iCurrMesh].meshHeader.iVertexNum; i++)
		{
			// interpolated vertices ----------------------------------
			// pseudo code: CurrVertex.vPosition +  fPol * (NextVertex.vPosition - CurrVertex.vPosition);
			pVertexBuffer[i].vPosition =
				md3Model->md3Meshes[iCurrMesh].meshVertices[iCurrOffsetVertex + i].vVector +
				fPol *
				(md3Model->md3Meshes[iCurrMesh].meshVertices[iNextCurrOffsetVertex + i].vVector -
					md3Model->md3Meshes[iCurrMesh].meshVertices[iCurrOffsetVertex + i].vVector);

			pVertexBuffer[i].vVertexNormal =
				md3Model->md3Meshes[iCurrMesh].meshVertices[iCurrOffsetVertex + i].vVertexNormal +
				fPol *
				(md3Model->md3Meshes[iCurrMesh].meshVertices[iNextCurrOffsetVertex + i].vVertexNormal -
					md3Model->md3Meshes[iCurrMesh].meshVertices[iCurrOffsetVertex + i].vVertexNormal);


			// texture coordinates -------------------------
			pVertexBuffer[i].vTexCoord = md3Model->md3Meshes[iCurrMesh].meshTexCoord[i].texvec;

			// tangent vector
			pVertexBuffer[i].vTangent = md3Model->md3Meshes[iCurrMesh].meshTangents[iCurrOffsetVertex + i].vTangent +
				fPol *
				(md3Model->md3Meshes[iCurrMesh].meshTangents[iNextCurrOffsetVertex + i].vTangent -
					md3Model->md3Meshes[iCurrMesh].meshTangents[iCurrOffsetVertex + i].vTangent);
		}

		m_pVB->Unlock();
		m_pIB->Unlock();

		// world * view * proj matrix
		D3DXMATRIX  matTemp;
		D3DXMatrixMultiply(&matClip, matWorld, matViewProj);
		D3DXMatrixTranspose(&matTemp, &matClip);
		m_pDevice->SetVertexShaderConstantF(8, (float*)&matTemp, 4);

		// world matrix
		D3DXMatrixTranspose(&matTemp, matWorld);
		m_pDevice->SetVertexShaderConstantF(0, (float*)&matTemp, 4);

		// light view-projection matrix
		D3DXMATRIX matLightViewProj;
		D3DXMatrixMultiply(&matLightViewProj, matWorld, lightViewProj);

		// light view-projection texture-adjusted matrix
		D3DXMATRIX matLightViewProjTexAdj;
		D3DXMatrixMultiply(&matLightViewProjTexAdj, &matLightViewProj, scaleBias);

		D3DXMatrixTranspose(&matLightViewProj, &matLightViewProj);
		D3DXMatrixTranspose(&matLightViewProjTexAdj, &matLightViewProjTexAdj);

		m_pDevice->SetVertexShaderConstantF(16, reinterpret_cast<float*>(&matLightViewProj), 4);
		m_pDevice->SetVertexShaderConstantF(20, reinterpret_cast<float*>(&matLightViewProjTexAdj), 4);

		m_pDevice->SetVertexDeclaration(m_pVertexDeclaration);

		//
		// set vertex shader 
		//
		//m_pDevice->SetVertexShader(md3Model->md3Meshes[iCurrMesh].pVertexShader[md3Model->md3Meshes[iCurrMesh].iChoosedShaderLevel[iShaderProfile]]);

		const int activeProfile = iShaderProfile;
		const int chosenShaderLevel =
			md3Model->md3Meshes[iCurrMesh].iChoosedShaderLevel[activeProfile];

		IDirect3DVertexShader9* vertexShader =
			md3Model->md3Meshes[iCurrMesh].pVertexShader[chosenShaderLevel];

		IDirect3DPixelShader9* pixelShader =
			md3Model->md3Meshes[iCurrMesh].pPixelShader[chosenShaderLevel];

		//{
		//	char msg[512] = {};
		//	sprintf_s(
		//		msg,
		//		"MD3 DrawSkeleton mesh='%s' profile=%d shaderLevel=%d VS=%p PS=%p",
		//		md3Model->md3Meshes[iCurrMesh].meshHeader.cName,
		//		activeProfile,
		//		chosenShaderLevel,
		//		vertexShader,
		//		pixelShader);

		//	LOG(std::string(msg), Logger::LOG_DATA);
		//}

		m_pDevice->SetVertexShader(vertexShader);
		m_pDevice->SetPixelShader(pixelShader);

		//
		// texture caching
		//
		if (md3Model->md3Meshes[iCurrMesh].iNumTextures > 0)
		{
			for (i = 0; i < md3Model->md3Meshes[iCurrMesh].iNumTextures; i++)
			{
				if (pMiniTextureCache[i] != md3Model->md3Meshes[iCurrMesh].pTexturesInterfaces[i])
				{
					pMiniTextureCache[i] = md3Model->md3Meshes[iCurrMesh].pTexturesInterfaces[i];
					m_pDevice->SetTexture(i, pMiniTextureCache[i]);
				}
			}
		}

		//
		// set pixel shader
		//
		m_pDevice->SetPixelShader(md3Model->md3Meshes[iCurrMesh].pPixelShader[md3Model->md3Meshes[iCurrMesh].iChoosedShaderLevel[iShaderProfile]]);



		// set the vertex buffer
		// == specify the source of stream 0
		m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(MD3VERTEXBUFFERSTRUCT));

		// set the index buffer
		m_pDevice->SetIndices(m_pIB);

		// ... rendering
		m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0,
			0,
			// number of vertices
			md3Model->md3Meshes[iCurrMesh].meshHeader.iVertexNum,
			0,
			// number of primitives
			md3Model->md3Meshes[iCurrMesh].meshHeader.iTriangleNum);
	}

	//
	// interpolate position of tags of lower.md3 (md3lower), upper.md3 (md3upper) and optional railgun.md3 (md3weapon)	
	// Not: head.md3
	//
	MD3Model* modelLink;

	D3DXQUATERNION quatFromMatrix, quatFromMatrix2, quatResult;
	D3DXMATRIX matFrame, matNextFrame;

	int iModelFrame = md3Model->iFrame * md3Model->modelHeader.iTagNum;
	int iModelNextFrame = md3Model->iNextFrame * md3Model->modelHeader.iTagNum;

	for (i = 0; i < md3Model->modelHeader.iTagNum; i++)
	{
		// pointer to a model class
		modelLink = md3Model->md3Links[i];

		if (modelLink)
		{
			// rotation matrix
			matFrame = &md3Model->d3dTag[iModelFrame + i].matTag.m[0][0];
			matNextFrame = &md3Model->d3dTag[iModelNextFrame + i].matTag.m[0][0];

			// quaternion interpolation 
			D3DXQuaternionRotationMatrix(&quatFromMatrix, &matFrame);
			D3DXQuaternionRotationMatrix(&quatFromMatrix2, &matNextFrame);
			D3DXQuaternionSlerp(&quatResult, &quatFromMatrix, &quatFromMatrix2, fPol);
			D3DXMatrixRotationQuaternion(&matTemp, &quatResult);

			// interpolated position vector
			// Pseudo code: Position[0] + fPol * (nextPosition[0] - Position[0]);
			matTemp[12] = md3Model->d3dTag[iModelFrame + i].matTag.m[3][0] + fPol *
				(md3Model->d3dTag[iModelNextFrame + i].matTag.m[3][0] -
					md3Model->d3dTag[iModelFrame + i].matTag.m[3][0]);

			matTemp[13] = md3Model->d3dTag[iModelFrame + i].matTag.m[3][1] + fPol *
				(md3Model->d3dTag[iModelNextFrame + i].matTag.m[3][1] -
					md3Model->d3dTag[iModelFrame + i].matTag.m[3][1]);

			matTemp[14] = md3Model->d3dTag[iModelFrame + i].matTag.m[3][2] + fPol *
				(md3Model->d3dTag[iModelNextFrame + i].matTag.m[3][2] -
					md3Model->d3dTag[iModelFrame + i].matTag.m[3][2]);
			matTemp[15] = 1.0f; matTemp[3] = matTemp[7] = matTemp[11] = 0;

			D3DXMatrixMultiply(&matTemp, &matTemp, matWorld);
			DrawSkeleton(
				modelLink,
				m_pDevice,
				m_pIB,
				m_pVB,
				matViewProj,
				&matTemp,
				lightViewProj,
				scaleBias,
				m_pVertexDeclaration);
		}
	}
}

//-------------------------------------------------------------------------
// LinkModel
//
// Link model to tag (f.e. "tag_weapon" to model weapon)
//-------------------------------------------------------------------------
int MD3Model::LinkModel(const char* tagname, MD3Model* mod)
{
	for (int i = 0; i < modelHeader.iTagNum; i++)
		if (!strcmp(d3dTag[i].name, tagname))
		{
			md3Links[i] = mod;
			return i;
		}
	return -1;
}

//--------------------------------------------------------------------------
// UnLinkModel
// 
// Desc: UnLink model from the tag
//--------------------------------------------------------------------------
void MD3Model::UnLinkModel(const char* tagname)
{
	for (int i = 0; i < modelHeader.iTagNum; i++)
		if (!strcmp(d3dTag[i].name, tagname))
		{
			md3Links[i] = NULL;
			return;
		}
}

//------------------------------------------------------------------------------
// LoadSkins
//
// Desc: Reads .skin file for current model and loads textures with 
//		D3DXCreateTextureFromFile()
//
// pcFileName	- name for the skin file with .skin extension
// pcImagepath	- directory for the texure to load f.e. *.TGA *.JPG, *.dds
//-------------------------------------------------------------------------------
void MD3Model::LoadSkins(IDirect3DDevice9* device, const char* filename, const char* imagePath)
{
	Legacy::LoadMD3Skins(device, md3Meshes, modelHeader.iMeshNum, filename, imagePath);
}

//-------------------------------------------------------------------
// DeleteSkins
//
// Desc: deletes skin of one model
//-------------------------------------------------------------------
void MD3Model::DeleteSkins()
{
	for (int j = 0; j < modelHeader.iMeshNum; j++)
	{

		// destroy textures
		for (int i = 0; i < md3Meshes[j].iNumTextures; i++)
		{
			SAFE_RELEASE(md3Meshes[j].pTexturesInterfaces[i]);
		}
		md3Meshes[j].iNumTextures = 0;
	}
}


//------------------------------------------------------------------------------
// LoadShaders
//
// Desc: Load .sha file for curent model 
//
// pcFileName	- name for the sha file with .sha extension
// pcShaderPath	- directory for the shader to load
//-------------------------------------------------------------------------------
void MD3Model::LoadShaders(IDirect3DDevice9* device, const char* filename, const char* shaderPath)
{
	Legacy::LoadMD3Shaders(device, md3Meshes, modelHeader.iMeshNum, filename, shaderPath);
}

//-------------------------------------------------------------------
// DeleteShaders
//
// Desc: deletes shaders of one model
//-------------------------------------------------------------------
void MD3Model::DeleteShaders(IDirect3DDevice9* m_pd3dDevice)
{

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		for (int j = 0; j < MAXSHADERLEVELINMESH; j++)
			if (md3Meshes[i].pVertexShader[j])
				SAFE_RELEASE(md3Meshes[i].pVertexShader[j]);
	}

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		for (int j = 0; j < MAXSHADERLEVELINMESH; j++)
			if (md3Meshes[i].pPixelShader[j])
				SAFE_RELEASE(md3Meshes[i].pPixelShader[j]);
	}
}

#ifdef _DEBUG

//-------------------------------------------------------------------
// DumpGeometryInfo
//
// Desc: Dump all quake3 model info
//-------------------------------------------------------------------
void MD3Model::DumpGeometryInfo()
{
	LOGFUNC("DumpGeometryInfo()");

	//
	// write into logfile
	//
	LOG("dump " + std::string(cModelName) + "-----------------------------------------------", Logger::LOG_DATA);
	CHAR strData[1024];
	sprintf(strData, "\n\t\t\t\t\tid: %s"
		"\n\t\t\t\t\tHeader Version: %d"
		"\n\t\t\t\t\tName of File: %s"
		"\n\t\t\t\t\tNumber of BoneFrames: %d"
		"\n\t\t\t\t\tNumber of Tags: %d"
		"\n\t\t\t\t\tNumber of Meshes: %d"
		"\n\t\t\t\t\tNumber of Textures: %d"
		"\n\t\t\t\t\tSize of Header: %d"
		"\n\t\t\t\t\tStart of Tags data: %d"
		"\n\t\t\t\t\tStart of Mesh data: %d"
		"\n\t\t\t\t\tSize of File: %d",
		modelHeader.id,
		modelHeader.iHeaderSize,
		modelHeader.cFileName,
		modelHeader.iBoneFrameNum,
		modelHeader.iTagNum,
		modelHeader.iMeshNum,
		modelHeader.iMaxTextureNum,
		modelHeader.iHeaderSize,
		modelHeader.iTagStart,
		modelHeader.iMeshStart,
		modelHeader.iFileSize);
	LOG(std::string(strData), Logger::LOG_DATA);

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		CHAR strData[1024];
		sprintf(strData, "\n\t\t\t\t\tMesh Number: %d -------"
			"\n\t\t\t\t\tMesh ID: %s"
			"\n\t\t\t\t\tName of Mesh: %s"
			"\n\t\t\t\t\tNumber of Frames: %d"
			"\n\t\t\t\t\tNumber of Textures: %d"
			"\n\t\t\t\t\tNumber of Vertices: %d"
			"\n\t\t\t\t\tNumber of Triangles: %d"
			"\n\t\t\t\t\tStart of Triangle Data: %d"
			"\n\t\t\t\t\tStart of Tags data: %d"
			"\n\t\t\t\t\tStart of Mesh data: %d"
			"\n\t\t\t\t\tSize of Header: %d"
			"\n\t\t\t\t\tStart of Texture Coordinate Data: %d"
			"\n\t\t\t\t\tStart of Vertex Data: %d"
			"\n\t\t\t\t\tSize of Mesh: %d",
			i,
			md3Meshes[i].meshHeader.cId,
			md3Meshes[i].meshHeader.cName,
			md3Meshes[i].meshHeader.iMeshFrameNum,
			md3Meshes[i].meshHeader.iTextureNum,
			md3Meshes[i].meshHeader.iVertexNum,
			md3Meshes[i].meshHeader.iTriangleNum,
			md3Meshes[i].meshHeader.iTriangleStart,
			0,
			0,
			md3Meshes[i].meshHeader.iHeaderSize,
			md3Meshes[i].meshHeader.iTecVecStart,
			md3Meshes[i].meshHeader.iVertexStart,
			md3Meshes[i].meshHeader.iMeshSize);
		LOG(std::string(strData), Logger::LOG_DATA);
	}
}

//-------------------------------------------------------------------
// DumpShaderInfo
//
// Desc: Dump all shader info
//-------------------------------------------------------------------
void MD3Model::DumpShaderInfo(void)
{
	LOGFUNC("DumpShaderInfo()");

	LOG("Only shaders of the choosed shaderlevel of " + std::string(cModelName) + "--------------", Logger::LOG_DATA);

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		LOG("Mesh name: " + std::string(md3Meshes[i].meshHeader.cName), Logger::LOG_DATA);

		CHAR strData[128];
		sprintf_s(strData,
			"Vertex shader handle: %p choosed shader level: %d",
			reinterpret_cast<void*>(md3Meshes[i].pVertexShader[md3Meshes[i].iChoosedShaderLevel[iShaderProfile]]),
			md3Meshes[i].iChoosedShaderLevel[iShaderProfile]);
		LOG(std::string(strData), Logger::LOG_DATA);
	}

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		LOG("Mesh name: " + std::string(md3Meshes[i].meshHeader.cName), Logger::LOG_DATA);

		CHAR strData[128];
		sprintf_s(strData,
			"Pixel shader handle: %p",
			reinterpret_cast<void*>(md3Meshes[i].pPixelShader[md3Meshes[i].iChoosedShaderLevel[iShaderProfile]]));
		LOG(std::string(strData), Logger::LOG_DATA);
	}

	LOG("All loaded shaders -----------------------", Logger::LOG_DATA);

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		LOG("Mesh name: " + std::string(md3Meshes[i].meshHeader.cName) + "----", Logger::LOG_DATA);

		for (int j = 0; j < MAXSHADERLEVELINMESH; j++)
		{
			if (md3Meshes[i].pVertexShader[j] != 0)
			{
				CHAR strData[128];
				sprintf_s(strData,
					"Vertex Shader Handle: %p;  Shaderlevel: %d",
					reinterpret_cast<void*>(md3Meshes[i].pVertexShader[j]),
					j);
				LOG(std::string(strData), Logger::LOG_DATA);
			}
		}

		for (int j = 0; j < MAXSHADERLEVELINMESH; j++)
		{
			if (md3Meshes[i].pPixelShader[j] != 0)
			{
				CHAR strData[128];
				sprintf_s(strData,
					"Pixel Shader Handle: %p;  Shaderlevel: %d",
					reinterpret_cast<void*>(md3Meshes[i].pPixelShader[j]),
					j);
				LOG(std::string(strData), Logger::LOG_DATA);
			}
		}
	}
}

//-------------------------------------------------------------------
// DumpSkinInfo
//
// Desc: Dump all shader info
//-------------------------------------------------------------------
void MD3Model::DumpSkinInfo(void)
{
	LOGFUNC("DumpSkinInfo()");

	//
	// write into logfile
	//
	LOG("Texture Slots of " + std::string(cModelName), Logger::LOG_DATA);

	for (int i = 0; i < modelHeader.iMeshNum; i++)
	{
		LOG("Mesh name: " + std::string(md3Meshes[i].meshHeader.cName) + " ---- ", Logger::LOG_DATA);

		for (int j = 0; j < MAXTEXTURESPERMESH; j++)
		{
			if (md3Meshes[i].pTexturesInterfaces[j] != 0)
			{
				CHAR strData[128];
				sprintf_s(strData,
					"Texture Handle %p in texture slot #%d",
					reinterpret_cast<void*>(md3Meshes[i].pTexturesInterfaces[j]),
					j);
				LOG(strData, Logger::LOG_DATA);
			}
		}
	}
}

#endif

//-----------------------------------------------------------------------------
// Name: QuantiseVector
// Desc: takes 3 floats and returns a quantised vector
//-----------------------------------------------------------------------------
/*
unsigned int QuantiseVector( const float nx, const float ny, const float nz)
{
	// -1.0 - 1.f -> 0.f - 255.f
	unsigned int ix = unsigned int( (nx * 127.5f) + 127.5f );
	unsigned int iy = unsigned int( (ny * 127.5f) + 127.5f );
	unsigned int iz = unsigned int( (nz * 127.5f) + 127.5f );
	unsigned int iw = 0; // we don't use the w component (just padding)

	unsigned int out = (iw << 24) | (iz << 16) | (iy << 8) | (ix << 0);

	return out;
}
*/
//-------------------------------------------------------------------
// CalculateNormals2
//
// Desc: Calculates vertex normals
//-------------------------------------------------------------------
void MD3Model::CalculateNormals2(MD3MESH* MD3Mesh)
{
	int iIndice0, iIndice1, iIndice2;
	D3DXVECTOR3 v, v0, v1, v2, vEdge1, vEdge2, vNormal;
	D3DXVECTOR3 vSum;
	int iShared;

	// allcocate memory for face normals
	D3DXVECTOR3* vFaceNormals = (D3DXVECTOR3*)malloc(sizeof(D3DXVECTOR3) *
		MD3Mesh->meshHeader.iMeshFrameNum *
		MD3Mesh->meshHeader.iTriangleNum);
	// compute face normals
	// for every triangle, take three vertices and cross the edges
	for (int x = 0; x < MD3Mesh->meshHeader.iMeshFrameNum; x++)
	{
		int iBase = x * MD3Mesh->meshHeader.iVertexNum;
		int iTriangleBase = x * MD3Mesh->meshHeader.iTriangleNum;

		for (int i = 0; i < MD3Mesh->meshHeader.iTriangleNum; i++)
		{
			// three indices of a triangle
			iIndice0 = MD3Mesh->meshTriangles[i].index[0];
			iIndice1 = MD3Mesh->meshTriangles[i].index[1];
			iIndice2 = MD3Mesh->meshTriangles[i].index[2];

			// the three vertices of this triangle
			v0 = MD3Mesh->meshVertices[iBase + iIndice0].vVector;
			v1 = MD3Mesh->meshVertices[iBase + iIndice1].vVector;
			v2 = MD3Mesh->meshVertices[iBase + iIndice2].vVector;

			// two edges of this triangle
			vEdge1 = v1 - v0;
			vEdge2 = v2 - v0;
			D3DXVec3Cross(&vNormal, &vEdge1, &vEdge2);
			D3DXVec3Normalize(&vNormal, &vNormal);
			vFaceNormals[iTriangleBase + i] = vNormal;
		}
	}


	// compute vertex normals
	// go through every vertex and average its face normals
	for (int y = 0; y < MD3Mesh->meshHeader.iMeshFrameNum; y++)
	{
		int iBase = y * MD3Mesh->meshHeader.iVertexNum;
		int iTriangleBase = y * MD3Mesh->meshHeader.iTriangleNum;

		for (int c = 0; c < MD3Mesh->meshHeader.iVertexNum; c++)
		{
			v = MD3Mesh->meshVertices[iBase + c].vVector;
			iShared = 0;
			vSum = D3DXVECTOR3(0.0, 0.0, 0.0);

			// searches through all triangles to find the vertices who
			// share vertex v
			for (int i = 0; i < MD3Mesh->meshHeader.iTriangleNum; i++)
			{
				// three indices of a triangle
				iIndice0 = MD3Mesh->meshTriangles[i].index[0];
				iIndice1 = MD3Mesh->meshTriangles[i].index[1];
				iIndice2 = MD3Mesh->meshTriangles[i].index[2];

				// the three vertices of this triangle
				v0 = MD3Mesh->meshVertices[iBase + iIndice0].vVector;
				v1 = MD3Mesh->meshVertices[iBase + iIndice1].vVector;
				v2 = MD3Mesh->meshVertices[iBase + iIndice2].vVector;

				// does one of these vertices share v ?
				// if yes take the facenormal of this vertex
				// and sum it up
				if (v0 == v || v1 == v || v2 == v)
				{
					vSum += vFaceNormals[iTriangleBase + i];
					++iShared;
				}
			}
			// the average face normal should be the vertex normal
			vNormal = vSum / (float)iShared;
			D3DXVec3Normalize(&vNormal, &vNormal);
			MD3Mesh->meshVertices[iBase + c].vVertexNormal = vNormal;
		}
	}
	free(vFaceNormals);
}

//-------------------------------------------------------------------
// ComputeDuDv
//
// Desc: the following computations are based on the idea
// and code by Sim Dietrich from NVidia
//-------------------------------------------------------------------
void MD3Model::ComputeDuDv(const D3DXVECTOR3& v0_pos, const D3DXVECTOR2& v0_uv,
	const D3DXVECTOR3& v1_pos, const D3DXVECTOR2& v1_uv,
	const D3DXVECTOR3& v2_pos, const D3DXVECTOR2& v2_uv,
	TANGENTS& meshTangents)
{
	D3DXVECTOR3 e0, e1, n;
	D3DXVECTOR3 du(0, 0, 0);
	D3DXVECTOR3 dv(0, 0, 0);

	// we need partial derivatives of u,v (s,t)
	// relative to vertex x,y,z world coordinates

	// we take two edges of the triangle
	// for vertex x world coordinate
	// and texture u,v coordinates
	e0 = D3DXVECTOR3(v1_pos.x - v0_pos.x,
		v1_uv.x - v0_uv.x,
		v1_uv.y - v0_uv.y);
	e1 = D3DXVECTOR3(v2_pos.x - v0_pos.x,
		v2_uv.x - v0_uv.x,
		v2_uv.y - v0_uv.y);

	// cross product the edges; this creates a vector
	// that is a normal vector to a plane in which the edges lie
	D3DXVec3Cross(&n, &e0, &e1);

	// the plane equation is:
	// Ax + Bu + Cv + D = 0; (A,B,C) is our normal
	// dudx = -Bu/A
	// dvdx = -Cv/A
	if (fabs(n.x) > 1e-12)
	{
		du.x = -n.y / n.x;
		dv.x = -n.z / n.x;
	}

	// for vertex y world coordinate
	// and texture u,v coordinates
	e0 = D3DXVECTOR3(v1_pos.y - v0_pos.y,
		v1_uv.x - v0_uv.x,
		v1_uv.y - v0_uv.y);
	e1 = D3DXVECTOR3(v2_pos.y - v0_pos.y,
		v2_uv.x - v0_uv.x,
		v2_uv.y - v0_uv.y);
	// generate normal
	D3DXVec3Cross(&n, &e0, &e1);

	// Ay + Bu + Cv + D = 0; (A,B,C) is our normal
	// dudy = -Bu/A
	// dvdy = -Cv/A
	if (fabs(n.x) > 1e-12) {
		du.y = -n.y / n.x;
		dv.y = -n.z / n.x;
	}
	// for vertex z world coordinate
	// and texture u,v coordinates
	e0 = D3DXVECTOR3(v1_pos.z - v0_pos.z,
		v1_uv.x - v0_uv.x,
		v1_uv.y - v0_uv.y);
	e1 = D3DXVECTOR3(v2_pos.z - v0_pos.z,
		v2_uv.x - v0_uv.x,
		v2_uv.y - v0_uv.y);
	// generate normal
	D3DXVec3Cross(&n, &e0, &e1);

	// Az + Bu + Cv + D = 0; (A,B,C) is our normal
	// dudz = -Bu/A
	// dvdz = -Cv/A
	if (fabs(n.x) > 1e-12) {
		du.z = -n.y / n.x;
		dv.z = -n.z / n.x;
	}
	// accumulate
	meshTangents.vTangent += du;
	meshTangents.vBinormal += dv;
}


//-------------------------------------------------------------------
// AverageTriangles
//
// Desc: averages du, dv 
//-------------------------------------------------------------------
void MD3Model::AverageTriangles(int index, int tri_count, int frame, MD3MESH* MD3Mesh)
{
	int iIndice;
	D3DXVECTOR3 v0, tmp;
	int count = 0;

	int base = frame * MD3Mesh->meshHeader.iVertexNum;
	D3DXVECTOR3 v = MD3Mesh->meshVertices[base + index].vVector;

	// go through triangles and find vertices that
	// share coordinates with our vertex and average du,dv
	for (int i = 0; i < tri_count; i++)
	{
		// check all verts
		for (int c = 0; c < 3; c++)
		{
			iIndice = MD3Mesh->meshTriangles[i].index[c];
			v0 = MD3Mesh->meshVertices[base + iIndice].vVector;

			// same coords, different index                        
			if (v.x == v0.x && v.y == v0.y && v.z == v0.z && index != iIndice)
			{
				// average du,dv
				tmp = MD3Mesh->meshTangents[base + index].vTangent;
				MD3Mesh->meshTangents[base + index].vTangent += MD3Mesh->meshTangents[base + iIndice].vTangent;
				MD3Mesh->meshTangents[base + iIndice].vTangent += tmp;

				tmp = MD3Mesh->meshTangents[base + index].vBinormal;
				MD3Mesh->meshTangents[base + index].vBinormal += MD3Mesh->meshTangents[base + iIndice].vBinormal;
				MD3Mesh->meshTangents[base + iIndice].vBinormal += tmp;
			}
		}
	}
}

//-------------------------------------------------------------------
// GenerateTangent
//
// Desc: generates tangent and binormal vector
//		 for per-vertex tangent space coordinates system
//-------------------------------------------------------------------
void MD3Model::GenerateTangent(MD3MESH* MD3Mesh)
{
	int iIndice0, iIndice1, iIndice2;
	D3DXVECTOR3 v0_pos, v1_pos, v2_pos;
	D3DXVECTOR2 v0_uv, v1_uv, v2_uv;

	// 

	for (int c = 0; c < (MD3Mesh->meshHeader.iMeshFrameNum * MD3Mesh->meshHeader.iVertexNum); c++)
	{
		MD3Mesh->meshTangents[c].vTangent = D3DXVECTOR3(0, 0, 0);
		MD3Mesh->meshTangents[c].vBinormal = D3DXVECTOR3(0, 0, 0);
	}

	for (int frame = 0; frame < MD3Mesh->meshHeader.iMeshFrameNum; frame++)
	{
		int base = frame * MD3Mesh->meshHeader.iVertexNum;

		for (int i = 0; i < MD3Mesh->meshHeader.iTriangleNum; i++)
		{
			iIndice0 = MD3Mesh->meshTriangles[i].index[0];
			iIndice1 = MD3Mesh->meshTriangles[i].index[1];
			iIndice2 = MD3Mesh->meshTriangles[i].index[2];

			// the three vertices of this triangle
			v0_pos = MD3Mesh->meshVertices[base + iIndice0].vVector;
			v1_pos = MD3Mesh->meshVertices[base + iIndice1].vVector;
			v2_pos = MD3Mesh->meshVertices[base + iIndice2].vVector;

			v0_uv = (D3DXVECTOR2)MD3Mesh->meshTexCoord[iIndice0].texvec;
			v1_uv = (D3DXVECTOR2)MD3Mesh->meshTexCoord[iIndice1].texvec;
			v2_uv = (D3DXVECTOR2)MD3Mesh->meshTexCoord[iIndice2].texvec;

			ComputeDuDv(v0_pos, v0_uv,
				v1_pos, v1_uv,
				v2_pos, v2_uv,
				MD3Mesh->meshTangents[base + iIndice0]);

			ComputeDuDv(v2_pos, v2_uv,
				v0_pos, v0_uv,
				v1_pos, v1_uv,
				MD3Mesh->meshTangents[base + iIndice1]);

			ComputeDuDv(v1_pos, v1_uv,
				v2_pos, v2_uv,
				v0_pos, v0_uv,
				MD3Mesh->meshTangents[base + iIndice2]);

			AverageTriangles(iIndice0, i, frame, MD3Mesh);
			AverageTriangles(iIndice1, i, frame, MD3Mesh);
			AverageTriangles(iIndice2, i, frame, MD3Mesh);
		}
	}

	// normalize accumulated du,dv
	for (int i = 0; i < (MD3Mesh->meshHeader.iMeshFrameNum * MD3Mesh->meshHeader.iVertexNum); i++)
	{
		D3DXVec3Normalize(&MD3Mesh->meshTangents[i].vTangent, &MD3Mesh->meshTangents[i].vTangent);
		D3DXVec3Normalize(&MD3Mesh->meshTangents[i].vBinormal, &MD3Mesh->meshTangents[i].vBinormal);
	}
}
