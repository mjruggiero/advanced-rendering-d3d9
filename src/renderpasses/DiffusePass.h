#pragma once

#include "../framework/D3D9ShaderProgram.h"
#include "../framework/IRenderPass.h"

namespace RenderPasses
{
    // First final-project render-pass lane.
    //
    // This pass intentionally does not draw legacy MD3 geometry yet. It gives new
    // advanced-rendering shader work a clean home outside MD3Model.cpp.
    class DiffusePass final : public Framework::IRenderPass
    {
    public:
        bool CreateDeviceResources(IDirect3DDevice9* device, const Framework::FileSystem& files) override;
        void DestroyDeviceResources() override;
        void Render(Framework::RenderContext& context) override;

        bool IsReady() const { return m_shader.IsLoaded(); }

    private:
        Framework::D3D9ShaderProgram m_shader;
    };
}
