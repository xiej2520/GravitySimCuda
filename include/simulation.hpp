#pragma once

#include "gpu_sim_data.cuh"

#include <algorithm>
#include <vector>

#include "DirectXMath.h"

namespace gravitysim {

using vec3f = DirectX::XMFLOAT3;

enum class SimulationMethod {
  CPU_PARTICLE_PARTICLE,
  GPU_PARTICLE_PARTICLE,
};

struct SIMDSimData {
  std::vector<DirectX::XMVECTOR> positions;
  std::vector<DirectX::XMVECTOR> vels;
  std::vector<DirectX::XMVECTOR> accs;
};

class Simulation {
  std::vector<float> masses;
  std::vector<float> inv_mass;
  std::vector<vec3f> positions;
  std::vector<vec3f> vels;
  std::vector<vec3f> accs;
  
  SIMDSimData simd_data;
  GPUSimData gpu_data;

  float time_step = 1;
  SimulationMethod method = SimulationMethod::GPU_PARTICLE_PARTICLE;
  
  static constexpr float G = 6.6743e-11f;

  void calc_accs_cpu_particle_particle();
  void calc_accs_gpu_particle_particle();
  
  void transfer_kinematics_to_simd();
  void transfer_simd_kinematics_to_cpu();

  void transfer_masses_to_gpu();
  void transfer_kinematics_to_gpu();
  void transfer_gpu_kinematics_to_cpu();
  
public:
  Simulation();
  Simulation(float time_step);
  Simulation(std::vector<float> masses, std::vector<vec3f> positions, std::vector<vec3f> vels, float time_step);

  inline std::vector<float> &get_masses() { return masses; }
  inline std::vector<vec3f> &get_positions() { return positions; }

  inline void step() {
    switch (method) {
    case SimulationMethod::CPU_PARTICLE_PARTICLE:
      calc_accs_cpu_particle_particle();
    break;
    case SimulationMethod::GPU_PARTICLE_PARTICLE:
      calc_accs_gpu_particle_particle();
    break;
    }
  }
};

} // namespace gravitysim
