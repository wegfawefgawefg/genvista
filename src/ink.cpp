#include "ink.hpp"

#include "palette.hpp"

#include <cmath>

void hatch(const Scene& scene, std::vector<Sample>& samples, int w, int h) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Sample& s = samples[static_cast<std::size_t>(y * w + x)];
            if (s.solid < 0 || s.band > 0 || (x + y) % 3 != 0) {
                continue;
            }
            const Solid& solid = scene.solids[static_cast<std::size_t>(s.solid)];
            if (scene.materials[static_cast<std::size_t>(solid.material)].hatch) {
                s.rgb *= 0.8f;
            }
        }
    }
}

static const Material& material_of(const Scene& scene, int solid) {
    const Solid& s = scene.solids[static_cast<std::size_t>(solid)];
    return scene.materials[static_cast<std::size_t>(s.material)];
}

// is the neighbour something in front of this pixel that earns a line
static bool line_against(const Scene& scene, const Sample& here, const Sample& there) {
    if (there.solid < 0) {
        return true; // the sky behind an edge: outline it
    }
    if (there.solid == here.solid || there.reflected != here.reflected) {
        return false;
    }
    const Material& front = material_of(scene, there.solid);
    if (!front.outline) {
        return false;
    }
    const Solid& a = scene.solids[static_cast<std::size_t>(here.solid)];
    const Solid& b = scene.solids[static_cast<std::size_t>(there.solid)];
    if (a.material == b.material && !front.inner_lines) {
        return false;
    }
    const float gap = here.depth - there.depth;
    return gap > 0.35f + here.depth * 0.012f;
}

void ink_lines(const Scene& scene, std::vector<Sample>& samples, int w, int h) {
    std::vector<char> marked(samples.size(), 0);
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Sample& here = samples[static_cast<std::size_t>(y * w + x)];
            if (here.solid < 0 || !material_of(scene, here.solid).outline) {
                continue;
            }
            for (int k = 0; k < 4; ++k) {
                const int nx = x + dx[k];
                const int ny = y + dy[k];
                if (nx < 0 || ny < 0 || nx >= w || ny >= h) {
                    continue;
                }
                const Sample& there = samples[static_cast<std::size_t>(ny * w + nx)];
                // the waterline is not an edge of anything
                if (there.solid < 0 && there.reflected != here.reflected) {
                    continue;
                }
                if (line_against(scene, here, there)) {
                    marked[static_cast<std::size_t>(y * w + x)] = 1;
                    break;
                }
            }
        }
    }
    for (std::size_t i = 0; i < samples.size(); ++i) {
        if (!marked[i]) {
            continue;
        }
        Sample& s = samples[i];
        const Material& mat = material_of(scene, s.solid);
        Vec3 ink = mat.own_line ? mat.line : s.rgb * 0.45f;
        if (s.reflected) {
            ink = lerp(ink, scene.water.deep, 0.3f);
        }
        s.rgb = ink;
    }
}

static const float bayer4[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};

// luma plus two colour-opponent axes, hue shifts weighted to cost more
static Vec3 opponent(Vec3 c) {
    const float y = 0.3f * c.x + 0.59f * c.y + 0.11f * c.z;
    return Vec3{y * 1.6f, (c.x - c.y) * 1.1f, ((c.x + c.y) * 0.5f - c.z) * 0.9f};
}

static std::uint8_t to_byte(float v) {
    return static_cast<std::uint8_t>(v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v + 0.5f));
}

std::vector<std::uint8_t> quantize(const std::vector<Sample>& samples, int w, int h, float dither,
                                   bool use_palette) {
    const std::vector<Vec3>& palette = vista_palette();
    std::vector<Vec3> palette_o;
    for (const Vec3& c : palette) {
        palette_o.push_back(opponent(c));
    }
    std::vector<std::uint8_t> out(static_cast<std::size_t>(w * h * 3));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t i = static_cast<std::size_t>(y * w + x);
            const float offset = (bayer4[(y % 4) * 4 + x % 4] / 16.0f - 0.47f) * dither;
            Vec3 c = samples[i].rgb + Vec3{offset, offset, offset};
            if (use_palette) {
                const Vec3 o = opponent(c);
                float best = 1e30f;
                for (std::size_t p = 0; p < palette.size(); ++p) {
                    const Vec3 d = o - palette_o[p];
                    const float dist = dot(d, d);
                    if (dist < best) {
                        best = dist;
                        c = palette[p];
                    }
                }
            }
            out[i * 3 + 0] = to_byte(c.x);
            out[i * 3 + 1] = to_byte(c.y);
            out[i * 3 + 2] = to_byte(c.z);
        }
    }
    return out;
}
