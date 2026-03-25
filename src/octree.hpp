#pragma once
#include "parser.hpp"
#include <vector>
#include <map>
#include <memory>
#include <mutex>

struct Octree {
    Vec3 minBound, maxBound;
    std::unique_ptr<Octree> children[8];
    bool isLeaf = false;
};

// --- Divide & Conquer: bounding box ---
// Computes tight AABB of verts[left..right] using divide & conquer.
std::pair<Vec3, Vec3> boundingBox(const std::vector<Vec3>& verts);

// Midpoint of two Vec3
Vec3 midpoint(const Vec3& a, const Vec3& b);

// Create one of 8 child octants. i (0-7) encodes which octant:
//   bit0=1 → right X half, bit1=1 → upper Y half, bit2=1 → back Z half
std::unique_ptr<Octree> makeOctant(const Vec3& mn, const Vec3& mx,
                                   const Vec3& mid, int i);

// Collect all leaf nodes
void collectLeaves(const Octree* node, std::vector<const Octree*>& leaves);

// Build the octree recursively with concurrency (std::thread).
// prunedCounts[depth] accumulates count of pruned (empty) nodes at that depth.
void buildOctree(Octree* node,
                 const std::vector<Vec3>& verts,
                 const std::vector<Face>& faces,
                 int depth, int maxDepth,
                 std::map<int,int>& prunedCounts,
                 std::mutex& mu);

// Count total built nodes per depth
void countNodes(const Octree* node, int depth, std::map<int,int>& counts);
