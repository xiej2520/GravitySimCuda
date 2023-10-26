#pragma once

#define UNICODE
#include "imgui.h"
#include "imgui_impl_win32.h"

#pragma comment(lib, "dxgi")
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include <wrl/client.h>
#include "stdafx.h"

#include "camera.hpp"
#include "simulation.hpp"


namespace gravitysim {
  
using Microsoft::WRL::ComPtr;

class Renderer {
  // Refs
  HWND hwnd;
  WNDCLASSEXW &wc;
  
  // Data
  ComPtr<ID3D11Device>            device;
  ComPtr<ID3D11DeviceContext>     device_context;
  ComPtr<IDXGISwapChain>          swap_chain;
  UINT                            resize_width = 0, resize_height = 0;
  ComPtr<ID3D11RenderTargetView>  main_render_tgtview;
  
  // Depth stencil
  ComPtr<ID3D11DepthStencilView> depth_stencil_view;
  ComPtr<ID3D11Texture2D> depth_stencil_buf;
  
  ImGuiIO *io;

public:
  Renderer(HWND hwnd, WNDCLASSEXW &wc);
  int InitPipeline();
  int InitImGui();

  // return true on error
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();

  void CreateRenderTarget();
  void CleanupRenderTarget();
  
  // set resize_width and resize_height for RenderFrame to handle
  void set_size(UINT width, UINT height);

  
  struct RenderOptions {
    bool run_simulation = false;
    bool show_demo_window = true;
    float body_scale = 1.0f;
    // temporary
    SimulationMethod method = SimulationMethod::CPU_PARTICLE_PARTICLE;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  };

  void RenderFrame(Camera &camera, RenderOptions &opts, Simulation &sim);
};

  

} // namespace gravitysim