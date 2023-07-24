#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "DirectXMath.h"

#include "Keyboard.h"
#include "Mouse.h"

#include <memory>

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

  DirectX::XMMATRIX cam_rot;

  DirectX::XMVECTOR DefaultForward;
  DirectX::XMVECTOR DefaultRight;
  DirectX::XMVECTOR cam_forward;
  DirectX::XMVECTOR cam_right;

  float move_left_right = 0.0f;
  float move_back_forward = 0.0f;

  float cam_yaw = 0.0f;
  float cam_pitch = 0.0f;

public:
  std::unique_ptr<DirectX::Keyboard> m_keyboard;
  std::unique_ptr<DirectX::Mouse> m_mouse;

  inline DirectX::XMMATRIX &get_WVP() { return WVP; }
  Camera(HWND hwnd);

  void UpdateCamera(DirectX::Keyboard::KeyboardStateTracker &m_keys,
                    DirectX::Mouse::ButtonStateTracker &m_mouseButtons,
                    float elapsed);
};

} // namespace gravitysim
