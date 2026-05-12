#include "D3D9Compat.h"
#include "MD3SkinLoader.h"

#include "logger.h"
#include "Utility.h"

#include <cstring>
#include <string>

namespace
{
    int FindMeshIndex(MD3MESH* meshes, int meshCount, const char* meshName)
    {
        if (!meshes || !meshName)
            return -1;

        for (int i = 0; i < meshCount; ++i)
        {
            if (strstr(meshes[i].meshHeader.cName, meshName))
                return i;
        }

        return -1;
    }
}

namespace Legacy
{
    void LoadMD3Skins(
        IDirect3DDevice9* device,
        MD3MESH* meshes,
        int meshCount,
        const char* skinFilePath,
        const char* imageRootPath)
    {
        LOGFUNC("LoadMD3Skins()");

        if (!device || !meshes || meshCount <= 0 || CheckFile(skinFilePath) == 0)
            return;

        FILE* skinFile = fopen(skinFilePath, "rt");
        if (!skinFile)
            return;

        CHAR cImageName[1024];
        CHAR cMeshName[16];
        CHAR strToken[1024];
        CHAR cPath[1024];
        CHAR pcTextLine[1024];
        WCHAR widePath[1024];

        while (!feof(skinFile))
        {
            BOOL bMesh = FALSE;
            BOOL bTag = FALSE;
            BOOL bImage = FALSE;
            pcTextLine[0] = '\0';

            fscanf(skinFile, "%s", pcTextLine);

            while (pcTextLine && pcTextLine[0])
            {
                ParseTextLine(pcTextLine, strToken);

                if (strstr(_strlwr(strToken), "tag_") && bMesh == FALSE && bTag == FALSE)
                {
                    bTag = TRUE;
                }
                else if (bMesh == FALSE && bTag == FALSE)
                {
                    strcpy(cMeshName, strToken);
                    bMesh = TRUE;
                }
                else if ((strstr(strToken, ".tga") || strstr(strToken, ".jpg") || strstr(strToken, ".dds")) && bMesh == TRUE && bTag == FALSE)
                {
                    strcpy(cImageName, strToken);
                    bImage = TRUE;
                }
            }

            if (!bMesh || !bImage)
                continue;

            sprintf_s(cPath, sizeof(cPath), "%s\\%s", imageRootPath, cImageName);
            LOG("Found Image " + std::string(cImageName) + " for mesh " + std::string(cMeshName), Logger::LOG_DATA);

            const int meshIndex = FindMeshIndex(meshes, meshCount, cMeshName);
            if (meshIndex < 0)
            {
                LOG("Didn't find a Mesh with the name " + std::string(cMeshName) + " for image " + std::string(cImageName), Logger::LOG_ERR);
                continue;
            }

            if (meshes[meshIndex].iNumTextures >= MAXTEXTURESPERMESH)
            {
                LOG("Too many textures for Mesh " + std::string(cMeshName), Logger::LOG_ERR);
                continue;
            }

            mbstowcs(widePath, cPath, strlen(cPath) + 1);

            IDirect3DTexture9** textureSlot = &meshes[meshIndex].pTexturesInterfaces[meshes[meshIndex].iNumTextures];
            if (FAILED(D3DXCreateTextureFromFileW(device, widePath, textureSlot)))
            {
                LOG("Could not load Image " + std::string(cImageName) + " for Mesh " + std::string(cMeshName), Logger::LOG_ERR);
                *textureSlot = nullptr;
            }
            else
            {
                LOG("Loaded Image " + std::string(cImageName) + " for Mesh " + std::string(cMeshName), Logger::LOG_DATA);
                meshes[meshIndex].iNumTextures += 1;
            }
        }

        fclose(skinFile);
    }
}
