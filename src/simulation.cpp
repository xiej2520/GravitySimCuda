#include "simulation.hpp"

namespace gravitysim {

using namespace DirectX;

Simulation::Simulation() {}

Simulation::Simulation(float time_step) : time_step(time_step) {}

Simulation::Simulation(std::vector<float> masses_,
                       std::vector<vec3f> positions_, std::vector<vec3f> vels_,
                       float time_step)
    : masses(std::move(masses_)), positions(std::move(positions_)),
      vels(std::move(vels_)), time_step(time_step) {

  size_t n = masses.size();
  positions.resize(n);
  vels.resize(n);
  for (float m : masses) {
    inv_mass.push_back(1.0f / m);
  }
}

void Simulation::transfer_kinematics_to_simd() {
  size_t n = positions.size();
  simd_data.positions.resize(n);
  simd_data.vels.resize(n);
  simd_data.accs.resize(n);
  for (int i=0; i<n; i++) {
    simd_data.positions[i] = XMLoadFloat3(&positions[i]);
    simd_data.vels[i] = XMLoadFloat3(&vels[i]);
  }
}

void Simulation::transfer_simd_kinematics_to_cpu() {
  size_t n = simd_data.positions.size();
  assert(n == positions.size());
  assert(n == vels.size());
  for (int i=0; i<n; i++) {
    XMStoreFloat3(&positions[i], simd_data.positions[i]);
    XMStoreFloat3(&vels[i], simd_data.vels[i]);
  }
}

void Simulation::calc_accs_cpu_particle_particle() {
  transfer_kinematics_to_simd();
  std::fill(simd_data.accs.begin(), simd_data.accs.end(), XMVectorZero());

  for (int i = 0; i < masses.size(); i++) {
    for (int j = i + 1; j < masses.size(); j++) {
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

  for (int i = 0; i < masses.size(); i++) {
    simd_data.accs[i] = XMVectorScale(simd_data.accs[i], inv_mass[i]);
    simd_data.vels[i] += simd_data.accs[i] * time_step;
    simd_data.positions[i] += simd_data.vels[i] * time_step;
  }
  transfer_simd_kinematics_to_cpu();
}

} // namespace gravitysim
