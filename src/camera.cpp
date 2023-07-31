#include "camera.hpp"
#include <cmath>

namespace gravitysim {

using namespace DirectX;

Camera::Camera(HWND hwnd) {
  cam_pos = XMVectorSet(0.0f, 0.0f, -0.5f, 0.0f);
  cam_tgt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  cam_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  
  cam_view = XMMatrixLookAtLH(cam_pos, cam_tgt, cam_up);
  cam_proj = XMMatrixPerspectiveFovLH(0.4f * 3.14f, 1, 1.0f, 1000.0f);
  
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

void Camera::set_size(UINT width, UINT height, float render_dist) {
  float aspect_ratio = static_cast<float>(width) / height;
  cam_proj = XMMatrixPerspectiveFovLH(0.4f * 3.14f, aspect_ratio, 1.0f, render_dist);
}

float normalize_rot(float val) {
  if (val > XM_PI) return val - XM_2PI;
  else if (val < -XM_PI) return val + XM_2PI;
  return val;
}

void Camera::UpdateCamera(Keyboard::KeyboardStateTracker &m_keys,
                  Mouse::ButtonStateTracker &m_mouseButtons, bool capture_mouse,
                  Mouse::State &mouse, float elapsed) {

  float move_left_right = 0.0f;
  float move_back_forward = 0.0f;
  float move_down_up = 0.0f;
  
  float speed = 10 * 15.0f * elapsed;

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
  if (m_keys.pressed.LeftShift) {
    move_down_up -= speed;
  }
  if (m_keys.pressed.Space) {
    move_down_up += speed;
  }

  cam_pos += move_left_right * cam_right;
  cam_pos += move_back_forward * cam_forward;
  cam_pos += move_down_up * cam_up;
  
  float mouse_speed = 0.001f;
  static int last_x = 0, last_y = 0;
  if (mouse.positionMode == Mouse::MODE_RELATIVE) {
    /*
    cam_yaw = normalize_rot(cam_yaw + mouse_speed * mouse.x);
    cam_pitch = normalize_rot(cam_pitch + mouse_speed * mouse.y);
    */
    /*
      |cos t  sin t|  |x|
      |-sin t cos t|  |y|
     */
    cam_yaw = normalize_rot(cam_yaw + mouse_speed * (mouse.x * cos(cam_roll) + mouse.y * sin(cam_roll)));
    cam_pitch = normalize_rot(cam_pitch + mouse_speed * (-mouse.x * sin(cam_roll) + mouse.y * cos(cam_roll)));
    SetCursorPos(last_x, last_y); // prevent mouse from going over ImGui window
  }
  
  float roll_speed = 2.0f * elapsed;
  
  if (m_keys.pressed.Q) {
    cam_roll -= roll_speed;
  }
  if (m_keys.pressed.E) {
    cam_roll += roll_speed;
  }
  cam_roll = normalize_rot(cam_roll);
  
  if (!capture_mouse) {
    if (mouse.leftButton) {
      last_x = mouse.x, last_y = mouse.y;
      m_mouse->SetMode(Mouse::MODE_RELATIVE);
    }
  }

  cam_rot = XMMatrixRotationRollPitchYaw(cam_pitch, cam_yaw, cam_roll);
  cam_tgt = XMVector3Normalize(XMVector3TransformCoord(DefaultForward, cam_rot));
  cam_tgt = cam_pos + cam_tgt;

  cam_right = XMVector3TransformCoord(DefaultRight, cam_rot);
  cam_forward = XMVector3TransformCoord(DefaultForward, cam_rot);
  cam_up = XMVector3Cross(cam_forward, cam_right);

  cam_view = XMMatrixLookAtLH(cam_pos, cam_tgt, cam_up);
  
  WVP = world * cam_view * cam_proj;
}

} // namespace gravitysim
