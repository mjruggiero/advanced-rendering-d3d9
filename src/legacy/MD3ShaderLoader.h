#pragma once

#include "MD3Types.h"

namespace Legacy
{
    void LoadMD3Shaders(
        IDirect3DDevice9* device,
        MD3MESH* meshes,
        int meshCount,
        const char* shaderListPath,
        const char* shaderRootPath);
}
