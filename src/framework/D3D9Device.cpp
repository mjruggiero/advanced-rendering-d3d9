#include "D3D9Device.h"
#include "FrameworkLog.h"

#include <algorithm>

namespace Framework
{
    bool D3D9Device::Create(const D3D9DeviceDesc& desc)
    {
        Destroy();
        m_desc = desc;

        m_d3d.Attach(Direct3DCreate9(D3D_SDK_VERSION));
        if (!m_d3d)
        {
            Framework::FrameworkLog::WriteError("Direct3DCreate9 failed");
            return false;
        }

        m_pp = {};
        m_pp.BackBufferWidth = static_cast<UINT>(std::max(desc.width, 1));
        m_pp.BackBufferHeight = static_cast<UINT>(std::max(desc.height, 1));
        m_pp.BackBufferFormat = desc.windowed ? D3DFMT_UNKNOWN : desc.backBufferFormat;
        m_pp.BackBufferCount = 1;
        m_pp.MultiSampleType = desc.multiSampleType;
        m_pp.MultiSampleQuality = 0;
        m_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        m_pp.hDeviceWindow = desc.hwnd;
        m_pp.Windowed = desc.windowed ? TRUE : FALSE;
        m_pp.EnableAutoDepthStencil = desc.enableDepthStencil ? TRUE : FALSE;
        m_pp.AutoDepthStencilFormat = desc.depthStencilFormat;
        m_pp.Flags = 0;
        m_pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        m_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        DWORD behaviorFlags = 0;
        if (desc.behaviorFlags != 0)
            behaviorFlags = desc.behaviorFlags;
        else if (!ChooseBehaviorFlags(desc, behaviorFlags))
            return false;

        const HRESULT hr = m_d3d->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            desc.hwnd,
            behaviorFlags,
            &m_pp,
            m_device.Put());

        if (FAILED(hr))
        {
            //Framework::FrameworkLog::WriteError("IDirect3D9::CreateDevice failed. hr=0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        //Framework::FrameworkLog::WriteInfo("Created D3D9 device: %dx%d windowed=%d", Width(), Height(), IsWindowed() ? 1 : 0);
        return true;
    }

    void D3D9Device::Destroy()
    {
        m_device.Reset();
        m_d3d.Reset();
        m_pp = {};
    }

    HRESULT D3D9Device::TestCooperativeLevel() const
    {
        return m_device ? m_device->TestCooperativeLevel() : D3DERR_INVALIDCALL;
    }

    bool D3D9Device::Reset(int width, int height)
    {
        if (!m_device)
            return false;

        m_pp.BackBufferWidth = static_cast<UINT>(std::max(width, 1));
        m_pp.BackBufferHeight = static_cast<UINT>(std::max(height, 1));

        const HRESULT hr = m_device->Reset(&m_pp);
        if (FAILED(hr))
        {
            //LOG_WARNING("IDirect3DDevice9::Reset failed. hr=0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        //Framework::FrameworkLog::WriteInfo("Reset D3D9 device: %dx%d", Width(), Height());
        return true;
    }

    void D3D9Device::Clear(D3DCOLOR color, float z, DWORD stencil)
    {
        if (!m_device)
            return;

        DWORD flags = D3DCLEAR_TARGET;
        if (m_pp.EnableAutoDepthStencil)
            flags |= D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;

        m_device->Clear(0, nullptr, flags, color, z, stencil);
    }

    bool D3D9Device::BeginScene()
    {
        return m_device && SUCCEEDED(m_device->BeginScene());
    }

    void D3D9Device::EndScene()
    {
        if (m_device)
            m_device->EndScene();
    }

    void D3D9Device::Present()
    {
        if (m_device)
            m_device->Present(nullptr, nullptr, nullptr, nullptr);
    }

    bool D3D9Device::ChooseBehaviorFlags(const D3D9DeviceDesc& desc, DWORD& outFlags) const
    {
        D3DCAPS9 caps{};
        const HRESULT hr = m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
        if (FAILED(hr))
        {
            //LOG_ERROR("GetDeviceCaps failed. hr=0x%08X", static_cast<unsigned>(hr));
            return false;
        }

        outFlags = 0;
        if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
            outFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        else
            outFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

        // Vertex/pixel shader demos generally behave better with FPU preservation off unless legacy code needs it.
        outFlags |= D3DCREATE_MULTITHREADED;
        return true;
    }
}
