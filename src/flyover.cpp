#include "flyover.hpp"

#include "render.hpp"

#include <cstdio>
#include <filesystem>

// mostly a steady glide, easing in from a standstill and out to a hover
static float glide(float t) {
    const float smooth = t * t * (3.0f - 2.0f * t);
    return lerpf(t, smooth, 0.6f);
}

bool fly_over(Scene& scene, const FlyPath& path, const PictureStyle& style, int threads,
              const std::string& dir) {
    std::filesystem::create_directories(dir);
    const Vec3 start = scene.camera.eye;
    const int frames = path.frames < 2 ? 2 : path.frames;
    for (int i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(frames - 1);
        scene.camera.eye = Vec3{start.x, path.height, start.z + path.to_z * glide(t)};
        std::vector<Sample> samples = render(scene, threads);
        char name[64];
        std::snprintf(name, sizeof name, "/frame_%04d.png", i);
        const std::string file = dir + name;
        if (!write_picture(scene, samples, style, file.c_str())) {
            return false;
        }
        std::printf("\rframe %d / %d", i + 1, frames);
        std::fflush(stdout);
    }
    std::printf("\n");
    scene.camera.eye = start;
    return true;
}
