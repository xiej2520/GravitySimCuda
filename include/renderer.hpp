#pragma once

#define UNICODE
#include "imgui.h"
#include "imgui_impl_win32.h"

#pragma comment(lib, "dxgi")
#include "d3dx12.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include "DXSampleHelper.h"
#include "stdafx.h"

namespace gravitysim {

struct FrameContext {
  ID3D12CommandAllocator *CommandAllocator;
  UINT64 FenceValue;
};
  

class Renderer {

};

  

} // namespace gravitysim