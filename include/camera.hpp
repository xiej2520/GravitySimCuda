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

  float cam_yaw = 0.0f;
  float cam_pitch = 0.0f;
  float cam_roll = 0.0f;

public:
  std::unique_ptr<DirectX::Keyboard> m_keyboard;
  std::unique_ptr<DirectX::Mouse> m_mouse;

  inline DirectX::XMMATRIX &get_WVP() { return WVP; }
  inline DirectX::XMVECTOR &get_pos() { return cam_pos; }
  inline DirectX::XMVECTOR &get_tgt() { return cam_tgt; }
  inline DirectX::XMMATRIX &get_world() { return world; }
  inline DirectX::XMMATRIX &get_view() { return cam_view; }
  inline DirectX::XMMATRIX &get_proj() { return cam_proj; }
  inline float get_yaw() { return cam_yaw; }
  inline float get_pitch() { return cam_pitch; }
  inline float get_roll() { return cam_roll; }

  void set_size(UINT width, UINT height);
  
  Camera(HWND hwnd);

  void UpdateCamera(DirectX::Keyboard::KeyboardStateTracker &m_keys,
                    DirectX::Mouse::ButtonStateTracker &m_mouseButtons,
                    bool capture_mouse,
                    DirectX::Mouse::State &mouse,
                    float elapsed);
};

inline void print_mat(const DirectX::XMMATRIX &mat) {
  DirectX::XMFLOAT4X4 mv;
  DirectX::XMStoreFloat4x4(&mv, mat);
  printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n %f %f %f %f\n",
      mv._11, mv._12, mv._13, mv._14,
      mv._21, mv._22, mv._23, mv._24,
      mv._31, mv._32, mv._33, mv._34,
      mv._41, mv._42, mv._43, mv._44
  );
}

} // namespace gravitysim
