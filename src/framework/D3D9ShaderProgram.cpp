#include "D3D9ShaderProgram.h"

#include <cstdio>

namespace Framework
{
    namespace
    {
        void DebugWrite(const char* text)
        {
            if (text)
                OutputDebugStringA(text);
        }

        std::string NarrowPath(const std::filesystem::path& path)
        {
            return path.string();
        }
    }

    DWORD D3D9ShaderProgram::DefaultAssembleFlags()
    {
        DWORD flags = 0;

#if defined(_DEBUG) || defined(DEBUG)
        flags |= D3DXSHADER_DEBUG;
#endif

        return flags;
    }

    bool D3D9ShaderProgram::Load(
        IDirect3DDevice9* device,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& pixelShaderPath,
        DWORD assembleFlags)
    {
        Release();

        if (!device)
            return false;

        m_vertexShaderPath = vertexShaderPath;
        m_pixelShaderPath = pixelShaderPath;

        if (!LoadVertexShader(device, vertexShaderPath, assembleFlags))
        {
            Release();
            return false;
        }

        if (!LoadPixelShader(device, pixelShaderPath, assembleFlags))
        {
            Release();
            return false;
        }

        return true;
    }

    void D3D9ShaderProgram::Release()
    {
        m_vertexShader.Reset();
        m_pixelShader.Reset();
        m_vertexShaderPath.clear();
        m_pixelShaderPath.clear();
    }

    void D3D9ShaderProgram::Bind(IDirect3DDevice9* device) const
    {
        if (!device)
            return;

        device->SetVertexShader(m_vertexShader.Get());
        device->SetPixelShader(m_pixelShader.Get());
    }

    void D3D9ShaderProgram::Unbind(IDirect3DDevice9* device)
    {
        if (!device)
            return;

        device->SetVertexShader(nullptr);
        device->SetPixelShader(nullptr);
    }

    bool D3D9ShaderProgram::IsLoaded() const
    {
        return m_vertexShader && m_pixelShader;
    }

    bool D3D9ShaderProgram::LoadVertexShader(
        IDirect3DDevice9* device,
        const std::filesystem::path& path,
        DWORD assembleFlags)
    {
        ComPtr<ID3DXBuffer> code;
        ComPtr<ID3DXBuffer> errors;

        const std::string shaderPath = NarrowPath(path);

        const HRESULT assembleHr = D3DXAssembleShaderFromFileA(
            shaderPath.c_str(),
            nullptr,
            nullptr,
            assembleFlags,
            code.Put(),
            errors.Put());

        if (FAILED(assembleHr))
        {
            LogShaderError("vertex", path, assembleHr, errors.Get());
            return false;
        }

        const HRESULT createHr = device->CreateVertexShader(
            reinterpret_cast<const DWORD*>(code->GetBufferPointer()),
            m_vertexShader.Put());

        if (FAILED(createHr))
        {
            LogShaderError("vertex-create", path, createHr, nullptr);
            return false;
        }

        return true;
    }

    bool D3D9ShaderProgram::LoadPixelShader(
        IDirect3DDevice9* device,
        const std::filesystem::path& path,
        DWORD assembleFlags)
    {
        ComPtr<ID3DXBuffer> code;
        ComPtr<ID3DXBuffer> errors;

        const std::string shaderPath = NarrowPath(path);

        const HRESULT assembleHr = D3DXAssembleShaderFromFileA(
            shaderPath.c_str(),
            nullptr,
            nullptr,
            assembleFlags,
            code.Put(),
            errors.Put());

        if (FAILED(assembleHr))
        {
            LogShaderError("pixel", path, assembleHr, errors.Get());
            return false;
        }

        const HRESULT createHr = device->CreatePixelShader(
            reinterpret_cast<const DWORD*>(code->GetBufferPointer()),
            m_pixelShader.Put());

        if (FAILED(createHr))
        {
            LogShaderError("pixel-create", path, createHr, nullptr);
            return false;
        }

        return true;
    }

    void D3D9ShaderProgram::LogShaderError(
        const char* stage,
        const std::filesystem::path& path,
        HRESULT hr,
        ID3DXBuffer* errors)
    {
        char buffer[2048] = {};
        const std::string shaderPath = NarrowPath(path);

        std::snprintf(
            buffer,
            sizeof(buffer),
            "[D3D9ShaderProgram] Failed to load %s shader '%s' hr=0x%08X\n",
            stage ? stage : "unknown",
            shaderPath.c_str(),
            static_cast<unsigned>(hr));
        DebugWrite(buffer);

        if (errors && errors->GetBufferPointer())
        {
            DebugWrite(static_cast<const char*>(errors->GetBufferPointer()));
            DebugWrite("\n");
        }
    }
}
