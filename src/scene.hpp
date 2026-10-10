#pragma once
// The scene: convex solids (each an intersection of half-spaces), the
// materials they are painted with, a flat lake at y = 0, the sky and the sun.

#include "math.hpp"

#include <vector>

// inside where dot(normal, p) <= offset
struct Plane {
    Vec3 normal;
    float offset = 0.0f;
};

enum class Pattern {
    plain,
    blotch, // patches of alt
    streak, // long vertical grain, needles and bark
    path,   // a trodden band of alt across the top (grass banks)
    snow,   // alt on the high, upward faces (peaks)
    billow, // white tops going grey-blue toward the base (clouds)
};

struct Material {
    Vec3 color;
    Vec3 alt;
    Vec3 line;             // ink for edges drawn on this material
    bool own_line = false; // false: edges are the colour darkened
    float grain = 0.16f;   // strength of the 3D noise
    float grain_scale = 1.0f;
    Pattern pattern = Pattern::plain;
    float pattern_scale = 1.0f;
    float pattern_cut = 0.6f;
    float facet = 0.06f;     // per-facet brightness jitter
    float haze = 1.0f;       // share of the aerial haze it takes
    float wrap = 0.0f;       // light wrapping round the shoulders: more of it in the bright bands
    bool casts = true;       // throws shadows
    bool outline = true;     // gets depth lines
    bool inner_lines = true; // also between two solids of this material
    bool hatch = true;       // hatched in its darkest band
    Vec3 shade_tint = Vec3{0.78f, 0.86f, 1.08f}; // what the dark bands lean toward
};

struct Solid {
    int first_plane = 0;
    int plane_count = 0;
    Vec3 centre; // also the origin of its grain
    float radius = 0.0f;
    int material = 0;
    int seed = 0;
    // pattern data: path line (a to b in xz) and width, or snow line height
    Vec3 path_a;
    Vec3 path_b;
    float path_width = 0.0f;
};

struct Camera {
    Vec3 eye;
    float tan_x = 0.6f; // half-width of the view at depth 1
    float tan_y = 0.45f;
    float horizon_v = 0.5f; // the image row at eye level (a shifted lens, not a tilt)
    // the scene is laid out in a 4:3 frame; a wider image shows the band of
    // rows view_v0..view_v1 of it, the full width always
    float view_v0 = 0.0f;
    float view_v1 = 1.0f;
    int w = 640;
    int h = 480;
    float wobble = 0.35f; // pixels of hand-drawn drift in the ray grid
    int seed = 1;
};

struct Sky {
    Vec3 zenith;  // straight up, on the left (the deepest)
    Vec3 high;    // upper sky
    Vec3 low;     // a little above the horizon
    Vec3 horizon; // at it
    float left_dark = 0.25f;
};

struct Water {
    Vec3 deep;                 // what it is straight down
    Vec3 tint;                 // what reflections are multiplied by
    float mirror_low = 0.55f;  // reflected share looking steeply down
    float mirror_high = 0.95f; // at a graze
    float ripple = 0.004f;     // normal wobble
    float ripple_scale = 1.5f;
    float sheen = 0.12f;
    float dark_gain = 0.6f; // what dark reflections are multiplied by; bright ones keep all // the
                            // faint long streaks of breeze across it
};

struct Scene {
    Camera camera;
    std::vector<Plane> planes;
    std::vector<Solid> solids;
    std::vector<Material> materials;
    Vec3 sun; // toward the sun
    Sky sky;
    Water water;
    float haze_distance = 6000.0f;
};

// the world point that lands at image (u, v), 0..1 from the top left, z ahead
Vec3 place(const Camera& camera, float u, float v, float z);
float image_v(const Camera& camera, Vec3 p);
