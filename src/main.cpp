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


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

gravitysim::Renderer *renderer_ptr;

  #include <iostream>
// Main code
int main(int, char**) {
  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
  ::RegisterClassExW(&wc);
  HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
  
  gravitysim::Renderer renderer(hwnd, wc);
  renderer_ptr = &renderer;
  
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
      renderer.RenderFrame(opts);
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
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
        return 0;
    // queue resize
    renderer_ptr->set_size(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
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
