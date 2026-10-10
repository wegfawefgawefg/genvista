#pragma once
// A straight, level glide over the lake toward the far shore, one PNG per
// frame. The scene is built once from the starting eye; only the eye moves.

#include "picture.hpp"
#include "scene.hpp"

#include <string>

struct FlyPath {
    int frames = 360;
    float to_z = 72.0f;  // metres travelled
    float height = 0.6f; // above the water, the whole way
};

bool fly_over(Scene& scene, const FlyPath& path, const PictureStyle& style, int threads,
              const std::string& dir);
