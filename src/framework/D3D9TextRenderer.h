#pragma once

#include <d3dx9.h>
#include <string>

#include "ComPtr.h"

namespace Framework
{
    // Small ID3DXFont / ID3DXSprite wrapper.
    //
    // This gives HUD and debug text a clean home without dragging DXUT-style
    // CDXUTTextHelper back into the project.
    class D3D9TextRenderer
    {
    public:
        enum class FontSize
        {
            Normal,
            Small
        };

        D3D9TextRenderer() = default;
        ~D3D9TextRenderer();

        D3D9TextRenderer(const D3D9TextRenderer&) = delete;
        D3D9TextRenderer& operator=(const D3D9TextRenderer&) = delete;

        bool Create(
            IDirect3DDevice9* device,
            const wchar_t* faceName = L"Arial",
            int normalHeight = 15,
            int smallHeight = 14,
            int weight = FW_BOLD);

        void Release();

        void DestroyResetResources();
        bool CreateResetResources(IDirect3DDevice9* device);

        bool Begin();
        void End();

        void DrawLine(
            FontSize fontSize,
            int x,
            int y,
            D3DCOLOR color,
            const wchar_t* text);

        ID3DXFont* NormalFont() const { return m_normalFont.Get(); }
        ID3DXFont* SmallFont() const { return m_smallFont.Get(); }
        ID3DXSprite* Sprite() const { return m_sprite.Get(); }

        bool IsValid() const;

    private:
        bool CreateFonts(
            IDirect3DDevice9* device,
            const wchar_t* faceName,
            int normalHeight,
            int smallHeight,
            int weight);

        ID3DXFont* SelectFont(FontSize fontSize) const;

        ComPtr<ID3DXFont> m_normalFont;
        ComPtr<ID3DXFont> m_smallFont;
        ComPtr<ID3DXSprite> m_sprite;

        std::wstring m_faceName = L"Arial";
        int m_normalHeight = 15;
        int m_smallHeight = 14;
        int m_weight = FW_BOLD;
        bool m_insideBegin = false;
    };
}
