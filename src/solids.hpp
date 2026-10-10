#pragma once
// Building convex solids out of planes, and casting rays against them.

#include "noise.hpp"
#include "scene.hpp"

#include <vector>

struct Hit {
    float t = 0.0f;
    int solid = -1;
    int facet = -1; // which plane it came in through
};

int add_material(Scene& scene, const Material& material);
// `inside` must be inside the solid; its bounding sphere is found from there
int add_solid(Scene& scene, const std::vector<Plane>& planes, Vec3 inside, int material, int seed);

// a lumpy ellipsoid: `count` planes tangent to it, each pushed out a little
std::vector<Plane> lump_planes(Vec3 centre, Vec3 radii, float yaw, int count, float jitter,
                               Rng& rng);
// one skirt of a spruce: a ragged frustum from radius r0 at base.y up to r1
std::vector<Plane> tier_planes(Vec3 base, float r0, float height, float r1, int sides, float twist,
                               float jitter, Rng& rng);
std::vector<Plane> box_planes(Vec3 centre, Vec3 half, float yaw);
// a peak: steep faces through (or just under) the top, down to floor_y
std::vector<Plane> peak_planes(Vec3 top, float floor_y, int sides, float slope_lo, float slope_hi,
                               Rng& rng);
// keep only the part above (or below) a height
void clip_below(std::vector<Plane>& planes, float y);
void clip_above(std::vector<Plane>& planes, float y);

// nearest hit of a ray within (0, t_max); `shadow` skips solids that cast no
// shadow and stops at the first hit
Hit cast(const Scene& scene, Vec3 origin, Vec3 dir, float t_max, bool shadow);
// the same against solids [first, first + count) only
Hit cast_among(const Scene& scene, Vec3 origin, Vec3 dir, float t_max, int first, int count);
Vec3 facet_normal(const Scene& scene, const Hit& hit);
