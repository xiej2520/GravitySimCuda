#include "camera.hpp"

namespace gravitysim {

using namespace DirectX;

Camera::Camera() {
  cam_pos = XMVectorSet(0.0f, 0.0f, -0.5f, 0.0f);
  cam_tgt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  cam_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  
  cam_view = XMMatrixLookAtLH(cam_pos, cam_tgt, cam_up);
  float width = 300;
  float height = 300;
  float aspect_ratio = width / height;
  cam_proj = XMMatrixPerspectiveFovLH(0.4f * 3.14f, aspect_ratio, 1.0f, 1000.0f);
  
  world = XMMatrixIdentity();
  WVP = world * cam_view * cam_proj;
}

} // namespace gravitysim
