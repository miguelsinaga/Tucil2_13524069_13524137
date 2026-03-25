#include "intersect.hpp"
#include <cmath>
#include <algorithm>

static double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static Vec3 sub(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static void projectTriangle(const Vec3& axis, const Vec3& v0, const Vec3& v1, const Vec3& v2,
                             double& outMin, double& outMax) {
    double p0 = dot(axis, v0);
    double p1 = dot(axis, v1);
    double p2 = dot(axis, v2);
    outMin = std::min({p0, p1, p2});
    outMax = std::max({p0, p1, p2});
}

static void projectBox(const Vec3& axis, const Vec3& half,
                       double& outMin, double& outMax) {
    // Box is centered at origin (we already translated verts)
    double r = std::abs(dot(axis, Vec3(half.x, 0, 0))) +
               std::abs(dot(axis, Vec3(0, half.y, 0))) +
               std::abs(dot(axis, Vec3(0, 0, half.z)));
    outMin = -r;
    outMax =  r;
}

static bool overlaps(double minA, double maxA, double minB, double maxB) {
    return maxA >= minB && maxB >= minA;
}

static bool testAxis(const Vec3& axis, const Vec3& v0, const Vec3& v1, const Vec3& v2,
                     const Vec3& half) {
    if (dot(axis, axis) < 1e-10) return true; // degenerate axis, skip
    double tMin, tMax, bMin, bMax;
    projectTriangle(axis, v0, v1, v2, tMin, tMax);
    projectBox(axis, half, bMin, bMax);
    return overlaps(tMin, tMax, bMin, bMax);
}

static bool triBoxOverlap(const Vec3& boxMin, const Vec3& boxMax,
                           Vec3 v0, Vec3 v1, Vec3 v2) {
    // Center the box at origin
    Vec3 center((boxMin.x + boxMax.x) / 2,
                (boxMin.y + boxMax.y) / 2,
                (boxMin.z + boxMax.z) / 2);
    Vec3 half((boxMax.x - boxMin.x) / 2,
              (boxMax.y - boxMin.y) / 2,
              (boxMax.z - boxMin.z) / 2);

    v0 = sub(v0, center);
    v1 = sub(v1, center);
    v2 = sub(v2, center);

    Vec3 e0 = sub(v1, v0);
    Vec3 e1 = sub(v2, v1);
    Vec3 e2 = sub(v0, v2);

    // 9 cross-product axes (edge x box-axis)
    Vec3 boxAxes[3] = { Vec3(1,0,0), Vec3(0,1,0), Vec3(0,0,1) };
    Vec3 edges[3]   = { e0, e1, e2 };

    for (int ei = 0; ei < 3; ++ei) {
        for (int bi = 0; bi < 3; ++bi) {
            Vec3 axis = cross(edges[ei], boxAxes[bi]);
            if (!testAxis(axis, v0, v1, v2, half)) return false;
        }
    }

    // 3 face-normal axes of the AABB
    for (int bi = 0; bi < 3; ++bi) {
        if (!testAxis(boxAxes[bi], v0, v1, v2, half)) return false;
    }

    // Triangle normal axis
    Vec3 normal = cross(e0, e1);
    if (!testAxis(normal, v0, v1, v2, half)) return false;

    return true;
}

bool triBoxIntersect(const Vec3& boxMin, const Vec3& boxMax,
                     const std::vector<Vec3>& verts,
                     const std::vector<Face>& faces) {
    for (const Face& f : faces) {
        if (triBoxOverlap(boxMin, boxMax, verts[f.v1], verts[f.v2], verts[f.v3]))
            return true;
    }
    return false;
}
