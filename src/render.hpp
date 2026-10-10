#pragma once
// One ray per pixel through a wobbling grid, spread over threads.

#include "shade.hpp"

#include <vector>

Vec3 camera_ray(const Camera& camera, float px, float py);
std::vector<Sample> render(const Scene& scene, int threads);
