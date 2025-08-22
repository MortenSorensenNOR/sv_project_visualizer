#pragma once

#include <cstdint>
#include <stdexcept>
#include <cmath>

// ===== vec2 =====
struct vec2 {
    float x, y;

    vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    // Indexing
    float operator[](size_t index) const {
        switch (index) {
            case 0: return x;
            case 1: return y;
            default: throw std::runtime_error("Invalid index for vec2");
        }
    }
    float& operator[](size_t index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
            default: throw std::runtime_error("Invalid index for vec2");
        }
    }

    // Comparison
    bool operator==(const vec2& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const vec2& rhs) const { return !(*this == rhs); }

    // Arithmetic with another vec2
    vec2 operator+(const vec2& rhs) const { return vec2(x + rhs.x, y + rhs.y); }
    vec2 operator-(const vec2& rhs) const { return vec2(x - rhs.x, y - rhs.y); }
    vec2 operator*(const vec2& rhs) const { return vec2(x * rhs.x, y * rhs.y); }
    vec2 operator/(const vec2& rhs) const { return vec2(x / rhs.x, y / rhs.y); }

    // Arithmetic with scalar
    vec2 operator*(float s) const { return vec2(x * s, y * s); }
    vec2 operator/(float s) const { return vec2(x / s, y / s); }

    // Compound assignment
    vec2& operator+=(const vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    vec2& operator-=(const vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    vec2& operator*=(const vec2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
    vec2& operator/=(const vec2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

    vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    vec2& operator/=(float s) { x /= s; y /= s; return *this; }

    // Unary minus
    vec2 operator-() const { return vec2(-x, -y); }
};

// ===== vec3 =====
struct vec3 {
    float x, y, z;

    vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    // Indexing
    float operator[](size_t index) const {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::runtime_error("Invalid index for vec3");
        }
    }
    float& operator[](size_t index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::runtime_error("Invalid index for vec3");
        }
    }

    // Comparison
    bool operator==(const vec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const vec3& rhs) const { return !(*this == rhs); }

    // Arithmetic with another vec3
    vec3 operator+(const vec3& rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    vec3 operator-(const vec3& rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    vec3 operator*(const vec3& rhs) const { return vec3(x * rhs.x, y * rhs.y, z * rhs.z); }
    vec3 operator/(const vec3& rhs) const { return vec3(x / rhs.x, y / rhs.y, z / rhs.z); }

    // Arithmetic with scalar
    vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(float s) const { return vec3(x / s, y / s, z / s); }

    // Compound assignment
    vec3& operator+=(const vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    vec3& operator-=(const vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    vec3& operator*=(const vec3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
    vec3& operator/=(const vec3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

    vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    // Unary minus
    vec3 operator-() const { return vec3(-x, -y, -z); }
};

// ===== vec4 =====
struct vec4 {
    float x, y, z, w;

    vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        : x(x), y(y), z(z), w(w) {}

    // Indexing
    float operator[](size_t index) const {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            default: throw std::runtime_error("Invalid index for vec4");
        }
    }
    float& operator[](size_t index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            default: throw std::runtime_error("Invalid index for vec4");
        }
    }

    // Comparison
    bool operator==(const vec4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    bool operator!=(const vec4& rhs) const { return !(*this == rhs); }

    // Arithmetic with another vec4
    vec4 operator+(const vec4& rhs) const { return vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
    vec4 operator-(const vec4& rhs) const { return vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
    vec4 operator*(const vec4& rhs) const { return vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
    vec4 operator/(const vec4& rhs) const { return vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

    // Arithmetic with scalar
    vec4 operator*(float s) const { return vec4(x * s, y * s, z * s, w * s); }
    vec4 operator/(float s) const { return vec4(x / s, y / s, z / s, w / s); }

    // Compound assignment
    vec4& operator+=(const vec4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
    vec4& operator-=(const vec4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
    vec4& operator*=(const vec4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
    vec4& operator/=(const vec4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

    vec4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    vec4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    // Unary minus
    vec4 operator-() const { return vec4(-x, -y, -z, -w); }
};

// Scalar * vecN (commutative scalar multiplication)
inline vec2 operator*(float s, const vec2& v) { return v * s; }
inline vec3 operator*(float s, const vec3& v) { return v * s; }
inline vec4 operator*(float s, const vec4& v) { return v * s; }

struct AABB {
    vec2 ul; 
    vec2 br; 

    AABB() {
        ul = vec2(0, 0);
        br = vec2(0, 0);
    }

    AABB(float ul_x, float ul_y, float br_x, float br_y) {
        ul = vec2(ul_x, ul_y);
        br = vec2(br_x, br_y);
    }

    AABB(vec2 ul, vec2 br) {
        this->ul = ul;
        this->br = br;
    }
};
