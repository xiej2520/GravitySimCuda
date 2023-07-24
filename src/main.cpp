// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma comment(lib, "d3d11.lib")
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>


#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "renderer.hpp"

#include "Keyboard.h"
#include "Mouse.h"
#include "StepTimer.hpp"

#include <iostream>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


gravitysim::Renderer *renderer_ptr;
gravitysim::Camera *camera_ptr;
DX::StepTimer *timer_ptr;

using namespace DirectX;

int main(int, char**) {
  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
  ::RegisterClassExW(&wc);
  HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
  
  gravitysim::Renderer renderer(hwnd, wc);
  renderer_ptr = &renderer;
  gravitysim::Camera camera(hwnd);
  camera_ptr = &camera;
  DX::StepTimer timer;
  timer_ptr = &timer;
  
  Keyboard::KeyboardStateTracker m_keys;
  Mouse::ButtonStateTracker m_mouseButtons;
  
  if (renderer.InitPipeline()) {
    return 1;
  }

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);
  
  renderer.InitImGui();

  gravitysim::Renderer::RenderOptions opts;
  // Main loop
  bool done = false;
  while (!done) {
      // Poll and handle messages (inputs, window resize, etc.)
      // See the WndProc() function below for our to dispatch events to the Win32 backend.
      MSG msg;
      while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
          ::TranslateMessage(&msg);
          ::DispatchMessage(&msg);
          if (msg.message == WM_QUIT)
              done = true;
      }
      if (done) break;
      renderer.RenderFrame(camera, opts);
      
      float elapsed;
      timer.Tick([&]() {
        elapsed = static_cast<float>(timer.GetElapsedSeconds());
      });
      
      auto kb = camera.m_keyboard->GetState();
      m_keys.Update(kb);
      if (kb.Escape) {
        camera.m_mouse->SetMode(Mouse::MODE_ABSOLUTE);
      }

      auto mouse = camera.m_mouse->GetState();
      m_mouseButtons.Update(mouse);
      
      ImGuiIO &io = ImGui::GetIO();
      camera.UpdateCamera(m_keys, m_mouseButtons, io.WantCaptureMouse, mouse, elapsed);
      m_keys.Reset();
      m_mouseButtons.Reset();

  }

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  renderer.CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

  return 0;
}


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
      return true;

  switch (msg) {
  case WM_ACTIVATEAPP:
      Keyboard::ProcessMessage(msg, wParam, lParam);
      Mouse::ProcessMessage(msg, wParam, lParam);
      break;
  case WM_ACTIVATE:
  case WM_INPUT:
  case WM_MOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MOUSEWHEEL:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  case WM_MOUSEHOVER:
      Mouse::ProcessMessage(msg, wParam, lParam);
      break;
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
      Keyboard::ProcessMessage(msg, wParam, lParam);
      break;
  case WM_SYSKEYDOWN:
      Keyboard::ProcessMessage(msg, wParam, lParam);
      if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) {
      }
      break;
  case WM_MOUSEACTIVATE:
      // When you click activate the window, we want Mouse to ignore it.
      return MA_ACTIVATEANDEAT;
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
        return 0;
    // queue resize
    renderer_ptr->set_size(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
    camera_ptr->set_size(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
