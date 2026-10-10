#include "shade.hpp"

#include "noise.hpp"

#include <cmath>

static const float band_light[band_count] = {0.62f, 0.8f, 0.96f, 1.1f};

static int band_of(float lambert) {
    if (lambert < 0.18f) {
        return 0;
    }
    if (lambert < 0.5f) {
        return 1;
    }
    if (lambert < 0.82f) {
        return 2;
    }
    return 3;
}

static float segment_distance_xz(Vec3 p, Vec3 a, Vec3 b) {
    const float abx = b.x - a.x;
    const float abz = b.z - a.z;
    const float apx = p.x - a.x;
    const float apz = p.z - a.z;
    const float t = clamp01((apx * abx + apz * abz) / (abx * abx + abz * abz));
    const float dx = apx - abx * t;
    const float dz = apz - abz * t;
    return std::sqrt(dx * dx + dz * dz);
}

// the material's own colour at a point, before light
static Vec3 base_color(const Material& mat, const Solid& solid, Vec3 p, Vec3 normal, int facet) {
    const Vec3 local = p - solid.centre;
    Vec3 color = mat.color;
    switch (mat.pattern) {
    case Pattern::plain:
        break;
    case Pattern::blotch: {
        const float blot = fbm3(local, mat.pattern_scale, solid.seed + 5, 2);
        if (blot > mat.pattern_cut) {
            color = lerp(color, mat.alt, 0.75f);
        }
        break;
    }
    case Pattern::streak: {
        const Vec3 stretched{local.x * 2.5f, local.y * 0.5f, local.z * 2.5f};
        const float streak = noise3(stretched, mat.pattern_scale, solid.seed + 11);
        color = color * (0.82f + 0.36f * streak);
        if (fbm3(local, mat.pattern_scale * 3.0f, solid.seed + 13, 2) > mat.pattern_cut) {
            color = lerp(color, mat.alt, 0.6f);
        }
        break;
    }
    case Pattern::path: {
        const float wander = (noise3(p, mat.pattern_scale, solid.seed + 3) - 0.5f) * 0.6f;
        const float d = segment_distance_xz(p, solid.path_a, solid.path_b) /
                        (solid.path_width * (1.0f + wander));
        if (d < 1.0f && normal.y > 0.3f) {
            color = mat.alt;
        } else if (d < 1.25f && normal.y > 0.3f) {
            color = lerp(mat.alt, color, 0.5f) * 0.85f;
        }
        const float blot = fbm3(local, mat.pattern_scale * 2.0f, solid.seed + 5, 2);
        if (d >= 1.25f && blot > mat.pattern_cut) {
            color = color * 0.82f;
        }
        break;
    }
    case Pattern::snow: {
        const float patch = fbm3(p, mat.pattern_scale, solid.seed + 7, 3);
        const float line = mat.pattern_cut + (patch - 0.5f) * mat.pattern_cut * 0.6f;
        if (p.y > line && normal.y > 0.35f && patch > 0.48f) {
            color = mat.alt;
        }
        break;
    }
    case Pattern::billow: {
        // greyer and bluer under each puff's middle
        const float under = clamp01(-local.y / (solid.radius * 0.6f));
        color = lerp(color, mat.alt, under * 0.7f);
        break;
    }
    }
    const float grain = fbm3(local, mat.grain_scale, solid.seed, 3) - 0.5f;
    color = color * (1.0f + grain * 2.0f * mat.grain);
    const float facet_tint = 1.0f + (hash01(facet, solid.seed, 0, 77) - 0.5f) * 2.0f * mat.facet;
    return color * facet_tint;
}

Vec3 sky_color(const Scene& scene, Vec3 dir) {
    const Sky& sky = scene.sky;
    const float e = dir.y < 0.0f ? 0.0f : dir.y;
    Vec3 color;
    if (e < 0.06f) {
        color = lerp(sky.horizon, sky.low, e / 0.06f);
    } else if (e < 0.28f) {
        color = lerp(sky.low, sky.high, (e - 0.06f) / 0.22f);
    } else {
        color = lerp(sky.high, sky.zenith, clamp01((e - 0.28f) / 0.2f));
    }
    // deeper toward the upper left
    const float dark = clamp01(-dir.x * 1.2f + e * 1.4f - 0.15f) * sky.left_dark;
    return color * (1.0f - dark);
}

static Sample shade_hit(const Scene& scene, const Hit& hit, Vec3 origin, Vec3 dir) {
    const Solid& solid = scene.solids[static_cast<std::size_t>(hit.solid)];
    const Material& mat = scene.materials[static_cast<std::size_t>(solid.material)];
    const Vec3 p = origin + dir * hit.t;
    const Vec3 normal = facet_normal(scene, hit);
    const Vec3 base = base_color(mat, solid, p, normal, hit.facet);
    const float lambert = (dot(normal, scene.sun) + mat.wrap) / (1.0f + mat.wrap);
    int band = band_of(lambert);
    if (band > 0 && mat.casts) {
        const Hit blocker = cast(scene, p + normal * 0.02f, scene.sun, 1e5f, true);
        if (blocker.solid >= 0) {
            band = 0;
        }
    }
    const float level = static_cast<float>(band) / static_cast<float>(band_count - 1);
    const Vec3 tint = lerp(mat.shade_tint, Vec3{1.0f, 1.0f, 1.0f}, level);
    Vec3 rgb = base * band_light[band] * tint;
    const float haze = (1.0f - std::exp(-hit.t / scene.haze_distance)) * mat.haze;
    rgb = lerp(rgb, scene.sky.horizon, clamp01(haze));
    Sample sample;
    sample.rgb = rgb;
    sample.solid = hit.solid;
    sample.depth = hit.t;
    sample.band = band;
    return sample;
}

static Sample trace_dry(const Scene& scene, Vec3 origin, Vec3 dir, float t_max) {
    const Hit hit = cast(scene, origin, dir, t_max, false);
    if (hit.solid >= 0) {
        return shade_hit(scene, hit, origin, dir);
    }
    Sample sky;
    sky.rgb = sky_color(scene, dir);
    return sky;
}

Sample trace(const Scene& scene, Vec3 origin, Vec3 dir) {
    if (dir.y >= 0.0f || origin.y <= 0.0f) {
        return trace_dry(scene, origin, dir, 1e7f);
    }
    const float t_water = -origin.y / dir.y;
    const Hit hit = cast(scene, origin, dir, t_water, false);
    if (hit.solid >= 0) {
        return shade_hit(scene, hit, origin, dir);
    }
    // the lake: a nearly still mirror
    const Water& water = scene.water;
    const Vec3 p = origin + dir * t_water;
    const float s = water.ripple_scale;
    const Vec3 ripple{(noise3(p * Vec3{1.0f, 1.0f, 0.3f}, s, 91) - 0.5f) * water.ripple, 1.0f,
                      (noise3(p * Vec3{0.3f, 1.0f, 1.0f}, s, 92) - 0.5f) * water.ripple};
    Vec3 bounce = reflect(dir, normalize(ripple));
    if (bounce.y < 0.0005f) {
        bounce.y = 0.0005f;
        bounce = normalize(bounce);
    }
    Sample seen = trace_dry(scene, p + Vec3{0.0f, 0.001f, 0.0f}, bounce, 1e7f);
    const float steep = clamp01(-dir.y / 0.45f);
    const float mirror = lerpf(water.mirror_high, water.mirror_low, steep);
    // the darks go darker in it and the whites stay white, like a polariser
    const float luma = (0.3f * seen.rgb.x + 0.59f * seen.rgb.y + 0.11f * seen.rgb.z) / 255.0f;
    const float gain = lerpf(water.dark_gain, 1.0f, clamp01(luma * luma * 1.3f));
    seen.rgb = lerp(water.deep, seen.rgb * water.tint * gain, mirror);
    // long thin streaks: stretched across the view, thinning with distance
    const float streak = noise3(Vec3{p.x * 0.04f, 0.0f, p.z * 0.9f}, 1.0f, 93);
    if (streak > 0.74f) {
        seen.rgb = seen.rgb * (1.0f + water.sheen);
    }
    seen.depth += t_water;
    seen.reflected = true;
    return seen;
}
