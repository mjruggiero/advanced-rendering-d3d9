#pragma once

struct IDirect3DDevice9;

namespace Framework
{
    class FileSystem;
    class RenderContext;

    // Small interface for new advanced-rendering passes.
    //
    // This is intentionally separate from the legacy MD3 renderer. Use it for new
    // final-project work such as diffuse, Cook-Torrance, shadow-map, or composite
    // passes without pushing more logic into CharacterApp or MD3.cpp.
    class IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;

        // Create D3D device resources owned by this pass, such as shaders,
        // declarations, textures, render targets, or static buffers.
        virtual bool CreateDeviceResources(
            IDirect3DDevice9* device,
            const FileSystem& files) = 0;

        // Destroy resources created by CreateDeviceResources().
        virtual void DestroyDeviceResources() = 0;

        // Render this pass. The framework owns Clear/BeginScene/EndScene/Present;
        // render passes should only set state, bind resources, and draw.
        virtual void Render(RenderContext& context) = 0;
    };
}
