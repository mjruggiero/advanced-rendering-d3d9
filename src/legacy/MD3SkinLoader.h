#pragma once

#include "MD3Types.h"

namespace Legacy
{
    void LoadMD3Skins(
        IDirect3DDevice9* device,
        MD3MESH* meshes,
        int meshCount,
        const char* skinFilePath,
        const char* imageRootPath);
}
