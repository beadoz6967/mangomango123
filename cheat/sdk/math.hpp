// sdk/math.hpp
#pragma once
#include <cmath>

struct Vec3 {
    float x{}, y{}, z{};
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)       const { return {x*s, y*s, z*s}; }
    float Length() const { return std::sqrtf(x*x + y*y + z*z); }
};
