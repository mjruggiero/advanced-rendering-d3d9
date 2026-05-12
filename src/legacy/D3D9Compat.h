#pragma once

// D3D9 legacy compatibility helpers.
//
// This header is intentionally quarantined under src/legacy. It replaces the
// old DXUT sample header dxstdafx.h without carrying forward DXUT itself.
//
// New framework/app code should not include this file. Prefer ComPtr and
// explicit HRESULT handling there. The macros below exist only to keep the
// recovered legacy renderer readable while it is being cleaned up.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(DEBUG) || defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

namespace LegacyD3D9Compat
{
    template <typename T>
    inline void SafeRelease(T*& ptr)
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    template <typename T>
    inline void SafeDelete(T*& ptr)
    {
        delete ptr;
        ptr = nullptr;
    }

    template <typename T>
    inline void SafeDeleteArray(T*& ptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

// Legacy compatibility macros.
// These intentionally preserve the old sample-code style in legacy files.
// Do not use these in new framework code.
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) LegacyD3D9Compat::SafeRelease(p)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) LegacyD3D9Compat::SafeDelete(p)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) LegacyD3D9Compat::SafeDeleteArray(p)
#endif

#ifndef V
#define V(x) do { hr = (x); } while (0)
#endif

#ifndef V_RETURN
#define V_RETURN(x) do { hr = (x); if (FAILED(hr)) { return hr; } } while (0)
#endif
