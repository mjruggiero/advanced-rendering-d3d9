#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <filesystem>
#include <string>

#include "D3D9Device.h"
#include "FileSystem.h"
#include "GameTimer.h"
#include "InputState.h"
#include "RenderContext.h"

namespace Framework
{
    struct ApplicationConfig
    {
        std::wstring title = L"D3D9 Application";
        int width = 800;
        int height = 600;
        bool windowed = true;
        std::filesystem::path assetRoot = L".";
        std::filesystem::path logFile = L"app.log";
        D3DCOLOR clearColor = D3DCOLOR_XRGB(16, 16, 32);
    };

    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        int Run(HINSTANCE instance, int showCmd);

        HWND WindowHandle() const { return m_hwnd; }
        IDirect3DDevice9* Device() const { return m_device.Device(); }
        D3D9Device& DeviceContext() { return m_device; }
        const D3D9Device& DeviceContext() const { return m_device; }
        InputState& Input() { return m_input; }
        const InputState& Input() const { return m_input; }
        FileSystem& Files() { return m_files; }
        const FileSystem& Files() const { return m_files; }
        const ApplicationConfig& Config() const { return m_config; }

        int BackBufferWidth() const { return m_device.Width(); }
        int BackBufferHeight() const { return m_device.Height(); }
        float BackBufferAspectRatio() const
        {
            return m_device.Height() > 0
                ? static_cast<float>(m_device.Width()) / static_cast<float>(m_device.Height())
                : 1.0f;
        }

    protected:
        virtual ApplicationConfig GetConfig() const { return {}; }

        // Friendly application lifecycle.
        //
        // Application code should think in these terms. The framework still
        // handles the D3D9-specific device/reset dance internally and exposes
        // resource hooks only where D3D9 requires them.
        virtual bool Initialize() { return true; }
        virtual void Shutdown() {}

        virtual bool CreateDeviceResources() { return true; }
        virtual void DestroyDeviceResources() {}
        virtual bool CreateResetResources() { return true; }
        virtual void DestroyResetResources() {}

        virtual void Update(float deltaSeconds) {}
        virtual void Render(RenderContext& context) { (void)context; }

        virtual void OnKeyDown(uint32_t vk) {}
        virtual void OnKeyUp(uint32_t vk) {}
        virtual void OnMouseButtonDown(uint32_t button, int x, int y) {}
        virtual void OnMouseButtonUp(uint32_t button, int x, int y) {}
        virtual void OnMouseMove(int x, int y, int dx, int dy) {}
        virtual void OnMouseWheel(int delta) {}
        virtual void OnResize(int width, int height) {}
        virtual LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& handled)
        {
            handled = false;
            return 0;
        }

        void RequestQuit(int exitCode = 0);

    private:
        bool InitializeFramework(HINSTANCE instance, int showCmd);
        void ShutdownFramework();
        void MainLoop();
        void RenderFrame();
        void HandleDeviceLost();
        bool ResetDeviceToClientSize();
        bool CreateMainWindow(HINSTANCE instance, int showCmd);
        void DestroyMainWindow();
        std::filesystem::path GetExecutablePath() const;

        static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        ApplicationConfig m_config;
        HINSTANCE m_instance = nullptr;
        HWND m_hwnd = nullptr;
        bool m_running = false;
        bool m_initialized = false;
        bool m_deviceObjectsCreated = false;
        bool m_resetObjectsCreated = false;
        bool m_minimized = false;
        int m_exitCode = 0;

        D3D9Device m_device;
        GameTimer m_timer;
        InputState m_input;
        FileSystem m_files;
    };
}
