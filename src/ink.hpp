#pragma once
// The drawn look over the traced samples: hatching in the dark band, lines
// where one solid passes in front of another, outlines against the sky, then
// snapping to the palette with an ordered dither.

#include "shade.hpp"

#include <cstdint>
#include <vector>

void hatch(const Scene& scene, std::vector<Sample>& samples, int w, int h);
void ink_lines(const Scene& scene, std::vector<Sample>& samples, int w, int h);
// rgb bytes, w * h * 3
std::vector<std::uint8_t> quantize(const std::vector<Sample>& samples, int w, int h, float dither,
                                   bool use_palette);
