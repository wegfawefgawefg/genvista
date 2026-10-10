#include "palette.hpp"

// The first block is the wallpaper's own 28 colours (PIL max-coverage
// quantize of the 400x300 reference, most used first); the second fills in
// what crude lighting needs that a photo never shows: ink, pure white, lit
// grass, dirt, bark and the deepest blues.
static const char* const palette_hex[] = {
    "38acf5",
    "2f92e3",
    "081907",
    "cae3f9",
    "2577c5",
    "124568",
    "1e64a6",
    "185587",
    "e6f6fd",
    "0d3749",
    "0a2d0e",
    "aad2f2",
    "8fc1e7",
    "0f3d1e",
    "68c2f9",
    "77a7c0",
    "5191af",
    "2f7993",
    "1a6117",
    "59a7d8",
    "1f513f",
    "2b8635",
    "316864",
    "58a042",
    "4c7b38",
    "5c8884",
    "7e9c50",
    "bcbca1",
    // added
    "06101c",
    "ffffff",
    "6cc23a",
    "9ad04a",
    "c9ae7e",
    "8a7050",
    "3b2a1c",
    "0c3fae",
    "06205e",
    "b4c8e2",
    "0a2f8c",
    "1650c4",
    "d4e2f6",
    "9cb6e2",
    "86a2d6",
    "f2f8ff",
    "5a78b8",
    "4a68a8",
    "6f8cc8",
    "3a5898",
};

const std::vector<Vec3>& vista_palette() {
    static const std::vector<Vec3> palette = [] {
        std::vector<Vec3> out;
        for (const char* text : palette_hex) {
            out.push_back(hex(text));
        }
        return out;
    }();
    return palette;
}
