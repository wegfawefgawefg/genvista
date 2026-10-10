// The spruces: a trunk and a stack of ragged skirts each, set where the
// photo has them, plus the pale fallen log.

#include "solids.hpp"
#include "vista.hpp"

#include <cmath>

struct TreeSpot {
    float u;     // image column of the trunk
    float z;     // metres ahead
    float v_top; // image row of the tip
    float slim;  // crown radius over height
};

static int seed_of(Rng& rng) {
    return static_cast<int>(rng_unit(rng) * 1e6f);
}

static void spruce(Scene& scene, int crown, int bark, Vec3 base, float height, float slim,
                   Rng& rng) {
    const float radius = height * slim * 1.3f;
    const float trunk_half = 0.06f + radius * 0.06f;
    const std::vector<Plane> trunk =
        box_planes(base + Vec3{0.0f, height * 0.2f, 0.0f},
                   Vec3{trunk_half, height * 0.2f, trunk_half}, rng_range(rng, 0.0f, pi));
    add_solid(scene, trunk, base + Vec3{0.0f, height * 0.2f, 0.0f}, bark, seed_of(rng));

    const float bottom = base.y + height * 0.04f;
    const float top = base.y + height;
    int tiers = static_cast<int>(height / 1.1f);
    tiers = tiers < 6 ? 6 : (tiers > 16 ? 16 : tiers);
    const float step = (top - bottom) / static_cast<float>(tiers);
    const int seed = seed_of(rng);
    for (int i = 0; i < tiers; ++i) {
        const float t0 = static_cast<float>(i) / static_cast<float>(tiers);
        const float y0 = bottom + (top - bottom) * t0;
        const bool last = i == tiers - 1;
        float tier_h = last ? top - y0 : step * 1.9f;
        if (y0 + tier_h > top) {
            tier_h = top - y0;
        }
        const float r0 = radius * std::pow(1.0f - t0, 0.8f) * rng_range(rng, 0.8f, 1.15f) + 0.12f;
        const float r1 = last ? 0.02f : r0 * 0.18f;
        const Vec3 lean{rng_signed(rng) * radius * 0.08f, 0.0f, rng_signed(rng) * radius * 0.08f};
        const Vec3 at{base.x + lean.x, y0, base.z + lean.z};
        const std::vector<Plane> skirt =
            tier_planes(at, r0, tier_h, r1, 7, rng_range(rng, 0.0f, pi), 0.18f, rng);
        // one seed for the whole crown so the grain runs on across skirts
        add_solid(scene, skirt, at + Vec3{0.0f, tier_h * 0.3f, 0.0f}, crown, seed);
    }
}

// the bare dead stick standing in the right-hand group: a tapering pole
// thick enough to stay a pixel or two wide, a few scraps of needles clinging
// to it, never apart from it
static void snag(Scene& scene, int crown, int bark, Vec3 base, float height, Rng& rng) {
    const std::vector<Plane> pole = tier_planes(base, 0.32f, height, 0.06f, 5, 0.0f, 0.05f, rng);
    add_solid(scene, pole, base + Vec3{0.0f, height * 0.3f, 0.0f}, bark, seed_of(rng));
    for (int i = 0; i < 5; ++i) {
        const float up = 0.3f + 0.13f * static_cast<float>(i);
        const float side = (i % 2 == 0 ? 1.0f : -1.0f) * 0.3f;
        const Vec3 at{base.x + side, base.y + height * up, base.z};
        const float r = 1.4f - 0.2f * static_cast<float>(i);
        const std::vector<Plane> tuft = tier_planes(at, r, 1.3f, 0.2f, 6, 0.0f, 0.35f, rng);
        add_solid(scene, tuft, at + Vec3{0.0f, 0.3f, 0.0f}, crown, seed_of(rng));
    }
}

static const TreeSpot near_trees[] = {
    // the far left pair on the hill
    {0.045f, 122, 0.385f, 0.13f},
    {0.063f, 126, 0.35f, 0.12f},
    {0.08f, 121, 0.415f, 0.15f},
    // the tall left stand
    {0.135f, 100, 0.37f, 0.16f},
    {0.153f, 102, 0.33f, 0.15f},
    {0.17f, 96, 0.27f, 0.14f},
    {0.19f, 100, 0.285f, 0.15f},
    {0.207f, 94, 0.32f, 0.16f},
    {0.222f, 103, 0.42f, 0.18f},
    {0.233f, 98, 0.38f, 0.16f},
    // the middle-left stand
    {0.252f, 110, 0.33f, 0.15f},
    {0.272f, 114, 0.32f, 0.14f},
    {0.29f, 107, 0.315f, 0.16f},
    {0.307f, 112, 0.345f, 0.15f},
    {0.322f, 117, 0.4f, 0.17f},
    {0.334f, 110, 0.335f, 0.12f},
    {0.346f, 116, 0.385f, 0.15f},
    // lone ones toward the gap
    {0.376f, 158, 0.42f, 0.16f},
    {0.393f, 149, 0.375f, 0.1f},
    {0.416f, 166, 0.445f, 0.15f},
    {0.488f, 130, 0.43f, 0.22f},
    // the right-hand stand
    {0.624f, 108, 0.40f, 0.17f},
    {0.645f, 102, 0.33f, 0.14f},
    {0.662f, 110, 0.37f, 0.15f},
    {0.68f, 105, 0.34f, 0.15f},
    {0.697f, 112, 0.36f, 0.16f},
    {0.722f, 117, 0.37f, 0.15f},
    {0.758f, 105, 0.33f, 0.14f},
    {0.775f, 108, 0.335f, 0.15f},
    {0.792f, 116, 0.38f, 0.16f},
    {0.815f, 126, 0.36f, 0.11f},
    {0.836f, 130, 0.44f, 0.16f},
    // the big near ones on the right hill
    {0.885f, 74, 0.29f, 0.15f},
    {0.913f, 80, 0.36f, 0.16f},
    {0.94f, 70, 0.19f, 0.14f},
    {0.966f, 77, 0.32f, 0.15f},
    {0.988f, 72, 0.30f, 0.16f},
};

void build_trees(Scene& scene, const VistaMaterials& mats, Rng& rng, int land_first,
                 int land_count) {
    const Camera& camera = scene.camera;
    for (const TreeSpot& spot : near_trees) {
        Vec3 base = place(camera, spot.u, 0.5f, spot.z);
        base.y = ground_y(scene, base.x, base.z, land_first, land_count) - 0.2f;
        const float height = place(camera, spot.u, spot.v_top, spot.z).y - base.y;
        spruce(scene, mats.spruce, mats.bark, base, height, spot.slim, rng);
    }
    {
        Vec3 base = place(camera, 0.742f, 0.5f, 110.0f);
        base.y = ground_y(scene, base.x, base.z, land_first, land_count) - 0.2f;
        snag(scene, mats.spruce, mats.bark, base, place(camera, 0.742f, 0.29f, 110.0f).y - base.y,
             rng);
    }

    // the dark band of small trees along the far shore and up the slopes
    for (int i = 0; i < 70; ++i) {
        const float u = rng_range(rng, 0.36f, 0.66f);
        const float z = rng_range(rng, 200.0f, 450.0f);
        Vec3 base = place(camera, u, 0.5f, z);
        base.y = ground_y(scene, base.x, base.z, land_first, land_count) - 0.2f;
        const float height = rng_range(rng, 2.5f, 6.0f);
        spruce(scene, mats.spruce_far, mats.bark, base, height, rng_range(rng, 0.14f, 0.2f), rng);
    }

    // the fallen log, bleached, on the middle-left bank
    Vec3 log_at = place(camera, 0.315f, 0.5f, 105.0f);
    log_at.y = ground_y(scene, log_at.x, log_at.z, land_first, land_count) + 0.15f;
    const std::vector<Plane> log = box_planes(log_at, Vec3{2.4f, 0.28f, 0.3f}, 0.35f);
    add_solid(scene, log, log_at, mats.log, seed_of(rng));
}
