#pragma once

#include <d3d9.h>

#include "ComPtr.h"

namespace Framework
{
    // Small D3D9 render-target texture wrapper.
    //
    // This is intended for future advanced-rendering passes such as:
    // - shadow maps
    // - offscreen lighting buffers
    // - post-process source textures
    //
    // It does not call BeginScene/EndScene. The framework still owns the frame.
    class D3D9RenderTarget
    {
    public:
        D3D9RenderTarget() = default;
        ~D3D9RenderTarget();

        D3D9RenderTarget(const D3D9RenderTarget&) = delete;
        D3D9RenderTarget& operator=(const D3D9RenderTarget&) = delete;

        D3D9RenderTarget(D3D9RenderTarget&& other) noexcept;
        D3D9RenderTarget& operator=(D3D9RenderTarget&& other) noexcept;

        bool Create(
            IDirect3DDevice9* device,
            int width,
            int height,
            D3DFORMAT colorFormat = D3DFMT_A8R8G8B8,
            bool createDepthStencil = true,
            D3DFORMAT depthStencilFormat = D3DFMT_D24S8);

        void Release();

        bool Begin(
            IDirect3DDevice9* device,
            D3DCOLOR clearColor = D3DCOLOR_XRGB(0, 0, 0),
            bool clearColorBuffer = true,
            bool clearDepthBuffer = true);

        void End(IDirect3DDevice9* device);

        bool IsValid() const;

        int Width() const { return m_width; }
        int Height() const { return m_height; }
        float AspectRatio() const;

        IDirect3DTexture9* Texture() const { return m_texture.Get(); }
        IDirect3DSurface9* Surface() const { return m_surface.Get(); }
        IDirect3DSurface9* DepthStencilSurface() const { return m_depthStencilSurface.Get(); }

    private:
        void MoveFrom(D3D9RenderTarget& other) noexcept;

        int m_width = 0;
        int m_height = 0;
        D3DFORMAT m_colorFormat = D3DFMT_UNKNOWN;
        D3DFORMAT m_depthStencilFormat = D3DFMT_UNKNOWN;

        ComPtr<IDirect3DTexture9> m_texture;
        ComPtr<IDirect3DSurface9> m_surface;
        ComPtr<IDirect3DSurface9> m_depthStencilSurface;

        ComPtr<IDirect3DSurface9> m_savedRenderTarget;
        ComPtr<IDirect3DSurface9> m_savedDepthStencilSurface;
        D3DVIEWPORT9 m_savedViewport = {};
        bool m_active = false;
    };
}
