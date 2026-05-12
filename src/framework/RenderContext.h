#pragma once

#include <d3d9.h>

namespace Framework
{
    class RenderContext
    {
    public:
        RenderContext(IDirect3DDevice9* device, int width, int height);

        IDirect3DDevice9* Device() const { return m_device; }

        int Width() const { return m_width; }
        int Height() const { return m_height; }
        float AspectRatio() const;

        void SetWireframe(bool enabled) const;
        void SetDefault3DState() const;
        void SetDefault2DState() const;

    private:
        IDirect3DDevice9* m_device = nullptr;
        int m_width = 1;
        int m_height = 1;
    };
}
