#include "solids.hpp"

#include <cmath>
#include <cstdio>

static Vec3 fibonacci_dir(int i, int count) {
    const float golden = pi * (3.0f - std::sqrt(5.0f));
    const float y = 1.0f - (static_cast<float>(i) + 0.5f) / static_cast<float>(count) * 2.0f;
    const float r = std::sqrt(1.0f - y * y);
    const float a = golden * static_cast<float>(i);
    return Vec3{std::cos(a) * r, y, std::sin(a) * r};
}

int add_material(Scene& scene, const Material& material) {
    scene.materials.push_back(material);
    return static_cast<int>(scene.materials.size()) - 1;
}

// how far from `from` the solid's surface is along `dir`
static float exit_distance(const std::vector<Plane>& planes, Vec3 from, Vec3 dir) {
    float best = 1e9f;
    for (const Plane& plane : planes) {
        const float den = dot(plane.normal, dir);
        if (den <= 1e-6f) {
            continue;
        }
        const float t = (plane.offset - dot(plane.normal, from)) / den;
        if (t < best) {
            best = t;
        }
    }
    return best;
}

// the farthest corner from `from`: every point where three planes meet that
// lies inside the rest. Probing outward from inside misses the ends of long
// thin solids (a pole lost its top that way), so the corners are found exactly
static float corner_reach(const std::vector<Plane>& planes, Vec3 from) {
    const int count = static_cast<int>(planes.size());
    float reach = -1.0f;
    for (int a = 0; a < count; ++a) {
        for (int b = a + 1; b < count; ++b) {
            const Vec3 ab = cross(planes[a].normal, planes[b].normal);
            for (int c = b + 1; c < count; ++c) {
                const float det = dot(ab, planes[c].normal);
                if (std::fabs(det) < 1e-6f) {
                    continue;
                }
                const Vec3 p = (cross(planes[b].normal, planes[c].normal) * planes[a].offset +
                                cross(planes[c].normal, planes[a].normal) * planes[b].offset +
                                ab * planes[c].offset) /
                               det;
                bool inside = true;
                for (int k = 0; k < count && inside; ++k) {
                    inside = dot(planes[k].normal, p) <= planes[k].offset + 1e-3f;
                }
                const float d = length(p - from);
                if (inside && d > reach) {
                    reach = d;
                }
            }
        }
    }
    return reach;
}

int add_solid(Scene& scene, const std::vector<Plane>& planes, Vec3 inside, int material, int seed) {
    float reach = corner_reach(planes, inside);
    if (reach < 0.0f) {
        // no corners found: fall back to probing outward
        const int probes = 400;
        for (int i = 0; i < probes; ++i) {
            const float t = exit_distance(planes, inside, fibonacci_dir(i, probes));
            if (t > reach) {
                reach = t;
            }
        }
    }
    if (reach > 1e8f) {
        std::fprintf(stderr, "genvista: an unbounded solid near %.1f %.1f %.1f\n", inside.x,
                     inside.y, inside.z);
    }
    Solid solid;
    solid.first_plane = static_cast<int>(scene.planes.size());
    solid.plane_count = static_cast<int>(planes.size());
    solid.centre = inside;
    solid.radius = reach * 1.001f + 0.01f;
    solid.material = material;
    solid.seed = seed;
    scene.planes.insert(scene.planes.end(), planes.begin(), planes.end());
    scene.solids.push_back(solid);
    return static_cast<int>(scene.solids.size()) - 1;
}

std::vector<Plane> lump_planes(Vec3 centre, Vec3 radii, float yaw, int count, float jitter,
                               Rng& rng) {
    std::vector<Plane> planes;
    planes.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        // spread the touching points over the surface, not the normals over
        // the sphere: a flat lump would otherwise grow a tall tent of a roof
        Vec3 on = fibonacci_dir(i, count);
        on = normalize(on + Vec3{rng_signed(rng), rng_signed(rng), rng_signed(rng)} * 0.18f);
        const Vec3 local = normalize(Vec3{on.x / radii.x, on.y / radii.y, on.z / radii.z});
        // an ellipsoid's support along a normal
        const float support = std::sqrt(radii.x * radii.x * local.x * local.x +
                                        radii.y * radii.y * local.y * local.y +
                                        radii.z * radii.z * local.z * local.z);
        const Vec3 normal = yawed(local, yaw);
        const float push = support * (1.0f + jitter * rng_signed(rng));
        planes.push_back(Plane{normal, dot(normal, centre) + push});
    }
    return planes;
}

std::vector<Plane> tier_planes(Vec3 base, float r0, float height, float r1, int sides, float twist,
                               float jitter, Rng& rng) {
    std::vector<Plane> planes;
    const float slope = (r0 - r1) / height;
    for (int i = 0; i < sides; ++i) {
        const float a = twist + (static_cast<float>(i) + 0.35f * rng_signed(rng)) /
                                    static_cast<float>(sides) * 2.0f * pi;
        const Vec3 out{std::cos(a), 0.0f, std::sin(a)};
        const float r = r0 * (1.0f + jitter * rng_signed(rng));
        const Vec3 normal = normalize(Vec3{out.x, slope * (1.0f + 0.3f * rng_signed(rng)), out.z});
        const Vec3 rim = base + out * r;
        planes.push_back(Plane{normal, dot(normal, rim)});
    }
    planes.push_back(Plane{Vec3{0.0f, -1.0f, 0.0f}, -base.y});
    planes.push_back(Plane{Vec3{0.0f, 1.0f, 0.0f}, base.y + height});
    return planes;
}

std::vector<Plane> box_planes(Vec3 centre, Vec3 half, float yaw) {
    const Vec3 axes[3] = {yawed(Vec3{1, 0, 0}, yaw), Vec3{0, 1, 0}, yawed(Vec3{0, 0, 1}, yaw)};
    const float extent[3] = {half.x, half.y, half.z};
    std::vector<Plane> planes;
    for (int i = 0; i < 3; ++i) {
        planes.push_back(Plane{axes[i], dot(axes[i], centre) + extent[i]});
        planes.push_back(Plane{-axes[i], -dot(axes[i], centre) + extent[i]});
    }
    return planes;
}

std::vector<Plane> peak_planes(Vec3 top, float floor_y, int sides, float slope_lo, float slope_hi,
                               Rng& rng) {
    std::vector<Plane> planes;
    const float spin = rng_range(rng, 0.0f, 2.0f * pi);
    for (int i = 0; i < sides; ++i) {
        const float a = spin + (static_cast<float>(i) + 0.4f * rng_signed(rng)) /
                                   static_cast<float>(sides) * 2.0f * pi;
        const float s = rng_range(rng, slope_lo, slope_hi);
        const Vec3 normal = normalize(Vec3{std::cos(a) * s, 1.0f, std::sin(a) * s});
        // some faces pass a little under the top, which blunts it into a ridge
        const float drop = rng_unit(rng) < 0.4f ? rng_range(rng, 0.0f, 0.12f) : 0.0f;
        const float height = top.y - floor_y;
        planes.push_back(Plane{normal, dot(normal, top - Vec3{0.0f, drop * height, 0.0f})});
    }
    planes.push_back(Plane{Vec3{0.0f, -1.0f, 0.0f}, -floor_y});
    return planes;
}

void clip_below(std::vector<Plane>& planes, float y) {
    planes.push_back(Plane{Vec3{0.0f, -1.0f, 0.0f}, -y});
}

void clip_above(std::vector<Plane>& planes, float y) {
    planes.push_back(Plane{Vec3{0.0f, 1.0f, 0.0f}, y});
}

// slab test of one solid; returns the entry distance or -1
static float cast_solid(const Scene& scene, const Solid& solid, Vec3 origin, Vec3 dir, int& facet) {
    float t_in = -1e30f;
    float t_out = 1e30f;
    int entered = -1;
    const Plane* planes = scene.planes.data() + solid.first_plane;
    for (int i = 0; i < solid.plane_count; ++i) {
        const float num = planes[i].offset - dot(planes[i].normal, origin);
        const float den = dot(planes[i].normal, dir);
        if (std::fabs(den) < 1e-9f) {
            if (num < 0.0f) {
                return -1.0f;
            }
            continue;
        }
        const float t = num / den;
        if (den < 0.0f) {
            if (t > t_in) {
                t_in = t;
                entered = i;
            }
        } else if (t < t_out) {
            t_out = t;
        }
        if (t_in > t_out) {
            return -1.0f;
        }
    }
    if (t_in <= 1e-4f) {
        return -1.0f;
    }
    facet = entered;
    return t_in;
}

Hit cast(const Scene& scene, Vec3 origin, Vec3 dir, float t_max, bool shadow) {
    Hit best;
    best.t = t_max;
    const int count = static_cast<int>(scene.solids.size());
    for (int i = 0; i < count; ++i) {
        const Solid& solid = scene.solids[static_cast<std::size_t>(i)];
        if (shadow && !scene.materials[static_cast<std::size_t>(solid.material)].casts) {
            continue;
        }
        // bounding sphere first
        const Vec3 to = solid.centre - origin;
        const float along = dot(to, dir);
        const float miss2 = dot(to, to) - along * along;
        const float r2 = solid.radius * solid.radius;
        if (miss2 > r2) {
            continue;
        }
        const float half = std::sqrt(r2 - miss2);
        if (along + half < 0.0f || along - half > best.t) {
            continue;
        }
        int facet = -1;
        const float t = cast_solid(scene, solid, origin, dir, facet);
        if (t > 0.0f && t < best.t) {
            best.t = t;
            best.solid = i;
            best.facet = facet;
            if (shadow) {
                return best;
            }
        }
    }
    return best;
}

Hit cast_among(const Scene& scene, Vec3 origin, Vec3 dir, float t_max, int first, int count) {
    Hit best;
    best.t = t_max;
    for (int i = first; i < first + count; ++i) {
        int facet = -1;
        const float t =
            cast_solid(scene, scene.solids[static_cast<std::size_t>(i)], origin, dir, facet);
        if (t > 0.0f && t < best.t) {
            best.t = t;
            best.solid = i;
            best.facet = facet;
        }
    }
    return best;
}

Vec3 facet_normal(const Scene& scene, const Hit& hit) {
    const Solid& solid = scene.solids[static_cast<std::size_t>(hit.solid)];
    return scene.planes[static_cast<std::size_t>(solid.first_plane + hit.facet)].normal;
}
