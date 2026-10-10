#include "vista.hpp"

#include "solids.hpp"

#include <cmath>

static VistaMaterials add_materials(Scene& scene) {
    VistaMaterials mats;

    Material cloud;
    cloud.color = hex("ffffff");
    cloud.alt = hex("c8dcf0");
    cloud.line = hex("b4c8e2");
    cloud.own_line = true;
    cloud.grain = 0.04f;
    cloud.grain_scale = 60.0f;
    cloud.pattern = Pattern::billow;
    cloud.facet = 0.0f;
    cloud.wrap = 0.45f;
    cloud.inner_lines = false;
    cloud.hatch = false;
    cloud.haze = 0.2f;
    cloud.casts = false;
    cloud.shade_tint = Vec3{0.84f, 0.94f, 1.08f};
    mats.cloud = add_material(scene, cloud);

    Material spruce;
    spruce.color = hex("10381a");
    spruce.alt = hex("235e28");
    spruce.line = hex("06140a");
    spruce.own_line = true;
    spruce.grain = 0.22f;
    spruce.grain_scale = 0.7f;
    spruce.pattern = Pattern::streak;
    spruce.pattern_scale = 0.5f;
    spruce.pattern_cut = 0.6f;
    spruce.facet = 0.08f;
    spruce.shade_tint = Vec3{0.7f, 0.85f, 1.0f};
    mats.spruce = add_material(scene, spruce);

    Material far = spruce;
    far.color = hex("1c4826");
    far.line = hex("143a22");
    mats.spruce_far = add_material(scene, far);

    Material bark;
    bark.color = hex("4a3524");
    bark.line = hex("140c06");
    bark.own_line = true;
    bark.grain = 0.2f;
    bark.grain_scale = 0.2f;
    bark.pattern = Pattern::streak;
    bark.pattern_scale = 0.15f;
    bark.pattern_cut = 0.9f;
    mats.bark = add_material(scene, bark);

    Material grass;
    grass.color = hex("45a02a");
    grass.alt = hex("c9ae7e");
    grass.line = hex("1f4a14");
    grass.own_line = true;
    grass.grain = 0.16f;
    grass.grain_scale = 2.5f;
    grass.pattern = Pattern::path;
    grass.pattern_scale = 3.0f;
    grass.pattern_cut = 0.55f;
    grass.facet = 0.07f;
    grass.shade_tint = Vec3{0.72f, 0.88f, 0.95f};
    mats.grass = add_material(scene, grass);

    Material grass_far = grass;
    grass_far.color = hex("4c9a2c");
    grass_far.pattern = Pattern::blotch;
    grass_far.alt = hex("2e7428");
    grass_far.pattern_scale = 6.0f;
    mats.grass_far = add_material(scene, grass_far);

    Material mountain;
    mountain.color = hex("3a7a4c");
    mountain.alt = hex("e4ecee");
    mountain.line = hex("2a5a50");
    mountain.own_line = true;
    mountain.grain = 0.14f;
    mountain.grain_scale = 30.0f;
    mountain.pattern = Pattern::snow;
    mountain.pattern_scale = 25.0f;
    mountain.pattern_cut = 70.0f;
    mountain.facet = 0.1f;
    mountain.casts = false;
    mountain.haze = 0.6f;
    mats.mountain = add_material(scene, mountain);

    Material ridge = mountain;
    ridge.color = hex("2f6a44");
    ridge.pattern = Pattern::blotch;
    ridge.alt = hex("24583a");
    ridge.pattern_scale = 40.0f;
    mats.ridge = add_material(scene, ridge);

    Material log;
    log.color = hex("d8d2c0");
    log.line = hex("4a4436");
    log.own_line = true;
    log.grain = 0.12f;
    log.grain_scale = 0.3f;
    log.pattern = Pattern::streak;
    log.pattern_scale = 0.2f;
    log.pattern_cut = 0.9f;
    mats.log = add_material(scene, log);

    return mats;
}

void build_vista(Scene& scene, int seed) {
    Camera& camera = scene.camera;
    // crouched at the water's edge, the horizon a little under the middle
    camera.eye = Vec3{0.0f, 0.6f, 0.0f};
    camera.horizon_v = 0.525f;
    camera.tan_y = std::tan(25.0f * pi / 180.0f);
    camera.tan_x = camera.tan_y * 4.0f / 3.0f;
    camera.seed = seed;

    // high summer sun, behind the left shoulder
    scene.sun = normalize(Vec3{-0.4f, 0.7f, -0.6f});

    scene.sky.zenith = hex("1250c0");
    scene.sky.high = hex("2a8ce6");
    scene.sky.low = hex("4ab4f8");
    scene.sky.horizon = hex("c4e4fa");
    scene.sky.left_dark = 0.22f;

    scene.water.deep = hex("041a52");
    scene.water.tint = Vec3{0.9f, 0.94f, 1.0f};
    scene.water.mirror_low = 0.45f;
    scene.water.mirror_high = 1.0f;

    const VistaMaterials mats = add_materials(scene);
    Rng rng = rng_from(seed);
    const int land_first = static_cast<int>(scene.solids.size());
    build_land(scene, mats, rng);
    const int land_count = static_cast<int>(scene.solids.size()) - land_first;
    build_mountains(scene, mats, rng);
    build_trees(scene, mats, rng, land_first, land_count);
    build_clouds(scene, mats, rng);
}
