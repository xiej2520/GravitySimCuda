#include "simulation.hpp"
#include <algorithm>
#include <execution>

namespace gravitysim {

using namespace DirectX;

Simulation::Simulation() {}

Simulation::Simulation(float time_step) : time_step(time_step) {}

Simulation::Simulation(std::vector<float> masses_,
                       std::vector<vec3f> positions_, std::vector<vec3f> vels_,
                       float time_step)
    : num_bodies(masses_.size()), masses(std::move(masses_)), positions(std::move(positions_)),
      vels(std::move(vels_)), time_step(time_step) {

  positions.resize(num_bodies);
  vels.resize(num_bodies);

  transfer_kinematics_to_simd(); // for initial
  //switch_method(SimulationMethod::GPU_PARTICLE_PARTICLE);
}

void Simulation::transfer_kinematics_to_simd() {
  simd_data.positions.resize(num_bodies);
  simd_data.vels.resize(num_bodies);
  simd_data.accs.resize(num_bodies);
  for (int i = 0; i < num_bodies; i++) {
    simd_data.positions[i] = XMLoadFloat3(&positions[i]);
    simd_data.vels[i] = XMLoadFloat3(&vels[i]);
  }
}

void Simulation::transfer_simd_kinematics_to_cpu() {
  assert(num_bodies == positions.size());
  assert(num_bodies == vels.size());
  std::for_each(std::execution::par_unseq, simd_data.positions.begin(), simd_data.positions.end(),
    [&](const XMVECTOR &pos) {
      size_t index = &pos - simd_data.positions.data();
      XMStoreFloat3(&positions[index], pos);
      XMStoreFloat3(&vels[index], simd_data.vels[index]);
    }
  );
  //for (size_t i=0; i<num_bodies; i++) {
  //  XMStoreFloat3(&positions[i], simd_data.positions[i]);
  //  XMStoreFloat3(&vels[i], simd_data.vels[i]);
  //}
}

void Simulation::transfer_simd_positions_to_cpu() {
  assert(num_bodies == simd_data.positions.size());
  std::for_each(std::execution::par_unseq, simd_data.positions.begin(), simd_data.positions.end(),
    [&](const XMVECTOR &pos) {
      size_t index = &pos - simd_data.positions.data();
      XMStoreFloat3(&positions[index], pos);
    }
  );
}

void Simulation::calc_accs_cpu_particle_particle() {
  std::fill(simd_data.accs.begin(), simd_data.accs.end(), XMVectorZero());

  for (int i = 0; i < num_bodies; i++) {
    for (int j = i + 1; j < num_bodies; j++) {
      XMVECTOR p1 = simd_data.positions[i];
      XMVECTOR p2 = simd_data.positions[j];
      XMVECTOR diff = p2 - p1;

      float F =
          G * masses[i] * masses[j] / XMVectorGetX(XMVector3Dot(diff, diff));

      XMVECTOR unit_diff = XMVector3Normalize(diff);
      XMVECTOR F_DIR = F * unit_diff;

      simd_data.accs[i] += F_DIR;
      simd_data.accs[j] -= F_DIR;
    }
  }

  for (int i = 0; i < num_bodies; i++) {
    simd_data.accs[i] = XMVectorScale(simd_data.accs[i], inv_mass[i]);
    simd_data.vels[i] += simd_data.accs[i] * time_step;
    simd_data.positions[i] += simd_data.vels[i] * time_step;
  }
}


float Simulation::get_KE() {
  float KE = 0.0f;
  for (int i = 0; i < num_bodies; i++) {
    XMVECTOR vi = XMLoadFloat3(&vels[i]);
    KE += 0.5f * masses[i] * XMVectorGetX(XMVector3Dot(vi, vi));
  }
  return KE;
}

float Simulation::get_PE() {
  float PE = 0.0f;
  for (int i = 0; i < num_bodies; i++) {
    for (int j = i + 1; j < num_bodies; j++) {
      XMVECTOR pi = XMLoadFloat3(&positions[i]);
      XMVECTOR pj = XMLoadFloat3(&positions[j]);
      PE -= G * masses[i] * masses[j] / XMVectorGetX(XMVector3Length(pj - pi));
    }
  }
  return PE;
}

void Simulation::switch_method(SimulationMethod new_method) {
  switch (method) {
  case SimulationMethod::CPU_PARTICLE_PARTICLE:
    transfer_simd_kinematics_to_cpu();
    break;
  case SimulationMethod::GPU_PARTICLE_PARTICLE:
    transfer_gpu_kinematics_to_cpu();
    break;
  }
  method = new_method;
  switch (new_method) {
  case SimulationMethod::CPU_PARTICLE_PARTICLE:
    transfer_kinematics_to_simd();
    break;
  case SimulationMethod::GPU_PARTICLE_PARTICLE:
    transfer_masses_to_gpu();
    transfer_kinematics_to_gpu();
    break;
  }
}

void Simulation::step() {
  switch (method) {
  case SimulationMethod::CPU_PARTICLE_PARTICLE:
    for (int i=0; i<1; i++)
      calc_accs_cpu_particle_particle();
    transfer_simd_positions_to_cpu();
  break;
  case SimulationMethod::GPU_PARTICLE_PARTICLE:
    for (int i=0; i<10; i++)
      calc_accs_gpu_particle_particle();
    transfer_gpu_positions_to_cpu();
  break;
  }
}

} // namespace gravitysim
