// genvista: the wallpaper's mountain lake, ray cast out of crude convex
// solids and drawn the way Mans In My House draws its sprites.
//
//   genvista [--w 640] [--h 480] [--scale 5] [--seed 1] [--threads N]
//            [--dither 10] [--no-palette] [--top 0.06] [--out out/vista.png]
//            [--fly FRAMES] [--fly-to 72] [--fly-height 0.6] [--frames-dir out/fly]
//
// The scene is laid out in a 4:3 frame. A wider image keeps the full width
// and shows a band of it starting --top of the way down. --fly writes a
// straight glide over the lake as numbered frames instead of one picture.

#include "flyover.hpp"
#include "picture.hpp"
#include "render.hpp"
#include "vista.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

struct Options {
    int w = 640;
    int h = 480;
    int scale = 5;
    int seed = 1;
    int threads = 0;
    float dither = 10.0f;
    float top = 0.06f;
    bool palette = true;
    std::string out = "out/vista.png";
    FlyPath fly;
    bool flying = false;
    std::string frames_dir = "out/fly";
};

static Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        const char* next = i + 1 < argc ? argv[i + 1] : nullptr;
        if (std::strcmp(arg, "--no-palette") == 0) {
            options.palette = false;
            continue;
        }
        if (next == nullptr) {
            std::fprintf(stderr, "genvista: %s wants a value\n", arg);
            std::exit(1);
        }
        if (std::strcmp(arg, "--w") == 0) {
            options.w = std::atoi(next);
        } else if (std::strcmp(arg, "--h") == 0) {
            options.h = std::atoi(next);
        } else if (std::strcmp(arg, "--scale") == 0) {
            options.scale = std::atoi(next);
        } else if (std::strcmp(arg, "--seed") == 0) {
            options.seed = std::atoi(next);
        } else if (std::strcmp(arg, "--threads") == 0) {
            options.threads = std::atoi(next);
        } else if (std::strcmp(arg, "--dither") == 0) {
            options.dither = static_cast<float>(std::atof(next));
        } else if (std::strcmp(arg, "--top") == 0) {
            options.top = static_cast<float>(std::atof(next));
        } else if (std::strcmp(arg, "--fly") == 0) {
            options.fly.frames = std::atoi(next);
            options.flying = true;
        } else if (std::strcmp(arg, "--fly-to") == 0) {
            options.fly.to_z = static_cast<float>(std::atof(next));
        } else if (std::strcmp(arg, "--fly-height") == 0) {
            options.fly.height = static_cast<float>(std::atof(next));
        } else if (std::strcmp(arg, "--frames-dir") == 0) {
            options.frames_dir = next;
        } else if (std::strcmp(arg, "--out") == 0) {
            options.out = next;
        } else {
            std::fprintf(stderr, "genvista: unknown option %s\n", arg);
            std::exit(1);
        }
        ++i;
    }
    if (options.threads <= 0) {
        options.threads = static_cast<int>(std::thread::hardware_concurrency());
    }
    return options;
}

int main(int argc, char** argv) {
    const Options options = parse_options(argc, argv);
    const auto start = std::chrono::steady_clock::now();

    // build the scene
    Scene scene;
    scene.camera.w = options.w;
    scene.camera.h = options.h;
    const float band =
        (4.0f / 3.0f) * static_cast<float>(options.h) / static_cast<float>(options.w);
    if (band < 1.0f) {
        scene.camera.view_v0 = options.top;
        scene.camera.view_v1 = options.top + band;
    }
    build_vista(scene, options.seed);

    const PictureStyle style{options.scale, options.dither, options.palette};
    if (options.flying) {
        return fly_over(scene, options.fly, style, options.threads, options.frames_dir) ? 0 : 1;
    }

    // trace every pixel
    std::vector<Sample> samples = render(scene, options.threads);
    const auto traced = std::chrono::steady_clock::now();

    // ink it, snap it to the palette, write it out
    if (!write_picture(scene, samples, style, options.out.c_str())) {
        return 1;
    }
    const auto done = std::chrono::steady_clock::now();
    const auto ms = [](auto a, auto b) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    };
    std::printf("%zu solids, %zu planes; traced %dx%d in %lld ms on %d threads, %lld ms total -> "
                "%s\n",
                scene.solids.size(), scene.planes.size(), options.w, options.h,
                static_cast<long long>(ms(start, traced)), options.threads,
                static_cast<long long>(ms(start, done)), options.out.c_str());
    return 0;
}
