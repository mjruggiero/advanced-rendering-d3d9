#include "RenderContext.h"

namespace Framework
{
    RenderContext::RenderContext(IDirect3DDevice9* device, int width, int height)
        : m_device(device)
        , m_width(width > 0 ? width : 1)
        , m_height(height > 0 ? height : 1)
    {
    }

    float RenderContext::AspectRatio() const
    {
        return static_cast<float>(m_width) / static_cast<float>(m_height);
    }

    void RenderContext::SetWireframe(bool enabled) const
    {
        if (!m_device)
            return;

        m_device->SetRenderState(
            D3DRS_FILLMODE,
            enabled ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
    }

    void RenderContext::SetDefault3DState() const
    {
        if (!m_device)
            return;

        m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }

    void RenderContext::SetDefault2DState() const
    {
        if (!m_device)
            return;

        m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    }
}
