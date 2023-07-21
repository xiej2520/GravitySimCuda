#include "simulation.hpp"

namespace gravitysim {

using namespace DirectX;

Simulation::Simulation() {}

Simulation::Simulation(float time_step) : time_step(time_step) {}

Simulation::Simulation(std::vector<Body> bodies, std::vector<vec3f> vels,
                       float time_step)
    : bodies(std::move(bodies)), vels(std::move(vels)), time_step(time_step) {

  vels.resize(bodies.size());
  for (const Body &b : bodies) {
    inv_mass.push_back(1.0f / b.mass);
  }
  accs = std::vector<vec3f>(bodies.size(), {0, 0, 0});
}


void Simulation::calc_accs_cpu_particle_particle() {
  std::fill(accs.begin(), accs.end(), vec3f{0, 0, 0});
  for (int i=0; i<bodies.size(); i++) {
    for (int j=i+1; j<bodies.size(); j++) {
      XMVECTOR p1 = XMLoadFloat3(&bodies[i].pos);
      XMVECTOR p2 = XMLoadFloat3(&bodies[j].pos);
      XMVECTOR diff = p2 - p1;
      
      float F = G * bodies[i].mass * bodies[j].mass / XMVectorGetX(XMVector3Dot(diff, diff));
      
      XMVECTOR unit_diff = XMVector3Normalize(diff);
      XMVECTOR F_DIR = F * unit_diff;
      
      XMVECTOR a1 = XMLoadFloat3(&accs[i]);
      XMVECTOR a2 = XMLoadFloat3(&accs[j]);
      a1 += F_DIR;
      a2 -= F_DIR;
      XMStoreFloat3(&accs[i], a1);
      XMStoreFloat3(&accs[j], a2);
    }
  }
  
  for (int i=0; i<bodies.size(); i++) {
    XMVECTOR acc = XMLoadFloat3(&accs[i]);
    acc = XMVectorScale(acc, inv_mass[i]);
    XMStoreFloat3(&accs[i], acc);
  }
}




} // namespace gravitysim
