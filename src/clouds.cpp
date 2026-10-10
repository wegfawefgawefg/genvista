// The cumulus: each cloud a few hand-placed puffs, every puff grown into a
// cauliflower of smaller lumps, all cut flat along the cloud's base.

#include "solids.hpp"
#include "vista.hpp"

#include <vector>

struct Puff {
    float u;
    float v;
    float r; // radius in image widths
};

struct CloudSpec {
    float z;
    float v_base;
    std::vector<Puff> puffs;
};

static void add_lump(Scene& scene, int material, Vec3 centre, float r, float base_y, int planes,
                     Rng& rng) {
    std::vector<Plane> shape = lump_planes(
        centre, Vec3{r, r * rng_range(rng, 0.8f, 0.95f), r * rng_range(rng, 0.75f, 0.95f)},
        rng_range(rng, 0.0f, pi), planes, 0.07f, rng);
    clip_below(shape, base_y);
    Vec3 inside = centre;
    if (inside.y < base_y + r * 0.2f) {
        inside.y = base_y + r * 0.2f;
    }
    add_solid(scene, shape, inside, material, static_cast<int>(rng_unit(rng) * 1e6f));
}

// a puff and the smaller lumps boiling up out of its top and sides
static void add_puff(Scene& scene, int material, Vec3 centre, float r, float base_y, int depth,
                     Rng& rng) {
    add_lump(scene, material, centre, r, base_y, 24, rng);
    if (depth == 0) {
        return;
    }
    const int children = depth >= 2 ? 6 : 4;
    for (int i = 0; i < children; ++i) {
        const Vec3 dir = normalize(
            Vec3{rng_signed(rng), rng_range(rng, 0.15f, 1.0f), rng_range(rng, -0.9f, 0.3f)});
        const float child_r = r * rng_range(rng, 0.38f, 0.55f);
        const Vec3 at = centre + dir * (r * rng_range(rng, 0.65f, 0.85f));
        add_puff(scene, material, at, child_r, base_y, depth - 1, rng);
    }
}

static void add_cloud(Scene& scene, int material, const CloudSpec& spec, Rng& rng) {
    const Camera& camera = scene.camera;
    const float width = 2.0f * camera.tan_x * spec.z;
    const float base_y = place(camera, 0.5f, spec.v_base, spec.z).y;
    for (const Puff& puff : spec.puffs) {
        const float z = spec.z + rng_signed(rng) * puff.r * width * 0.6f;
        const Vec3 centre = place(camera, puff.u, puff.v, z);
        const int depth = puff.r > 0.044f ? 3 : (puff.r > 0.028f ? 2 : 1);
        add_puff(scene, material, centre, puff.r * width * (z / spec.z), base_y, depth, rng);
    }
}

void build_clouds(Scene& scene, const VistaMaterials& mats, Rng& rng) {
    const std::vector<CloudSpec> clouds = {
        // the great tower left of middle
        {6000.0f, 0.47f, {{0.47f, 0.06f, 0.05f},  {0.53f, 0.05f, 0.055f},  {0.58f, 0.075f, 0.045f},
                          {0.61f, 0.1f, 0.03f},   {0.44f, 0.115f, 0.035f}, {0.5f, 0.125f, 0.05f},
                          {0.56f, 0.135f, 0.04f}, {0.27f, 0.2f, 0.04f},    {0.31f, 0.15f, 0.045f},
                          {0.36f, 0.14f, 0.04f},  {0.33f, 0.22f, 0.05f},   {0.4f, 0.185f, 0.035f},
                          {0.28f, 0.3f, 0.05f},   {0.34f, 0.3f, 0.055f},   {0.4f, 0.28f, 0.04f},
                          {0.3f, 0.38f, 0.05f},   {0.38f, 0.36f, 0.05f},   {0.44f, 0.38f, 0.04f},
                          {0.47f, 0.4f, 0.035f},  {0.53f, 0.42f, 0.04f},   {0.58f, 0.43f, 0.035f},
                          {0.62f, 0.44f, 0.03f},  {0.5f, 0.45f, 0.03f}}},
        // the big one on the right
        {6500.0f,
         0.46f,
         {{0.62f, 0.3f, 0.035f},
          {0.66f, 0.26f, 0.045f},
          {0.7f, 0.24f, 0.04f},
          {0.74f, 0.28f, 0.04f},
          {0.77f, 0.32f, 0.03f},
          {0.68f, 0.33f, 0.045f},
          {0.64f, 0.37f, 0.03f},
          {0.72f, 0.36f, 0.035f}}},
        // small ones high on the right
        {7000.0f, 0.225f, {{0.79f, 0.2f, 0.022f}, {0.82f, 0.185f, 0.025f}, {0.85f, 0.2f, 0.02f}}},
        {7000.0f, 0.265f, {{0.86f, 0.25f, 0.018f}, {0.88f, 0.245f, 0.012f}}},
        {7000.0f, 0.335f, {{0.82f, 0.31f, 0.02f}, {0.845f, 0.315f, 0.015f}}},
        {7000.0f, 0.42f, {{0.85f, 0.395f, 0.025f}, {0.88f, 0.4f, 0.02f}}},
        // the bank on the left edge
        {6500.0f,
         0.45f,
         {{0.02f, 0.28f, 0.035f},
          {0.06f, 0.3f, 0.035f},
          {0.1f, 0.33f, 0.03f},
          {0.03f, 0.36f, 0.035f},
          {0.08f, 0.38f, 0.03f},
          {0.12f, 0.4f, 0.025f}}},
        // along the top right edge
        {6000.0f,
         0.045f,
         {{0.83f, 0.0f, 0.03f},
          {0.88f, 0.01f, 0.03f},
          {0.93f, 0.0f, 0.035f},
          {0.97f, 0.02f, 0.03f}}},
    };
    for (const CloudSpec& spec : clouds) {
        add_cloud(scene, mats.cloud, spec, rng);
    }
}
