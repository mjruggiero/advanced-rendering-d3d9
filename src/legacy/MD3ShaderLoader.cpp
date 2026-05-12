#include "D3D9Compat.h"
#include "MD3ShaderLoader.h"

#include "logger.h"
#include "Utility.h"

#include <cstring>
#include <string>

namespace
{
    static void BuildShaderFilePath(
        char* outPath,
        size_t outPathSize,
        const char* shaderName,
        const char* shaderRootPath)
    {
        if (!outPath || outPathSize == 0)
            return;

        outPath[0] = '\0';

        if (!shaderName || shaderName[0] == '\0')
            return;

        // If the .sha file already provides a path, preserve it.
        if (strchr(shaderName, '\\') || strchr(shaderName, '/'))
        {
            sprintf_s(outPath, outPathSize, "%s", shaderName);
            return;
        }

        const char* root = (shaderRootPath && shaderRootPath[0] != '\0')
            ? shaderRootPath
            : "shaders";

        sprintf_s(outPath, outPathSize, "%s\\%s", root, shaderName);
    }

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

    void LogShaderAssemblyError(const char* stage, const char* path, ID3DXBuffer* errors)
    {
        if (errors)
        {
            LOG(std::string(stage) + " shader assembly error: " +
                std::string(static_cast<const char*>(errors->GetBufferPointer())),
                Logger::LOG_ERR);
        }

        LOG(std::string("Could not assemble ") + stage + " Shader: " + path, Logger::LOG_ERR);
    }

    void LoadVertexShader(IDirect3DDevice9* device, MD3MESH& mesh, int shaderLevel, const char* shaderPath, const char* meshName)
    {
        LPD3DXBUFFER pCode = nullptr;
        LPD3DXBUFFER pErrors = nullptr;
        HRESULT hr = S_OK;

        DWORD dwFlags = 0;
#if defined(_DEBUG) || defined(DEBUG)
        dwFlags |= D3DXSHADER_DEBUG;
#endif

        hr = D3DXAssembleShaderFromFileA(shaderPath, nullptr, nullptr, dwFlags, &pCode, &pErrors);
        if (FAILED(hr))
        {
            LogShaderAssemblyError("Vertex", shaderPath, pErrors);
            SAFE_RELEASE(pErrors);
            SAFE_RELEASE(pCode);
            mesh.pVertexShader[shaderLevel] = nullptr;
            return;
        }

        hr = device->CreateVertexShader(
            reinterpret_cast<DWORD*>(pCode->GetBufferPointer()),
            &mesh.pVertexShader[shaderLevel]);

        SAFE_RELEASE(pErrors);
        SAFE_RELEASE(pCode);

        if (SUCCEEDED(hr))
        {
            CHAR strData[256];
            sprintf_s(strData, "Created Vertex Shader: %s for Mesh: %s with handle #%p", shaderPath, meshName, mesh.pVertexShader[shaderLevel]);
            LOG(std::string(strData), Logger::LOG_DATA);
        }
        else
        {
            LOG("Could not create Vertex Shader " + std::string(shaderPath) + " for Mesh " + std::string(meshName), Logger::LOG_ERR);
            mesh.pVertexShader[shaderLevel] = nullptr;
        }
    }

    void LoadPixelShader(IDirect3DDevice9* device, MD3MESH& mesh, int shaderLevel, const char* shaderPath, const char* meshName)
    {
        LPD3DXBUFFER pCode = nullptr;
        LPD3DXBUFFER pErrors = nullptr;
        HRESULT hr = D3DXAssembleShaderFromFileA(shaderPath, nullptr, nullptr, 0, &pCode, &pErrors);

        if (FAILED(hr))
        {
            LogShaderAssemblyError("Pixel", shaderPath, pErrors);
            SAFE_RELEASE(pErrors);
            SAFE_RELEASE(pCode);
            mesh.pPixelShader[shaderLevel] = nullptr;
            return;
        }

        hr = device->CreatePixelShader(
            reinterpret_cast<DWORD*>(pCode->GetBufferPointer()),
            &mesh.pPixelShader[shaderLevel]);

        SAFE_RELEASE(pErrors);
        SAFE_RELEASE(pCode);

        if (SUCCEEDED(hr))
        {
            CHAR strData[256];
            sprintf_s(strData, "Created Pixel Shader: %s for Mesh: %s with handle #%p", shaderPath, meshName, mesh.pPixelShader[shaderLevel]);
            LOG(std::string(strData), Logger::LOG_DATA);
        }
        else
        {
            LOG("Could not create Pixel Shader " + std::string(shaderPath) + " for Mesh " + std::string(meshName), Logger::LOG_ERR);
            mesh.pPixelShader[shaderLevel] = nullptr;
        }
    }
}

namespace Legacy
{
    void LoadMD3Shaders(
        IDirect3DDevice9* device,
        MD3MESH* meshes,
        int meshCount,
        const char* shaderListPath,
        const char* shaderRootPath)
    {
        LOGFUNC("LoadMD3Shaders()");

        if (!device || !meshes || meshCount <= 0 || CheckFile(shaderListPath) == 0)
            return;

        FILE* shaderFile = fopen(shaderListPath, "rt");
        if (!shaderFile)
            return;

        CHAR cMeshName[16];
        CHAR cShaderName[256];
        CHAR strToken[1024];
        CHAR cPath[1024];
        CHAR pcTextLine[1024];

        while (!feof(shaderFile))
        {
            BOOL bMesh = FALSE;
            BOOL bShader = FALSE;
            BOOL bShaderLevel = FALSE;
            int iShaderLevel = 0;

            pcTextLine[0] = '\0';
            fscanf(shaderFile, "%s", pcTextLine);

            while (pcTextLine && pcTextLine[0])
            {
                ParseTextLine(pcTextLine, strToken);

                if (bMesh == 0 && bShaderLevel == 0)
                {
                    strcpy(cMeshName, _strlwr(strToken));
                    bMesh = TRUE;
                }
                else if ((strstr(_strlwr(strToken), ".vsh") || strstr(strToken, ".psh")) && bMesh == TRUE && bShader == FALSE)
                {
                    strcpy(cShaderName, strToken);
                    bShader = TRUE;
                }
                else if ((strstr(strToken, "shaderlevel")) && bShaderLevel == FALSE && bMesh == TRUE && bShader == TRUE)
                {
                    iShaderLevel = ParseNumber(strToken);
                    bShaderLevel = TRUE;
                }
            }

            if (!bShader || iShaderLevel < 0 || iShaderLevel >= MAXSHADERLEVELINMESH)
                continue;

            BuildShaderFilePath(cPath, sizeof(cPath), cShaderName, shaderRootPath);

            const int meshIndex = FindMeshIndex(meshes, meshCount, cMeshName);
            if (meshIndex < 0)
            {
                LOG("Didn't find a Mesh with the Name: " + std::string(cMeshName), Logger::LOG_ERR);
                continue;
            }

            if (strstr(cShaderName, ".vsh"))
                LoadVertexShader(device, meshes[meshIndex], iShaderLevel, cPath, cMeshName);
            else if (strstr(cShaderName, ".psh"))
                LoadPixelShader(device, meshes[meshIndex], iShaderLevel, cPath, cMeshName);
        }

        fclose(shaderFile);
    }
}
