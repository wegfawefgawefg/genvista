#include "noise.hpp"

#include <cmath>

std::uint32_t hash_u32(std::uint32_t a) {
    a ^= a >> 16;
    a *= 0x7feb352dU;
    a ^= a >> 15;
    a *= 0x846ca68bU;
    a ^= a >> 16;
    return a;
}

float hash01(int x, int y, int z, int seed) {
    std::uint32_t h = hash_u32(static_cast<std::uint32_t>(seed) * 0x9e3779b9U);
    h = hash_u32(h ^ static_cast<std::uint32_t>(x) * 0x85ebca6bU);
    h = hash_u32(h ^ static_cast<std::uint32_t>(y) * 0xc2b2ae35U);
    h = hash_u32(h ^ static_cast<std::uint32_t>(z) * 0x27d4eb2fU);
    return static_cast<float>(h >> 8) / 16777216.0f;
}

static float fade(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float noise3(Vec3 p, float scale, int seed) {
    const float x = p.x / scale;
    const float y = p.y / scale;
    const float z = p.z / scale;
    const int ix = static_cast<int>(std::floor(x));
    const int iy = static_cast<int>(std::floor(y));
    const int iz = static_cast<int>(std::floor(z));
    const float fx = fade(x - static_cast<float>(ix));
    const float fy = fade(y - static_cast<float>(iy));
    const float fz = fade(z - static_cast<float>(iz));
    float corner[8];
    for (int i = 0; i < 8; ++i) {
        corner[i] = hash01(ix + (i & 1), iy + ((i >> 1) & 1), iz + ((i >> 2) & 1), seed);
    }
    const float x00 = lerpf(corner[0], corner[1], fx);
    const float x10 = lerpf(corner[2], corner[3], fx);
    const float x01 = lerpf(corner[4], corner[5], fx);
    const float x11 = lerpf(corner[6], corner[7], fx);
    return lerpf(lerpf(x00, x10, fy), lerpf(x01, x11, fy), fz);
}

float fbm3(Vec3 p, float scale, int seed, int octaves) {
    float sum = 0.0f;
    float weight = 0.5f;
    float total = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        sum += noise3(p, scale, seed + i * 31) * weight;
        total += weight;
        scale *= 0.5f;
        weight *= 0.5f;
    }
    return sum / total;
}

float noise2(float x, float y, int seed) {
    return noise3(Vec3{x, y, 0.5f}, 1.0f, seed);
}

Rng rng_from(int seed) {
    Rng rng;
    rng.state = hash_u32(static_cast<std::uint32_t>(seed) * 747796405U + 2891336453U) | 1U;
    return rng;
}

float rng_unit(Rng& rng) {
    rng.state ^= rng.state << 13;
    rng.state ^= rng.state >> 17;
    rng.state ^= rng.state << 5;
    return static_cast<float>(rng.state >> 8) / 16777216.0f;
}

float rng_range(Rng& rng, float lo, float hi) {
    return lo + (hi - lo) * rng_unit(rng);
}

float rng_signed(Rng& rng) {
    return rng_unit(rng) * 2.0f - 1.0f;
}
