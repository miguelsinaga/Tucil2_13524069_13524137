#pragma once
#include <vector>
#include <string>
#include <stdexcept>

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
};

struct Face {
    int v1, v2, v3; // 0-indexed
};

struct OBJModel {
    std::vector<Vec3> verts;
    std::vector<Face> faces;
};

// Parse a .obj file. Throws std::runtime_error on invalid input.
OBJModel parseOBJ(const std::string& path);
