#pragma once
// Hashed value noise, for grain that sticks to a solid and for the wobble of
// the ray grid. Everything here is a pure function of its inputs.

#include "math.hpp"

#include <cstdint>

std::uint32_t hash_u32(std::uint32_t a);
float hash01(int x, int y, int z, int seed);

// smooth value noise in 0..1, features about `scale` apart
float noise3(Vec3 p, float scale, int seed);
float fbm3(Vec3 p, float scale, int seed, int octaves);
float noise2(float x, float y, int seed);

// a small seeded generator for scene building
struct Rng {
    std::uint32_t state = 1;
};

Rng rng_from(int seed);
float rng_unit(Rng& rng);
float rng_range(Rng& rng, float lo, float hi);
float rng_signed(Rng& rng); // -1..1
