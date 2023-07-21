#pragma once

#include <algorithm>
#include <vector>

#include "DirectXMath.h"

namespace gravitysim {

using vec3f = DirectX::XMFLOAT3;

struct Body {
  vec3f pos;
  float mass;
};

enum class SimulationMethod {
  CPU_PARTICLE_PARTICLE,
  GPU_PARTICLE_PARTICLE,
};

class Simulation {
  std::vector<Body> bodies;
  std::vector<float> inv_mass;
  std::vector<vec3f> vels;
  std::vector<vec3f> accs;

  float time_step = 1;
  SimulationMethod method = SimulationMethod::CPU_PARTICLE_PARTICLE;
  
  static constexpr float G = 6.6743e-11f;

  void calc_accs_cpu_particle_particle();
  void calc_accs_gpu_particle_particle();
  
public:
  Simulation();
  Simulation(float time_step);
  Simulation(std::vector<Body> bodies, std::vector<vec3f> vels, float time_step);

  inline void step() {
    switch (method) {
    case SimulationMethod::CPU_PARTICLE_PARTICLE:
      calc_accs_cpu_particle_particle();
    break;
    case SimulationMethod::GPU_PARTICLE_PARTICLE:
      ;
    break;
    }
  }
};

} // namespace gravitysim
