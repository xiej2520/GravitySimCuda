#include "camera.hpp"

namespace gravitysim {

using namespace DirectX;

Camera::Camera(HWND hwnd) {
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
  
  m_keyboard = std::make_unique<Keyboard>();
  m_mouse = std::make_unique<Mouse>();
  m_mouse->SetWindow(hwnd);

  DefaultForward = XMVectorSet(0.0f,0.0f,1.0f, 0.0f);
  DefaultRight = XMVectorSet(1.0f,0.0f,0.0f, 0.0f);
  cam_forward = XMVectorSet(0.0f,0.0f,1.0f, 0.0f);
  cam_right = XMVectorSet(1.0f,0.0f,0.0f, 0.0f);

}

void Camera::UpdateCamera(Keyboard::KeyboardStateTracker &m_keys,
                  Mouse::ButtonStateTracker &m_mouseButtons, Mouse::State &mouse,
                  float elapsed) {
  
  float speed = 15.0f * elapsed;

  if (m_keys.pressed.A) {
    move_left_right -= speed;
  }
  if (m_keys.pressed.D) {
    move_left_right += speed;
  }
  if (m_keys.pressed.W) {
    move_back_forward += speed;
  }
  if (m_keys.pressed.S) {
    move_back_forward -= speed;
  }
  
  float mouse_speed = 0.001;
  if (mouse.positionMode == Mouse::MODE_RELATIVE) {
    cam_yaw += mouse_speed * mouse.x;
    if (cam_yaw > XM_PI) cam_yaw -= XM_2PI;
    else if (cam_yaw < -XM_PI) cam_yaw += XM_2PI;
    cam_pitch += mouse_speed * mouse.y;
  }
  
  if (mouse.leftButton) {
    m_mouse->SetMode(Mouse::MODE_RELATIVE);
  }

  cam_pos += move_left_right * cam_right;
  cam_pos += move_back_forward * cam_forward;

  move_left_right = 0.0f;
  move_back_forward = 0.0f;

  cam_rot = XMMatrixRotationRollPitchYaw(cam_pitch, cam_yaw, 0);
  cam_tgt = XMVector3TransformCoord(DefaultForward, cam_rot);
  cam_tgt = XMVector3Normalize(cam_tgt);

  cam_right = XMVector3TransformCoord(DefaultRight, cam_rot);
  cam_forward = XMVector3TransformCoord(DefaultForward, cam_rot);
  cam_up = XMVector3Cross(cam_forward, cam_right);

  cam_tgt = cam_pos + cam_tgt;

  cam_view = XMMatrixLookAtLH(cam_pos, cam_tgt, cam_up);
  
  WVP = world * cam_view * cam_proj;
}

} // namespace gravitysim
