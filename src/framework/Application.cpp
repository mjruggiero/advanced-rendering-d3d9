#include "Application.h"
#include "FrameworkLog.h"

#include <algorithm>
#include <cassert>
#include <windowsx.h>

namespace Framework
{
    int Application::Run(HINSTANCE instance, int showCmd)
    {
        if (!InitializeFramework(instance, showCmd))
        {
            ShutdownFramework();
            return -1;
        }

        MainLoop();
        ShutdownFramework();
        return m_exitCode;
    }

    bool Application::InitializeFramework(HINSTANCE instance, int showCmd)
    {
        m_instance = instance;
        m_config = GetConfig();

        const auto exePath = GetExecutablePath();
        m_files.Initialize(exePath, m_config.assetRoot);
        //Logger::Instance().Open(m_files.ExecutableDirectory() / m_config.logFile);
        Framework::FrameworkLog::WriteInfo("Application starting");

        if (!CreateMainWindow(instance, showCmd))
            return false;

        if (!Initialize())
            return false;
        m_initialized = true;

        RECT rc{};
        GetClientRect(m_hwnd, &rc);

        D3D9DeviceDesc deviceDesc{};
        deviceDesc.hwnd = m_hwnd;
        deviceDesc.width = std::max<LONG>(rc.right - rc.left, 1);
        deviceDesc.height = std::max<LONG>(rc.bottom - rc.top, 1);
        deviceDesc.windowed = m_config.windowed;

        if (!m_device.Create(deviceDesc))
            return false;

        if (!CreateDeviceResources())
            return false;
        m_deviceObjectsCreated = true;

        if (!CreateResetResources())
            return false;
        m_resetObjectsCreated = true;

        m_timer.Reset();
        m_running = true;
        return true;
    }

    void Application::ShutdownFramework()
    {
        if (m_resetObjectsCreated)
        {
            DestroyResetResources();
            m_resetObjectsCreated = false;
        }

        if (m_deviceObjectsCreated)
        {
            DestroyDeviceResources();
            m_deviceObjectsCreated = false;
        }

        if (m_initialized)
        {
            Shutdown();
            m_initialized = false;
        }

        m_device.Destroy();
        DestroyMainWindow();

        Framework::FrameworkLog::WriteInfo("Application shutdown");
        //Logger::Instance().Close();
    }

    void Application::MainLoop()
    {
        MSG msg{};
        while (m_running)
        {
            m_input.BeginFrame();

            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_running = false;
                    m_exitCode = static_cast<int>(msg.wParam);
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!m_running)
                break;

            m_timer.Tick();

            if (!m_minimized)
            {
                Update(m_timer.DeltaSeconds());
                RenderFrame();
            }
            else
            {
                Sleep(16);
            }
        }
    }

    void Application::RenderFrame()
    {
        const HRESULT cooperative = m_device.TestCooperativeLevel();
        if (cooperative == D3DERR_DEVICELOST || cooperative == D3DERR_DEVICENOTRESET)
        {
            HandleDeviceLost();
            return;
        }

        IDirect3DDevice9* device = m_device.Device();
        if (!device)
            return;

        m_device.Clear(m_config.clearColor);

        if (m_device.BeginScene())
        {
            RenderContext context(device, m_device.Width(), m_device.Height());
            Render(context);
            m_device.EndScene();
        }

        m_device.Present();
    }

    void Application::HandleDeviceLost()
    {
        const HRESULT cooperative = m_device.TestCooperativeLevel();
        if (cooperative == D3DERR_DEVICELOST)
        {
            Sleep(50);
            return;
        }

        if (cooperative == D3DERR_DEVICENOTRESET)
        {
            if (m_resetObjectsCreated)
            {
                DestroyResetResources();
                m_resetObjectsCreated = false;
            }

            if (ResetDeviceToClientSize())
            {
                if (CreateResetResources())
                    m_resetObjectsCreated = true;
                else
                    RequestQuit(-2);
            }
        }
    }

    bool Application::ResetDeviceToClientSize()
    {
        RECT rc{};
        GetClientRect(m_hwnd, &rc);
        const int width = std::max<LONG>(rc.right - rc.left, 1);
        const int height = std::max<LONG>(rc.bottom - rc.top, 1);
        return m_device.Reset(width, height);
    }

    bool Application::CreateMainWindow(HINSTANCE instance, int showCmd)
    {
        const wchar_t* className = L"StandaloneD3D9ApplicationWindow";

        WNDCLASSEX wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = &Application::StaticWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = className;
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        RegisterClassEx(&wc);

        RECT windowRect{ 0, 0, m_config.width, m_config.height };
        const DWORD style = WS_OVERLAPPEDWINDOW;
        AdjustWindowRect(&windowRect, style, FALSE);

        m_hwnd = CreateWindowEx(
            0,
            className,
            m_config.title.c_str(),
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            instance,
            this);

        if (!m_hwnd)
        {
            Framework::FrameworkLog::WriteError("CreateWindowEx failed");
            return false;
        }

        ShowWindow(m_hwnd, showCmd);
        UpdateWindow(m_hwnd);
        return true;
    }

    void Application::DestroyMainWindow()
    {
        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    void Application::RequestQuit(int exitCode)
    {
        m_exitCode = exitCode;
        m_running = false;
        PostQuitMessage(exitCode);
    }

    std::filesystem::path Application::GetExecutablePath() const
    {
        wchar_t path[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return path;
    }

    LRESULT CALLBACK Application::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Application* app = nullptr;

        if (msg == WM_NCCREATE)
        {
            auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<Application*>(createStruct->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        }
        else
        {
            app = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (app)
            return app->WndProc(hwnd, msg, wParam, lParam);

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        bool handled = false;
        const LRESULT customResult = OnMessage(hwnd, msg, wParam, lParam, handled);
        if (handled)
            return customResult;

        switch (msg)
        {
        case WM_CLOSE:
            RequestQuit(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(m_exitCode);
            return 0;

        case WM_SIZE:
        {
            m_minimized = (wParam == SIZE_MINIMIZED);
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            OnResize(width, height);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if ((lParam & (1 << 30)) == 0)
                OnKeyDown(static_cast<uint32_t>(wParam));
            m_input.SetKey(static_cast<uint32_t>(wParam), true);
            if (wParam == VK_ESCAPE)
                RequestQuit(0);
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            m_input.SetKey(static_cast<uint32_t>(wParam), false);
            OnKeyUp(static_cast<uint32_t>(wParam));
            return 0;

        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButton(0, true);
            OnMouseButtonDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_LBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButton(0, false);
            OnMouseButtonUp(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_RBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButton(1, true);
            OnMouseButtonDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_RBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButton(1, false);
            OnMouseButtonUp(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButton(2, true);
            OnMouseButtonDown(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButton(2, false);
            OnMouseButtonUp(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_MOUSEMOVE:
        {
            const int oldX = m_input.MouseX();
            const int oldY = m_input.MouseY();
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            m_input.SetMousePosition(x, y);
            OnMouseMove(x, y, x - oldX, y - oldY);
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            m_input.AddMouseWheelDelta(delta);
            OnMouseWheel(delta);
            return 0;
        }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
