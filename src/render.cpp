#include "render.hpp"

#include "noise.hpp"

#include <atomic>
#include <thread>

Vec3 place(const Camera& camera, float u, float v, float z) {
    return Vec3{camera.eye.x + (2.0f * u - 1.0f) * camera.tan_x * z,
                camera.eye.y + 2.0f * (camera.horizon_v - v) * camera.tan_y * z, camera.eye.z + z};
}

float image_v(const Camera& camera, Vec3 p) {
    const float z = p.z - camera.eye.z;
    return camera.horizon_v - (p.y - camera.eye.y) / (z * camera.tan_y) * 0.5f;
}

Vec3 camera_ray(const Camera& camera, float px, float py) {
    const float u = px / static_cast<float>(camera.w);
    const float v = lerpf(camera.view_v0, camera.view_v1, py / static_cast<float>(camera.h));
    return normalize(
        Vec3{(2.0f * u - 1.0f) * camera.tan_x, 2.0f * (camera.horizon_v - v) * camera.tan_y, 1.0f});
}

// the hand-drawn drift of the ray grid: smooth, a few dozen pixels across,
// sized to the image so a bigger render wobbles the same
static void wobble_at(const Camera& camera, int x, int y, float& dx, float& dy) {
    const float cell = static_cast<float>(camera.w) / 28.0f;
    const float fx = static_cast<float>(x) / cell;
    const float fy = static_cast<float>(y) / cell;
    const float amount = camera.wobble * static_cast<float>(camera.w) / 640.0f;
    dx = (noise2(fx, fy, camera.seed + 1) - 0.5f) * 2.0f * amount;
    dy = (noise2(fx, fy, camera.seed + 2) - 0.5f) * 2.0f * amount;
}

std::vector<Sample> render(const Scene& scene, int threads) {
    const Camera& camera = scene.camera;
    std::vector<Sample> samples(static_cast<std::size_t>(camera.w * camera.h));
    std::atomic<int> next_row{0};
    auto work = [&]() {
        for (int y = next_row++; y < camera.h; y = next_row++) {
            for (int x = 0; x < camera.w; ++x) {
                float dx = 0.0f;
                float dy = 0.0f;
                wobble_at(camera, x, y, dx, dy);
                const Vec3 dir = camera_ray(camera, static_cast<float>(x) + 0.5f + dx,
                                            static_cast<float>(y) + 0.5f + dy);
                samples[static_cast<std::size_t>(y * camera.w + x)] = trace(scene, camera.eye, dir);
            }
        }
    };
    std::vector<std::thread> pool;
    for (int i = 0; i < threads; ++i) {
        pool.emplace_back(work);
    }
    for (std::thread& thread : pool) {
        thread.join();
    }
    return samples;
}
