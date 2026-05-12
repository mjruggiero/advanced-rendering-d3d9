#include "D3D9TextRenderer.h"

namespace Framework
{
    D3D9TextRenderer::~D3D9TextRenderer()
    {
        Release();
    }

    bool D3D9TextRenderer::Create(
        IDirect3DDevice9* device,
        const wchar_t* faceName,
        int normalHeight,
        int smallHeight,
        int weight)
    {
        Release();

        if (!device)
            return false;

        m_faceName = faceName ? faceName : L"Arial";
        m_normalHeight = normalHeight;
        m_smallHeight = smallHeight;
        m_weight = weight;

        if (!CreateFonts(device, m_faceName.c_str(), m_normalHeight, m_smallHeight, m_weight))
        {
            Release();
            return false;
        }

        HRESULT hr = D3DXCreateSprite(device, m_sprite.Put());
        if (FAILED(hr))
        {
            Release();
            return false;
        }

        return true;
    }

    bool D3D9TextRenderer::CreateFonts(
        IDirect3DDevice9* device,
        const wchar_t* faceName,
        int normalHeight,
        int smallHeight,
        int weight)
    {
        HRESULT hr = D3DXCreateFontW(
            device,
            normalHeight,
            0,
            weight,
            1,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            faceName,
            m_normalFont.Put());

        if (FAILED(hr))
            return false;

        hr = D3DXCreateFontW(
            device,
            smallHeight,
            0,
            weight,
            1,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            faceName,
            m_smallFont.Put());

        return SUCCEEDED(hr);
    }

    void D3D9TextRenderer::Release()
    {
        m_insideBegin = false;
        m_sprite.Reset();
        m_smallFont.Reset();
        m_normalFont.Reset();
    }

    void D3D9TextRenderer::DestroyResetResources()
    {
        if (m_normalFont.Get())
            m_normalFont->OnLostDevice();

        if (m_smallFont.Get())
            m_smallFont->OnLostDevice();

        // Sprite is a default-pool object. Recreate it on reset.
        m_sprite.Reset();
        m_insideBegin = false;
    }

    bool D3D9TextRenderer::CreateResetResources(IDirect3DDevice9* device)
    {
        if (!device)
            return false;

        if (m_normalFont.Get())
            m_normalFont->OnResetDevice();

        if (m_smallFont.Get())
            m_smallFont->OnResetDevice();

        if (!m_sprite.Get())
        {
            HRESULT hr = D3DXCreateSprite(device, m_sprite.Put());
            if (FAILED(hr))
                return false;
        }

        return true;
    }

    bool D3D9TextRenderer::Begin()
    {
        if (!m_sprite.Get() || m_insideBegin)
            return false;

        HRESULT hr = m_sprite->Begin(D3DXSPRITE_ALPHABLEND);
        if (FAILED(hr))
            return false;

        m_insideBegin = true;
        return true;
    }

    void D3D9TextRenderer::End()
    {
        if (!m_sprite.Get() || !m_insideBegin)
            return;

        m_sprite->End();
        m_insideBegin = false;
    }

    void D3D9TextRenderer::DrawLine(
        FontSize fontSize,
        int x,
        int y,
        D3DCOLOR color,
        const wchar_t* text)
    {
        ID3DXFont* font = SelectFont(fontSize);
        if (!font || !text)
            return;

        RECT rc = {};
        rc.left = x;
        rc.top = y;
        rc.right = 4096;
        rc.bottom = 4096;

        font->DrawTextW(
            m_sprite.Get(),
            text,
            -1,
            &rc,
            DT_NOCLIP,
            color);
    }

    ID3DXFont* D3D9TextRenderer::SelectFont(FontSize fontSize) const
    {
        switch (fontSize)
        {
        case FontSize::Normal:
            return m_normalFont.Get();

        case FontSize::Small:
            return m_smallFont.Get();

        default:
            return m_normalFont.Get();
        }
    }

    bool D3D9TextRenderer::IsValid() const
    {
        return m_normalFont.Get() != nullptr &&
               m_smallFont.Get() != nullptr &&
               m_sprite.Get() != nullptr;
    }
}
