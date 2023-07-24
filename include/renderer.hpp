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

  // shaders
  ComPtr<ID3DBlob> vs_blob_ptr, ps_blob_ptr, error_blob;
  ComPtr<ID3D11VertexShader> vertex_shader_ptr;
  ComPtr<ID3D11PixelShader> pixel_shader_ptr;
  ComPtr<ID3D11InputLayout> input_layout_ptr;

  ComPtr<ID3D11Buffer> vertex_buffer_ptr;
  UINT vertex_stride              = 3 * sizeof( float );
  UINT vertex_offset              = 0;
  UINT vertex_count               = 3;
  
  ImGuiIO *io;

  ComPtr<ID3D11Buffer> cbPerObjBuf;

public:
  Renderer(HWND hwnd, WNDCLASSEXW &wc);
  int InitPipeline();
  int InitImGui();

  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();

  void CreateRenderTarget();
  void CleanupRenderTarget();

  void CreateShaders();
  void LoadVertexBuffer();
  
  void set_size(UINT width, UINT height);

  
  struct RenderOptions {
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  };
  void RenderFrame(Camera &camera, RenderOptions &opts);
};

  

} // namespace gravitysim