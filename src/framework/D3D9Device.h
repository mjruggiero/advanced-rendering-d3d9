#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d9.h>

#include "ComPtr.h"

namespace Framework
{
    struct D3D9DeviceDesc
    {
        HWND hwnd = nullptr;
        int width = 800;
        int height = 600;
        bool windowed = true;
        bool enableDepthStencil = true;
        D3DFORMAT backBufferFormat = D3DFMT_UNKNOWN;
        D3DFORMAT depthStencilFormat = D3DFMT_D24S8;
        D3DMULTISAMPLE_TYPE multiSampleType = D3DMULTISAMPLE_NONE;
        DWORD behaviorFlags = 0;
    };

    class D3D9Device
    {
    public:
        bool Create(const D3D9DeviceDesc& desc);
        void Destroy();

        HRESULT TestCooperativeLevel() const;
        bool Reset(int width, int height);

        void Clear(D3DCOLOR color, float z = 1.0f, DWORD stencil = 0);
        bool BeginScene();
        void EndScene();
        void Present();

        IDirect3D9* D3D() const { return m_d3d.Get(); }
        IDirect3DDevice9* Device() const { return m_device.Get(); }
        const D3DPRESENT_PARAMETERS& PresentParameters() const { return m_pp; }

        int Width() const { return static_cast<int>(m_pp.BackBufferWidth); }
        int Height() const { return static_cast<int>(m_pp.BackBufferHeight); }
        bool IsWindowed() const { return m_pp.Windowed != FALSE; }

    private:
        bool ChooseBehaviorFlags(const D3D9DeviceDesc& desc, DWORD& outFlags) const;

        ComPtr<IDirect3D9> m_d3d;
        ComPtr<IDirect3DDevice9> m_device;
        D3DPRESENT_PARAMETERS m_pp{};
        D3D9DeviceDesc m_desc{};
    };
}
