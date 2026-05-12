#include "D3D9StateBlock.h"

namespace Framework
{
    D3D9ScopedStateBlock::D3D9ScopedStateBlock(IDirect3DDevice9* device)
        : m_device(device)
    {
        if (!m_device)
            return;

        // D3DSBT_ALL is heavy, but it is very useful while stabilizing legacy
        // rendering code and isolating new render passes.
        HRESULT hr = m_device->CreateStateBlock(D3DSBT_ALL, m_stateBlock.Put());
        if (SUCCEEDED(hr) && m_stateBlock.Get())
            m_stateBlock->Capture();
    }

    D3D9ScopedStateBlock::~D3D9ScopedStateBlock()
    {
        Restore();
    }

    void D3D9ScopedStateBlock::Restore()
    {
        if (m_restored)
            return;

        if (m_stateBlock.Get())
            m_stateBlock->Apply();

        m_restored = true;
    }

    D3D9ScopedRenderState::D3D9ScopedRenderState(
        IDirect3DDevice9* device,
        D3DRENDERSTATETYPE state,
        DWORD newValue)
        : m_device(device)
        , m_state(state)
    {
        if (!m_device)
            return;

        HRESULT hr = m_device->GetRenderState(m_state, &m_oldValue);
        if (FAILED(hr))
            return;

        hr = m_device->SetRenderState(m_state, newValue);
        if (FAILED(hr))
            return;

        m_valid = true;
    }

    D3D9ScopedRenderState::~D3D9ScopedRenderState()
    {
        Restore();
    }

    void D3D9ScopedRenderState::Restore()
    {
        if (m_restored)
            return;

        if (m_valid && m_device)
            m_device->SetRenderState(m_state, m_oldValue);

        m_restored = true;
    }
}
