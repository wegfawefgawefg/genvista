#include "picture.hpp"

#include "ink.hpp"

#include <cstdio>
#include <cstring>
#include <stb/stb_image_write.h>

// nearest-neighbour blow-up so the pixels stay pixels
static std::vector<std::uint8_t> upscale(const std::vector<std::uint8_t>& rgb, int w, int h,
                                         int scale) {
    const int ow = w * scale;
    std::vector<std::uint8_t> out(static_cast<std::size_t>(ow * h * scale * 3));
    for (int y = 0; y < h * scale; ++y) {
        for (int x = 0; x < ow; ++x) {
            const std::size_t from = static_cast<std::size_t>(((y / scale) * w + x / scale) * 3);
            const std::size_t to = static_cast<std::size_t>((y * ow + x) * 3);
            std::memcpy(out.data() + to, rgb.data() + from, 3);
        }
    }
    return out;
}

bool write_picture(const Scene& scene, std::vector<Sample>& samples, const PictureStyle& style,
                   const char* path) {
    const int w = scene.camera.w;
    const int h = scene.camera.h;

    // ink it and snap it to the palette
    hatch(scene, samples, w, h);
    ink_lines(scene, samples, w, h);
    const std::vector<std::uint8_t> rgb = quantize(samples, w, h, style.dither, style.palette);

    // blow it up and write it out
    const int scale = style.scale < 1 ? 1 : style.scale;
    const std::vector<std::uint8_t> big = upscale(rgb, w, h, scale);
    if (!stbi_write_png(path, w * scale, h * scale, 3, big.data(), w * scale * 3)) {
        std::fprintf(stderr, "genvista: could not write %s\n", path);
        return false;
    }
    return true;
}
