cmake_minimum_required(VERSION 3.8)
project(GravitySimCuda LANGUAGES CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES 75)

enable_language(CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)

set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cu" "src/*.cpp")


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(gravity_sim_cuda ${SOURCES})

target_compile_features(gravity_sim_cuda PUBLIC cxx_std_20)
set_target_properties(gravity_sim_cuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_subdirectory(imgui)
add_subdirectory(DirectXMath)
add_subdirectory(${CMAKE_SOURCE_DIR}/DirectXTK ${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK)
target_link_libraries(gravity_sim_cuda PRIVATE imgui d3d12.lib DirectXTK)


## Google Test for simulation

enable_testing()
add_executable(tests test/test_main.cpp
  src/camera.cpp
  src/simulation.cpp
  src/simulation.cu
)
target_link_libraries(
  tests
  d3d12.lib DirectXTK
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests DISCOVERY_MODE PRE_TEST)

##

# cmake -S . -B build -DCMAKE_CUDA_ARCHITECTURES=75
# cmake --build build --config Release