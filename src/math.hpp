#pragma once
// Small float vectors for the tracer. World units are metres: +x right, +y up,
// +z away from the camera.

#include <cmath>

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec3 v3(float x, float y, float z) {
    return Vec3{x, y, z};
}

inline Vec3 operator+(Vec3 a, Vec3 b) {
    return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vec3 operator-(Vec3 a, Vec3 b) {
    return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vec3 operator-(Vec3 a) {
    return Vec3{-a.x, -a.y, -a.z};
}

inline Vec3 operator*(Vec3 a, float s) {
    return Vec3{a.x * s, a.y * s, a.z * s};
}

inline Vec3 operator*(float s, Vec3 a) {
    return Vec3{a.x * s, a.y * s, a.z * s};
}

// per component
inline Vec3 operator*(Vec3 a, Vec3 b) {
    return Vec3{a.x * b.x, a.y * b.y, a.z * b.z};
}

inline Vec3 operator/(Vec3 a, float s) {
    return Vec3{a.x / s, a.y / s, a.z / s};
}

inline Vec3& operator+=(Vec3& a, Vec3 b) {
    a = a + b;
    return a;
}

inline Vec3& operator*=(Vec3& a, float s) {
    a = a * s;
    return a;
}

inline float dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(Vec3 a, Vec3 b) {
    return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline float length(Vec3 a) {
    return std::sqrt(dot(a, a));
}

inline Vec3 normalize(Vec3 a) {
    const float len = length(a);
    return len > 0.0f ? a / len : a;
}

inline Vec3 lerp(Vec3 a, Vec3 b, float t) {
    return a + (b - a) * t;
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float clamp01(float t) {
    return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
}

inline float smooth01(float edge0, float edge1, float t) {
    const float k = clamp01((t - edge0) / (edge1 - edge0));
    return k * k * (3.0f - 2.0f * k);
}

inline Vec3 reflect(Vec3 dir, Vec3 normal) {
    return dir - normal * (2.0f * dot(dir, normal));
}

// a colour from "rrggbb", 0..255 per channel
inline Vec3 hex(const char* text) {
    auto digit = [](char c) { return c <= '9' ? c - '0' : (c | 32) - 'a' + 10; };
    float out[3];
    for (int i = 0; i < 3; ++i) {
        out[i] = static_cast<float>(digit(text[i * 2]) * 16 + digit(text[i * 2 + 1]));
    }
    return Vec3{out[0], out[1], out[2]};
}

// turning about the vertical axis
inline Vec3 yawed(Vec3 a, float yaw) {
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    return Vec3{a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
}

constexpr float pi = 3.14159265358979f;
