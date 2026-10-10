#pragma once
// Colouring a ray: flat-shaded facets in four light bands, hard shadows, haze,
// the sky, and the lake that mirrors all of it.

#include "scene.hpp"
#include "solids.hpp"

constexpr int band_count = 4;

// one traced pixel, before the ink and the palette
struct Sample {
    Vec3 rgb;
    int solid = -1; // what was seen, through the water if reflected; -1 sky
    float depth = 1e30f;
    int band = band_count - 1;
    bool reflected = false;
};

Vec3 sky_color(const Scene& scene, Vec3 dir);
Sample trace(const Scene& scene, Vec3 origin, Vec3 dir);
