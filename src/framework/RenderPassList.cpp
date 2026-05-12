#include "RenderPassList.h"

#include "IRenderPass.h"

namespace Framework
{
    void RenderPassList::Add(IRenderPass& pass)
    {
        m_passes.push_back(&pass);
    }

    void RenderPassList::Clear()
    {
        m_passes.clear();
        m_created = false;
    }

    bool RenderPassList::CreateDeviceResources(IDirect3DDevice9* device, const FileSystem& files)
    {
        if (m_created)
            return true;

        for (IRenderPass* pass : m_passes)
        {
            if (!pass)
                continue;

            if (!pass->CreateDeviceResources(device, files))
            {
                DestroyDeviceResources();
                return false;
            }
        }

        m_created = true;
        return true;
    }

    void RenderPassList::DestroyDeviceResources()
    {
        // Destroy in reverse creation order.
        for (auto it = m_passes.rbegin(); it != m_passes.rend(); ++it)
        {
            if (*it)
                (*it)->DestroyDeviceResources();
        }

        m_created = false;
    }

    void RenderPassList::Render(RenderContext& context)
    {
        for (IRenderPass* pass : m_passes)
        {
            if (pass)
                pass->Render(context);
        }
    }
}
