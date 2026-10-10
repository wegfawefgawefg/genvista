#pragma once
// From traced samples to a PNG on disk: ink, palette, blow-up, write.

#include "shade.hpp"

#include <vector>

struct PictureStyle {
    int scale = 5; // each traced pixel becomes scale x scale
    float dither = 10.0f;
    bool palette = true;
};

bool write_picture(const Scene& scene, std::vector<Sample>& samples, const PictureStyle& style,
                   const char* path);
