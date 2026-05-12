#include "DiffusePass.h"

#include "../framework/FileSystem.h"
#include "../framework/RenderContext.h"

namespace RenderPasses
{
    bool DiffusePass::CreateDeviceResources(IDirect3DDevice9* device, const Framework::FileSystem& files)
    {
        return m_shader.Load(
            device,
            files.Resolve(L"shaders/diffuse.vsh"),
            files.Resolve(L"shaders/diffuse.psh"));
    }

    void DiffusePass::DestroyDeviceResources()
    {
        m_shader.Release();
    }

    void DiffusePass::Render(Framework::RenderContext& context)
    {
        if (!m_shader.IsLoaded())
            return;

        m_shader.Bind(context.Device());

        // Geometry submission intentionally stays outside this skeleton for now.
        // Future final-project passes can bind constants/textures here, then draw
        // either a mesh, full-screen primitive, or scene object supplied by the app.

        Framework::D3D9ShaderProgram::Unbind(context.Device());
    }
}
