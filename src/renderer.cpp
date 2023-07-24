#include "renderer.hpp"

#include <d3d11.h>

#include "GeometricPrimitive.h"

namespace gravitysim {

using namespace DirectX;

Renderer::Renderer(HWND hwnd, WNDCLASSEXW &wc) : hwnd(hwnd), wc(wc) {}

int Renderer::InitPipeline() {
  if (!CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  CreateShaders();
  LoadVertexBuffer();
  return 0;
}

int Renderer::InitImGui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &ImGui::GetIO();
  io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(device.Get(), device_context.Get());

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
  // for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
  return 0;
}

bool Renderer::CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
      swap_chain.ReleaseAndGetAddressOf(), device.ReleaseAndGetAddressOf(),
      &featureLevel, device_context.ReleaseAndGetAddressOf());
  if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                     // driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, swap_chain.ReleaseAndGetAddressOf(), device.ReleaseAndGetAddressOf(),
        &featureLevel, device_context.ReleaseAndGetAddressOf());
  if (res != S_OK)
    return false;

  CreateRenderTarget();

  D3D11_TEXTURE2D_DESC depth_stencil_desc;
  depth_stencil_desc.Width     = 2000;
  depth_stencil_desc.Height    = 2000;
  depth_stencil_desc.MipLevels = 1;
  depth_stencil_desc.ArraySize = 1;
  depth_stencil_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_desc.SampleDesc.Count   = 1;
  depth_stencil_desc.SampleDesc.Quality = 0;
  depth_stencil_desc.Usage          = D3D11_USAGE_DEFAULT;
  depth_stencil_desc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_desc.CPUAccessFlags = 0; 
  depth_stencil_desc.MiscFlags      = 0;

  device->CreateTexture2D(&depth_stencil_desc, NULL, depth_stencil_buf.ReleaseAndGetAddressOf());
  device->CreateDepthStencilView(depth_stencil_buf.Get(), NULL, depth_stencil_view.ReleaseAndGetAddressOf());
  
  device_context->OMSetRenderTargets(1, main_render_tgtview.GetAddressOf(), depth_stencil_view.Get());


  return true;
}

void Renderer::CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (swap_chain) {
    swap_chain.Reset();
  }
  if (device_context) {
    swap_chain.Reset();
  }
  if (device) {
    swap_chain.Reset();
  }
}

void Renderer::CreateRenderTarget() {
  ID3D11Texture2D *pBackBuffer;
  swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  device->CreateRenderTargetView(pBackBuffer, nullptr,
                                 main_render_tgtview.GetAddressOf());
  pBackBuffer->Release();
}

void Renderer::CleanupRenderTarget() {
  if (main_render_tgtview) {
    main_render_tgtview.Reset();
  }
}

void Renderer::CreateShaders() {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
  // COMPILE VERTEX SHADER
  HRESULT hr = D3DCompileFromFile(
      L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main",
      "vs_5_0", flags, 0, &vs_blob_ptr, &error_blob);
  if (FAILED(hr)) {
    if (error_blob) {
      OutputDebugStringA((char *)error_blob->GetBufferPointer());
      error_blob.Reset();
    }
    if (vs_blob_ptr) {
      vs_blob_ptr.Reset();
    }
    assert(false);
  }

  // COMPILE PIXEL SHADER
  hr = D3DCompileFromFile(L"shaders.hlsl", nullptr,
                          D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main",
                          "ps_5_0", flags, 0, &ps_blob_ptr, &error_blob);
  if (FAILED(hr)) {
    if (error_blob) {
      OutputDebugStringA((char *)error_blob->GetBufferPointer());
      error_blob.Reset();
    }
    if (ps_blob_ptr) {
      ps_blob_ptr.Reset();
    }
    assert(false);
  }

  hr = device->CreateVertexShader(vs_blob_ptr->GetBufferPointer(),
                                  vs_blob_ptr->GetBufferSize(), NULL,
                                  vertex_shader_ptr.GetAddressOf());
  assert(SUCCEEDED(hr));

  hr = device->CreatePixelShader(ps_blob_ptr->GetBufferPointer(),
                                 ps_blob_ptr->GetBufferSize(), NULL,
                                 pixel_shader_ptr.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,
       0},
      /*
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
      D3D11_INPUT_PER_VERTEX_DATA, 0 }, { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, {
      "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
      D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
  };
  hr = device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc),
                                 vs_blob_ptr->GetBufferPointer(),
                                 vs_blob_ptr->GetBufferSize(),
                                 input_layout_ptr.GetAddressOf());
  assert(SUCCEEDED(hr));
}

void Renderer::LoadVertexBuffer() {
  /*
  float vertex_data_array[] = {
      0.0f,  0.5f,  0.0f, // point at top
      0.5f,  -0.5f, 0.0f, // point at bottom-right
      -0.5f, -0.5f, 0.0f, // point at bottom-left
  };
  */
  float vertex_data_array[] = {
    -0.5f, -0.5f, 0.5,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f
  };

  /*** load mesh data into vertex buffer **/
  D3D11_BUFFER_DESC vertex_buff_descr = {};
  vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
  vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
  vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA sr_data = {0};
  sr_data.pSysMem = vertex_data_array;
  HRESULT hr =
      device->CreateBuffer(&vertex_buff_descr, &sr_data, vertex_buffer_ptr.GetAddressOf());
  assert(SUCCEEDED(hr));
}

void Renderer::set_size(UINT width, UINT height) {
  resize_width = width;
  resize_height = height;
}

void Renderer::RenderFrame(Camera &camera, RenderOptions &opts) {
  // Handle window resize (we don't resize directly in the WM_SIZE handler)
  if (resize_width != 0 && resize_height != 0) {
    CleanupRenderTarget();
    swap_chain->ResizeBuffers(0, resize_width, resize_height,
                              DXGI_FORMAT_UNKNOWN, 0);
    resize_width = resize_height = 0;
    CreateRenderTarget();
  }

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // 1. Show the big demo window (Most of the sample code is in
  // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
  // ImGui!).
  if (&opts.show_demo_window)
    ImGui::ShowDemoWindow(&opts.show_demo_window);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair
  // to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and
                                   // append into it.

    ImGui::Text("This is some useful text."); // Display some text (you can use
                                              // a format strings too)
    ImGui::Checkbox("Demo Window",
                    &opts.show_demo_window); // Edit bools storing our window
                                             // open/close state
    ImGui::Checkbox("Another Window", &opts.show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f,
                       1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3(
        "clear color",
        (float *)&opts.clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                 // widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io->Framerate, io->Framerate);

    ImGui::Text("Camera Position: (%.3f, %.3f, %.3f), Target: (%.3f, %.3f, %.3f)",
        XMVectorGetX(camera.get_pos()), XMVectorGetY(camera.get_pos()), XMVectorGetZ(camera.get_pos()),
        XMVectorGetX(camera.get_tgt()), XMVectorGetY(camera.get_tgt()), XMVectorGetZ(camera.get_tgt())
    );
    
    ImGui::Text("Yaw: %.3f, Pitch: %.3f, Roll: %.3f", camera.get_yaw(), camera.get_pitch(), camera.get_roll());

    ImGui::End();
  }

  // 3. Show another simple window.
  if (opts.show_another_window) {
    ImGui::Begin(
        "Another Window",
        &opts.show_another_window); // Pass a pointer to our bool variable (the
                                    // window will have a closing button that
                                    // will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      opts.show_another_window = false;
    ImGui::End();
  }

  // Rendering
  ImGui::Render();
  const float clear_color_with_alpha[4] = {
      opts.clear_color.x * opts.clear_color.w,
      opts.clear_color.y * opts.clear_color.w,
      opts.clear_color.z * opts.clear_color.w, opts.clear_color.w};
  device_context->OMSetRenderTargets(1, main_render_tgtview.GetAddressOf(),
                                     depth_stencil_view.Get());
  device_context->ClearRenderTargetView(main_render_tgtview.Get(),
                                        clear_color_with_alpha);

  device_context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  RECT winRect;
  GetClientRect(hwnd, &winRect);
  D3D11_VIEWPORT viewport = {0.0f,
                             0.0f,
                             (FLOAT)(winRect.right - winRect.left),
                             (FLOAT)(winRect.bottom - winRect.top),
                             0.0f,
                             1.0f};
  device_context->RSSetViewports(1, &viewport);
  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  device_context->IASetInputLayout(input_layout_ptr.Get());
  device_context->IASetVertexBuffers(0, 1, vertex_buffer_ptr.GetAddressOf(),
                                     &vertex_stride, &vertex_offset);
  device_context->VSSetShader(vertex_shader_ptr.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_ptr.Get(), NULL, 0);
  

  D3D11_BUFFER_DESC cbbd;
  ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
  cbbd.Usage = D3D11_USAGE_DEFAULT;
  cbbd.ByteWidth = sizeof(cbPerObject);
  cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbbd.CPUAccessFlags = 0;
  cbbd.MiscFlags = 0;
  
  device->CreateBuffer(&cbbd, NULL, cbPerObjBuf.ReleaseAndGetAddressOf());
  
  cbPerObject cbPerObj;
  
  cbPerObj.WVP = XMMatrixTranspose(camera.get_WVP());
  device_context->UpdateSubresource(cbPerObjBuf.Get(), 0, NULL, &cbPerObj, 0, 0);
  device_context->VSSetConstantBuffers(0, 1, cbPerObjBuf.GetAddressOf());


  device_context->Draw(vertex_count, 0);
  
  auto sphere = GeometricPrimitive::CreateSphere(device_context.Get());
  auto geosphere = GeometricPrimitive::CreateGeoSphere(device_context.Get());
  
  XMMATRIX sphere_trans = XMMatrixTranslation(4, 4, 4);
  XMMATRIX geosphere_trans = XMMatrixTranslation(4, 0, 4);
  
  sphere->Draw(sphere_trans, camera.get_view(), camera.get_proj(), Colors::Green);
  geosphere->Draw(geosphere_trans, camera.get_view(), camera.get_proj(), Colors::Blue);
  for (int i=0; i<100; i++) {
    for (int j=0; j<100; j++) {
      XMMATRIX sphere_trans = XMMatrixTranslation(i, j, 10);
      sphere->Draw(sphere_trans, camera.get_view(), camera.get_proj(), Colors::Green);
    }
  }

  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  swap_chain->Present(1, 0); // Present with vsync
  // swap_chain->Present(0, 0); // Present without vsync
}

} // namespace gravitysim
