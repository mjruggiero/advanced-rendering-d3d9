#include "D3D9RenderTarget.h"

namespace Framework
{
    D3D9RenderTarget::~D3D9RenderTarget()
    {
        Release();
    }

    D3D9RenderTarget::D3D9RenderTarget(D3D9RenderTarget&& other) noexcept
    {
        MoveFrom(other);
    }

    D3D9RenderTarget& D3D9RenderTarget::operator=(D3D9RenderTarget&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            MoveFrom(other);
        }

        return *this;
    }

    void D3D9RenderTarget::MoveFrom(D3D9RenderTarget& other) noexcept
    {
        m_width = other.m_width;
        m_height = other.m_height;
        m_colorFormat = other.m_colorFormat;
        m_depthStencilFormat = other.m_depthStencilFormat;

        m_texture = std::move(other.m_texture);
        m_surface = std::move(other.m_surface);
        m_depthStencilSurface = std::move(other.m_depthStencilSurface);

        m_savedRenderTarget = std::move(other.m_savedRenderTarget);
        m_savedDepthStencilSurface = std::move(other.m_savedDepthStencilSurface);
        m_savedViewport = other.m_savedViewport;
        m_active = other.m_active;

        other.m_width = 0;
        other.m_height = 0;
        other.m_colorFormat = D3DFMT_UNKNOWN;
        other.m_depthStencilFormat = D3DFMT_UNKNOWN;
        other.m_savedViewport = {};
        other.m_active = false;
    }

    bool D3D9RenderTarget::Create(
        IDirect3DDevice9* device,
        int width,
        int height,
        D3DFORMAT colorFormat,
        bool createDepthStencil,
        D3DFORMAT depthStencilFormat)
    {
        Release();

        if (!device || width <= 0 || height <= 0)
            return false;

        m_width = width;
        m_height = height;
        m_colorFormat = colorFormat;
        m_depthStencilFormat = createDepthStencil ? depthStencilFormat : D3DFMT_UNKNOWN;

        HRESULT hr = device->CreateTexture(
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            1,
            D3DUSAGE_RENDERTARGET,
            colorFormat,
            D3DPOOL_DEFAULT,
            m_texture.Put(),
            nullptr);

        if (FAILED(hr))
        {
            Release();
            return false;
        }

        hr = m_texture->GetSurfaceLevel(0, m_surface.Put());
        if (FAILED(hr))
        {
            Release();
            return false;
        }

        if (createDepthStencil)
        {
            hr = device->CreateDepthStencilSurface(
                static_cast<UINT>(width),
                static_cast<UINT>(height),
                depthStencilFormat,
                D3DMULTISAMPLE_NONE,
                0,
                TRUE,
                m_depthStencilSurface.Put(),
                nullptr);

            if (FAILED(hr))
            {
                Release();
                return false;
            }
        }

        return true;
    }

    void D3D9RenderTarget::Release()
    {
        m_active = false;
        m_savedViewport = {};

        m_savedRenderTarget.Reset();
        m_savedDepthStencilSurface.Reset();

        m_depthStencilSurface.Reset();
        m_surface.Reset();
        m_texture.Reset();

        m_width = 0;
        m_height = 0;
        m_colorFormat = D3DFMT_UNKNOWN;
        m_depthStencilFormat = D3DFMT_UNKNOWN;
    }

    bool D3D9RenderTarget::Begin(
        IDirect3DDevice9* device,
        D3DCOLOR clearColor,
        bool clearColorBuffer,
        bool clearDepthBuffer)
    {
        if (!device || !m_surface || m_active)
            return false;

        HRESULT hr = device->GetRenderTarget(0, m_savedRenderTarget.Put());
        if (FAILED(hr))
            return false;

        hr = device->GetDepthStencilSurface(m_savedDepthStencilSurface.Put());
        if (FAILED(hr))
        {
            // Some devices/passes may not have a depth surface currently bound.
            // Treat this as non-fatal and restore to nullptr in End().
            m_savedDepthStencilSurface.Reset();
        }

        hr = device->GetViewport(&m_savedViewport);
        if (FAILED(hr))
        {
            m_savedRenderTarget.Reset();
            m_savedDepthStencilSurface.Reset();
            return false;
        }

        hr = device->SetRenderTarget(0, m_surface.Get());
        if (FAILED(hr))
        {
            m_savedRenderTarget.Reset();
            m_savedDepthStencilSurface.Reset();
            return false;
        }

        hr = device->SetDepthStencilSurface(m_depthStencilSurface.Get());
        if (FAILED(hr))
        {
            device->SetRenderTarget(0, m_savedRenderTarget.Get());
            m_savedRenderTarget.Reset();
            m_savedDepthStencilSurface.Reset();
            return false;
        }

        D3DVIEWPORT9 viewport = {};
        viewport.X = 0;
        viewport.Y = 0;
        viewport.Width = static_cast<DWORD>(m_width);
        viewport.Height = static_cast<DWORD>(m_height);
        viewport.MinZ = 0.0f;
        viewport.MaxZ = 1.0f;
        device->SetViewport(&viewport);

        DWORD clearFlags = 0;
        if (clearColorBuffer)
            clearFlags |= D3DCLEAR_TARGET;

        if (clearDepthBuffer && m_depthStencilSurface)
            clearFlags |= D3DCLEAR_ZBUFFER;

        if (clearFlags != 0)
            device->Clear(0, nullptr, clearFlags, clearColor, 1.0f, 0);

        m_active = true;
        return true;
    }

    void D3D9RenderTarget::End(IDirect3DDevice9* device)
    {
        if (!device || !m_active)
            return;

        device->SetRenderTarget(0, m_savedRenderTarget.Get());
        device->SetDepthStencilSurface(m_savedDepthStencilSurface.Get());
        device->SetViewport(&m_savedViewport);

        m_savedRenderTarget.Reset();
        m_savedDepthStencilSurface.Reset();
        m_savedViewport = {};
        m_active = false;
    }

    bool D3D9RenderTarget::IsValid() const
    {
        return m_texture && m_surface && m_width > 0 && m_height > 0;
    }

    float D3D9RenderTarget::AspectRatio() const
    {
        if (m_height == 0)
            return 1.0f;

        return static_cast<float>(m_width) / static_cast<float>(m_height);
    }
}
