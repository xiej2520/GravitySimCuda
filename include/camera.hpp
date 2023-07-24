#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "DirectXMath.h"

namespace gravitysim {

using Microsoft::WRL::ComPtr;

struct cbPerObject {
  DirectX::XMMATRIX WVP;
};

class Camera {
  DirectX::XMMATRIX WVP;
  DirectX::XMMATRIX world;
  DirectX::XMMATRIX cam_view;
  DirectX::XMMATRIX cam_proj;

  DirectX::XMVECTOR cam_pos;
  DirectX::XMVECTOR cam_tgt;
  DirectX::XMVECTOR cam_up;
  
public:
  inline DirectX::XMMATRIX &get_WVP() { return WVP; }
  Camera();

};

} // namespace gravitysim
