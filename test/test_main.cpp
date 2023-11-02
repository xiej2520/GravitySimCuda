#include <gtest/gtest.h>

#include "simulation.hpp"

TEST(Hello, BasicAssertions) {
  EXPECT_STRNE("hello", "world");
  EXPECT_EQ(7 * 6, 42);
}

TEST(GravitySim, set_COM_frame) {
  std::vector<float> masses = {2e6, 1e6, 4e5};
  std::vector<DirectX::XMFLOAT3> positions = {{0, 0, 0}, {10, 10, 10}, {0, 100, -12}};
  std::vector<DirectX::XMFLOAT3> vels = {{0, 0, 0}, {3, 3, 3}, {-4, -6, 9}};
  gravitysim::Simulation simulation(masses, positions, vels, 0.01f);
  
  simulation.set_COM_frame();
  
  const auto &v = simulation.get_vels();
  
  DirectX::XMFLOAT3 total_momentum{};
  for (int i=0; i<masses.size(); i++) {
    total_momentum.x += masses[i] * v[i].x;
    total_momentum.y += masses[i] * v[i].y;
    total_momentum.z += masses[i] * v[i].z;
  }
  EXPECT_LT(total_momentum.x, 10.0f);
  EXPECT_LT(total_momentum.y, 10.0f);
  EXPECT_LT(total_momentum.z, 10.0f);
  printf("Total momentum: %g %g %g\n", total_momentum.x, total_momentum.y, total_momentum.z);
}

TEST(GravitySim, ConserveTE2Bodies) {
  std::vector<float> masses = {2e3, 1e3};
  std::vector<DirectX::XMFLOAT3> positions = {{0, 0, 0}, {10, 10, 10}};
  std::vector<DirectX::XMFLOAT3> vels = {{0, 0, 0}, {3, 3, 3}};
  gravitysim::Simulation simulation(masses, positions, vels, 0.01f);
  
  simulation.set_G(1e5); // avoid precision issues
  simulation.set_COM_frame();
  
  float KE = simulation.get_KE();
  float PE = simulation.get_PE();
  float TE = KE + PE;
  printf("Initial KE: %.7f, Initial PE: %.7f, TE: %.7f\n", KE, PE, TE);
  for (int i=0; i<10; i++) {
    for (int j=0; j<1000; j++) {
      simulation.step();
    }
    KE = simulation.get_KE();
    PE = simulation.get_PE();
    EXPECT_EQ(TE, KE + PE);
    //printf("KE: %.7f, PE: %.7f, TE: %.7f\n", KE, PE, KE + PE);
    //for (auto &p : simulation.get_positions()) {
    //  printf("%g %g %g\n", p.x, p.y, p.z);
    //}
  }
}
