// The grass banks round the lake and the far peaks in the gap.

#include "solids.hpp"
#include "vista.hpp"

#include <cmath>

// a bank: a flattened lump sunk to half its height in the lake. Placed by
// where the photo has it: centred under image column u, z ahead, its top at
// image row v_top, u_half image widths across either way and z_half deep.
struct BankSpec {
    float u;
    float z;
    float v_top;
    float u_half;
    float z_half;
    float yaw;
};

static int add_bank(Scene& scene, int material, const BankSpec& spec, Rng& rng) {
    const Camera& camera = scene.camera;
    const float top = place(camera, spec.u, spec.v_top, spec.z).y;
    const float tall = top * 2.0f + 1.0f;
    Vec3 centre = place(camera, spec.u, spec.v_top, spec.z);
    centre.y = top - tall;
    const float wide = spec.u_half * 2.0f * camera.tan_x * spec.z;
    std::vector<Plane> shape =
        lump_planes(centre, Vec3{wide, tall, spec.z_half}, spec.yaw, 48, 0.04f, rng);
    clip_below(shape, -0.5f);
    const Vec3 inside{centre.x, top * 0.5f, centre.z};
    return add_solid(scene, shape, inside, material, static_cast<int>(rng_unit(rng) * 1e6f));
}

// where the ray through image (u, v) lands on one solid
static Vec3 on_solid(const Scene& scene, int solid, float u, float v) {
    const Camera& camera = scene.camera;
    const Vec3 dir = normalize(place(camera, u, v, 1.0f) - camera.eye);
    const Hit hit = cast_among(scene, camera.eye, dir, 1e5f, solid, 1);
    return camera.eye + dir * (hit.solid >= 0 ? hit.t : 100.0f);
}

void build_land(Scene& scene, const VistaMaterials& mats, Rng& rng) {
    const Camera& camera = scene.camera;

    // the left hill with the trodden path over its shoulder
    const int left = add_bank(scene, mats.grass, {0.02f, 110, 0.46f, 0.33f, 55, 0.15f}, rng);
    Solid& hill = scene.solids[static_cast<std::size_t>(left)];
    (void)camera;
    // the path, drawn where the photo has it and dropped onto the hill
    hill.path_a = on_solid(scene, left, -0.01f, 0.507f);
    hill.path_b = on_solid(scene, left, 0.125f, 0.477f);
    hill.path_width = 3.0f;
    add_bank(scene, mats.grass, {0.3f, 115, 0.505f, 0.18f, 45, -0.2f}, rng);
    add_bank(scene, mats.grass, {-0.15f, 90, 0.47f, 0.2f, 40, 0.0f}, rng);

    // the far shore across the middle
    add_bank(scene, mats.grass_far, {0.42f, 105, 0.522f, 0.08f, 30, 0.1f}, rng);
    add_bank(scene, mats.grass_far, {0.52f, 110, 0.524f, 0.08f, 30, -0.1f}, rng);
    add_bank(scene, mats.grass_far, {0.6f, 100, 0.52f, 0.07f, 28, 0.3f}, rng);
    // and the forest floor behind it, rising to the foot of the peaks
    add_bank(scene, mats.grass_far, {0.5f, 420, 0.518f, 0.25f, 160, 0.0f}, rng);

    // the right hill climbing out of frame
    add_bank(scene, mats.grass, {0.98f, 85, 0.46f, 0.25f, 40, -0.25f}, rng);
    add_bank(scene, mats.grass, {0.76f, 105, 0.505f, 0.14f, 35, 0.2f}, rng);
    add_bank(scene, mats.grass, {1.2f, 75, 0.44f, 0.2f, 35, 0.0f}, rng);
}

// a peak whose top lands at image (u, v), z ahead
static void peak_at(Scene& scene, int material, float u, float v, float z, float slope, Rng& rng) {
    const Vec3 top = place(scene.camera, u, v, z);
    std::vector<Plane> shape = peak_planes(top, -20.0f, 7 + static_cast<int>(rng_unit(rng) * 4.0f),
                                           slope * 0.7f, slope * 1.3f, rng);
    const Vec3 inside{top.x, (top.y - 20.0f) * 0.3f, top.z};
    add_solid(scene, shape, inside, material, static_cast<int>(rng_unit(rng) * 1e6f));
}

void build_mountains(Scene& scene, const VistaMaterials& mats, Rng& rng) {
    // the snowy pair in the gap, and the long ridges either side of them
    peak_at(scene, mats.mountain, 0.61f, 0.448f, 2600.0f, 0.55f, rng);
    peak_at(scene, mats.mountain, 0.572f, 0.46f, 2800.0f, 0.5f, rng);
    peak_at(scene, mats.mountain, 0.525f, 0.465f, 2500.0f, 0.4f, rng);
    peak_at(scene, mats.ridge, 0.47f, 0.474f, 2200.0f, 0.3f, rng);
    peak_at(scene, mats.ridge, 0.43f, 0.478f, 2000.0f, 0.28f, rng);
    peak_at(scene, mats.ridge, 0.555f, 0.482f, 1400.0f, 0.35f, rng);
    peak_at(scene, mats.ridge, 0.64f, 0.47f, 2000.0f, 0.4f, rng);
    // further off behind the trees on the right
    peak_at(scene, mats.mountain, 0.8f, 0.455f, 3200.0f, 0.45f, rng);
    peak_at(scene, mats.ridge, 0.71f, 0.466f, 2600.0f, 0.3f, rng);
}

float ground_y(const Scene& scene, float x, float z, int first, int count) {
    const Hit hit =
        cast_among(scene, Vec3{x, 500.0f, z}, Vec3{0.0f, -1.0f, 0.0f}, 1000.0f, first, count);
    if (hit.solid < 0) {
        return 0.0f;
    }
    const float y = 500.0f - hit.t;
    return y > 0.0f ? y : 0.0f;
}
