#pragma once
// The wallpaper's lake: built by hand from where things sit in the photo,
// with the lumps, skirts and puffs themselves grown from the seed.

#include "noise.hpp"
#include "scene.hpp"

struct VistaMaterials {
    int cloud = 0;
    int spruce = 0;
    int spruce_far = 0;
    int bark = 0;
    int grass = 0;
    int grass_far = 0;
    int mountain = 0;
    int ridge = 0;
    int log = 0;
};

void build_vista(Scene& scene, int seed);

// the parts, each in its own file
void build_land(Scene& scene, const VistaMaterials& mats, Rng& rng);
void build_mountains(Scene& scene, const VistaMaterials& mats, Rng& rng);
void build_trees(Scene& scene, const VistaMaterials& mats, Rng& rng, int land_first,
                 int land_count);
void build_clouds(Scene& scene, const VistaMaterials& mats, Rng& rng);

// the height of the land under (x, z) among solids [first, first + count), or 0
float ground_y(const Scene& scene, float x, float z, int first, int count);
