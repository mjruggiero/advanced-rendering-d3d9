#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <filesystem>
#include <string>

#include "ComPtr.h"

namespace Framework
{
    // Small wrapper for legacy D3D9 assembly shader pairs (.vsh/.psh).
    // This is intentionally simple so the recovered CharacterEngine code can keep
    // working while new advanced-rendering code has a cleaner place to load shaders.
    class D3D9ShaderProgram
    {
    public:
        D3D9ShaderProgram() = default;
        ~D3D9ShaderProgram() = default;

        D3D9ShaderProgram(const D3D9ShaderProgram&) = delete;
        D3D9ShaderProgram& operator=(const D3D9ShaderProgram&) = delete;

        D3D9ShaderProgram(D3D9ShaderProgram&&) noexcept = default;
        D3D9ShaderProgram& operator=(D3D9ShaderProgram&&) noexcept = default;

        bool Load(
            IDirect3DDevice9* device,
            const std::filesystem::path& vertexShaderPath,
            const std::filesystem::path& pixelShaderPath,
            DWORD assembleFlags = DefaultAssembleFlags());

        void Release();
        void Bind(IDirect3DDevice9* device) const;
        static void Unbind(IDirect3DDevice9* device);

        bool IsLoaded() const;

        IDirect3DVertexShader9* VertexShader() const { return m_vertexShader.Get(); }
        IDirect3DPixelShader9* PixelShader() const { return m_pixelShader.Get(); }

        const std::filesystem::path& VertexShaderPath() const { return m_vertexShaderPath; }
        const std::filesystem::path& PixelShaderPath() const { return m_pixelShaderPath; }

        static DWORD DefaultAssembleFlags();

    private:
        bool LoadVertexShader(
            IDirect3DDevice9* device,
            const std::filesystem::path& path,
            DWORD assembleFlags);

        bool LoadPixelShader(
            IDirect3DDevice9* device,
            const std::filesystem::path& path,
            DWORD assembleFlags);

        static void LogShaderError(
            const char* stage,
            const std::filesystem::path& path,
            HRESULT hr,
            ID3DXBuffer* errors);

        ComPtr<IDirect3DVertexShader9> m_vertexShader;
        ComPtr<IDirect3DPixelShader9> m_pixelShader;
        std::filesystem::path m_vertexShaderPath;
        std::filesystem::path m_pixelShaderPath;
    };
}
