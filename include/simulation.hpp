#pragma once

#include "gpu_sim_data.cuh"

#include <vector>

#include "DirectXMath.h"

namespace gravitysim {

using vec3f = DirectX::XMFLOAT3;

enum class SimulationMethod : int {
  CPU_PARTICLE_PARTICLE,
  GPU_PARTICLE_PARTICLE,
};

// store simulation data as SIMD XMVECTORS
struct SIMDSimData {
  std::vector<DirectX::XMVECTOR> positions;
  std::vector<DirectX::XMVECTOR> vels;
  std::vector<DirectX::XMVECTOR> accs;
};

class Simulation {
  size_t num_bodies;

  std::vector<float> masses;
  // mu = G * mass
  std::vector<float> mus;
  // 1 / mu
  std::vector<float> inv_mu;

  std::vector<vec3f> positions;
  std::vector<vec3f> vels;
  
  SIMDSimData simd_data;
  GPUSimData gpu_data;

  float time_step = 1.0f;
  SimulationMethod method = SimulationMethod::CPU_PARTICLE_PARTICLE;
  
  // to be implemented
  float dist_scale = 1.0f;
  float mass_scale = 1.0f;
  
  static constexpr float G = 6.6743e-11f;

  // steps simulation forward, updates data in simd_data
  void calc_accs_cpu_particle_particle();
  // tries calculating forces, then scaling sum of forces; can cause precision errors
  void calc_accs_cpu_particle_particle_halved();
  // steps simulation forward, updates data in gpu_data
  void calc_accs_gpu_particle_particle();
  
  // moves data in positions and vels to simd_data, needed for calculating using SIMD
  void transfer_kinematics_to_simd();
  // moves data from simd_data to positions and vels, needed for synchronizing gpu data
  void transfer_simd_kinematics_to_cpu();
  // moves positions from simd_data to positions, needed for rendering
  void transfer_simd_positions_to_cpu();

  void transfer_mus_to_gpu();
  // moves kinematics data to gpu_data, needed for calculating on gpu
  void transfer_kinematics_to_gpu();
  // moves kinematics data from gpu to cpu, needed for synchronizing with SIMD
  void transfer_gpu_kinematics_to_cpu();
  // moves positions from gpu to cpu, needed for rendering
  void transfer_gpu_positions_to_cpu();
  
public:
  Simulation();
  Simulation(float time_step);
  Simulation(std::vector<float> masses, std::vector<vec3f> positions, std::vector<vec3f> vels, float time_step);

  inline SimulationMethod get_method() { return method; }
  inline const std::vector<float> &get_masses() { return masses; }
  inline const std::vector<vec3f> &get_positions() { return positions; }
  
  float get_KE();
  float get_PE();

  // sets simulation method and moves data
  void switch_method(SimulationMethod new_method);

  // steps the simulation forward
  void step();

  // sets COM frame: total momentum of system zeroed
  void set_COM_frame();
};

} // namespace gravitysim
