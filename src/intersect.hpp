#pragma once
#include "parser.hpp"
#include <vector>

// SAT-based triangle-AABB intersection test. Returns true if ANY triangle in 'faces' overlaps the box [boxMin, boxMax].
bool triBoxIntersect(const Vec3& boxMin, const Vec3& boxMax,
                     const std::vector<Vec3>& verts,
                     const std::vector<Face>& faces);
