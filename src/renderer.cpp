#include "renderer.hpp"

#include <d3d11.h>

#include "GeometricPrimitive.h"

namespace gravitysim {

using namespace DirectX;

Renderer::Renderer(HWND hwnd, WNDCLASSEXW &wc) : hwnd(hwnd), wc(wc) {}

int Renderer::InitPipeline() {
  if (CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }

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
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        swap_chain.ReleaseAndGetAddressOf(), device.ReleaseAndGetAddressOf(),
        &featureLevel, device_context.ReleaseAndGetAddressOf());

  if (res != S_OK)
    return true;

  CreateRenderTarget();
  return false;
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

  D3D11_TEXTURE2D_DESC depth_stencil_desc;
  depth_stencil_desc.Width = resize_width;
  depth_stencil_desc.Height = resize_height;
  depth_stencil_desc.MipLevels = 1;
  depth_stencil_desc.ArraySize = 1;
  depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_desc.SampleDesc.Count = 1;
  depth_stencil_desc.SampleDesc.Quality = 0;
  depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_desc.CPUAccessFlags = 0;
  depth_stencil_desc.MiscFlags = 0;

  device->CreateTexture2D(&depth_stencil_desc, NULL,
                          depth_stencil_buf.ReleaseAndGetAddressOf());
  device->CreateDepthStencilView(depth_stencil_buf.Get(), NULL,
                                 depth_stencil_view.ReleaseAndGetAddressOf());
}

void Renderer::CleanupRenderTarget() {
  if (main_render_tgtview) {
    main_render_tgtview.Reset();
  }
}

void Renderer::set_size(UINT width, UINT height) {
  resize_width = width;
  resize_height = height;
}

void Renderer::RenderFrame(Camera &camera, RenderOptions &opts,
                           Simulation &sim) {
  // Handle window resize (we don't resize directly in the WM_SIZE handler)
  if (resize_width != 0 && resize_height != 0) {
    CleanupRenderTarget();
    swap_chain->ResizeBuffers(0, resize_width, resize_height,
                              DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
    resize_width = resize_height = 0;
  }

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // Show demo window (See ImGui::ShowDemoWindow())
  if (&opts.show_demo_window)
    ImGui::ShowDemoWindow(&opts.show_demo_window);

  // ImGui::Begin and ImGui::End to create a simple window
  {
    static float f = 0.0f; // temporary

    ImGui::Begin("Gravity Simulation"); // Create window

    // checkbox for run_simulation
    ImGui::Checkbox("Run Simulation", &opts.run_simulation);

    // checkbox for show demo window
    ImGui::Checkbox("Demo Window", &opts.show_demo_window);

    // simulation calculation options using radio buttons
    ImGui::RadioButton(
        "CPU Particle-Particle", reinterpret_cast<int *>(&opts.method),
        static_cast<int>(SimulationMethod::CPU_PARTICLE_PARTICLE));
    ImGui::SameLine();
    ImGui::RadioButton(
        "GPU Particle-Particle", reinterpret_cast<int *>(&opts.method),
        static_cast<int>(SimulationMethod::GPU_PARTICLE_PARTICLE));

    // scale of the rendered bodies (temporary)
    ImGui::SliderFloat("Body scale", &opts.body_scale, 0.1f, 100.0f);

    ImGui::ColorEdit3("Background Color", (float *)&opts.clear_color);

    // button to set frame to COM frame
    if (ImGui::Button("Set center of mass frame")) {
      sim.set_COM_frame();
    }

    // show data about simulation
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io->Framerate, io->Framerate);

    ImGui::Text(
        "Camera Position: (%.3f, %.3f, %.3f), Target: (%.3f, %.3f, %.3f)",
        XMVectorGetX(camera.get_pos()), XMVectorGetY(camera.get_pos()),
        XMVectorGetZ(camera.get_pos()), XMVectorGetX(camera.get_tgt()),
        XMVectorGetY(camera.get_tgt()), XMVectorGetZ(camera.get_tgt()));

    ImGui::Text("Yaw: %.3f, Pitch: %.3f, Roll: %.3f", camera.get_yaw(),
                camera.get_pitch(), camera.get_roll());
    ImGui::SameLine();
    if (ImGui::Button("Reset")) camera.reset_look();

    ImGui::Text("KE: %g, PE: %g, TE: %g", sim.get_KE(), sim.get_KE(), sim.get_KE() + sim.get_PE());

    ImGui::End();
  }

  // example for showing another window
  if (opts.run_simulation) {
    ImGui::Begin(
        "Another Window",
        &opts.run_simulation);

    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      opts.run_simulation = false;
    ImGui::End();
  }

  // Rendering
  ImGui::Render();
  // clear the image to the background color
  const float clear_color_with_alpha[4] = {
      opts.clear_color.x * opts.clear_color.w,
      opts.clear_color.y * opts.clear_color.w,
      opts.clear_color.z * opts.clear_color.w, opts.clear_color.w};
  device_context->ClearDepthStencilView(depth_stencil_view.Get(),
                                        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                        1.0f, 0);
  device_context->OMSetRenderTargets(1, main_render_tgtview.GetAddressOf(),
                                     depth_stencil_view.Get());
  device_context->ClearRenderTargetView(main_render_tgtview.Get(),
                                        clear_color_with_alpha);

  // set viewport
  RECT winRect;
  GetClientRect(hwnd, &winRect);
  D3D11_VIEWPORT viewport = {0.0f,
                             0.0f,
                             (FLOAT)(winRect.right - winRect.left),
                             (FLOAT)(winRect.bottom - winRect.top),
                             0.0f,
                             1.0f};
  device_context->RSSetViewports(1, &viewport);

  // copy paste this for every body
  auto geosphere = GeometricPrimitive::CreateGeoSphere(device_context.Get(),
                                                       opts.body_scale, 2);

  for (auto &pos : sim.get_positions()) {
    XMMATRIX trans = XMMatrixTranslation(pos.x, pos.y, pos.z);
    geosphere->Draw(trans, camera.get_view(), camera.get_proj(), Colors::Purple,
                    NULL, false);
  }
  auto &pos = sim.get_positions().back();
  //auto bigsphere = GeometricPrimitive::CreateGeoSphere(device_context.Get(), opts.body_scale * 4, 2);
  //XMMATRIX trans = XMMatrixTranslation(pos.x, pos.y, pos.z);
  //bigsphere->Draw(trans, camera.get_view(), camera.get_proj(), Colors::Green,
  //                NULL, false);

  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  swap_chain->Present(1, 0); // Present with vsync
  // swap_chain->Present(0, 0); // Present without vsync
}

} // namespace gravitysim
