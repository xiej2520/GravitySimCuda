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
using Microsoft::WRL::ComPtr;
#include "stdafx.h"

namespace gravitysim {
  

class Renderer {
  void InitPipeline();
};

  

} // namespace gravitysim