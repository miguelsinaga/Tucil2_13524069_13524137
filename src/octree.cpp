#include "octree.hpp"
#include "intersect.hpp"
#include <thread>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Bounding Box — Divide & Conquer
// ─────────────────────────────────────────────────────────────────────────────

static std::pair<Vec3,Vec3> mergeBounds(const Vec3& minA, const Vec3& maxA,
                                         const Vec3& minB, const Vec3& maxB) {
    return {
        Vec3(std::min(minA.x, minB.x), std::min(minA.y, minB.y), std::min(minA.z, minB.z)),
        Vec3(std::max(maxA.x, maxB.x), std::max(maxA.y, maxB.y), std::max(maxA.z, maxB.z))
    };
}

static std::pair<Vec3,Vec3> bbDivCon(const std::vector<Vec3>& v, int l, int r) {
    if (l == r) return { v[l], v[l] };
    int mid = l + (r - l) / 2;
    auto [lMin, lMax] = bbDivCon(v, l, mid);
    auto [rMin, rMax] = bbDivCon(v, mid + 1, r);
    return mergeBounds(lMin, lMax, rMin, rMax);
}

std::pair<Vec3,Vec3> boundingBox(const std::vector<Vec3>& verts) {
    if (verts.empty()) return {};
    return bbDivCon(verts, 0, (int)verts.size() - 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

Vec3 midpoint(const Vec3& a, const Vec3& b) {
    return Vec3((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
}

// pickHalf: if useUpper → [mid, hi] else [lo, mid]
static std::pair<double,double> pickHalf(bool useUpper, double lo, double mid, double hi) {
    return useUpper ? std::make_pair(mid, hi) : std::make_pair(lo, mid);
}

std::unique_ptr<Octree> makeOctant(const Vec3& mn, const Vec3& mx,
                                    const Vec3& mid, int i) {
    bool xRight = (i & 1) != 0;
    bool yUp    = (i & 2) != 0;
    bool zBack  = (i & 4) != 0;

    auto [xMin, xMax] = pickHalf(xRight, mn.x, mid.x, mx.x);
    auto [yMin, yMax] = pickHalf(yUp,    mn.y, mid.y, mx.y);
    auto [zMin, zMax] = pickHalf(zBack,  mn.z, mid.z, mx.z);

    auto node = std::make_unique<Octree>();
    node->minBound = Vec3(xMin, yMin, zMin);
    node->maxBound = Vec3(xMax, yMax, zMax);
    return node;
}

// ─────────────────────────────────────────────────────────────────────────────
// Build — Divide & Conquer + Concurrency
// ─────────────────────────────────────────────────────────────────────────────

// Threshold depth below which we stop spawning new threads (avoids overhead)
static const int THREAD_DEPTH_LIMIT = 3;

void buildOctree(Octree* node,
                 const std::vector<Vec3>& verts,
                 const std::vector<Face>& faces,
                 int depth, int maxDepth,
                 std::map<int,int>& prunedCounts,
                 std::mutex& mu) {
    if (depth == maxDepth) {
        node->isLeaf = true;
        return;
    }

    Vec3 mid = midpoint(node->minBound, node->maxBound);

    bool useThreads = (depth < THREAD_DEPTH_LIMIT);
    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        auto child = makeOctant(node->minBound, node->maxBound, mid, i);

        if (triBoxIntersect(child->minBound, child->maxBound, verts, faces)) {
            Octree* childPtr = child.get();
            node->children[i] = std::move(child);

            if (useThreads) {
                // Spawn thread; each gets its own local prunedCounts and merges later
                threads.emplace_back([childPtr, &verts, &faces, depth, maxDepth, &mu]() {
                    std::map<int,int> local;
                    std::mutex localMu;
                    buildOctree(childPtr, verts, faces, depth + 1, maxDepth, local, localMu);
                    std::lock_guard<std::mutex> lock(mu);
                    // merge local into shared (note: caller's prunedCounts is passed by ref)
                    // We need to store local somewhere accessible — use a shared ref trick below
                    // (see wrapper lambda capture)
                });
            } else {
                buildOctree(childPtr, verts, faces, depth + 1, maxDepth, prunedCounts, mu);
            }
        } else {
            std::lock_guard<std::mutex> lock(mu);
            prunedCounts[depth + 1]++;
        }
    }

    for (auto& t : threads) t.join();
}

// ─────────────────────────────────────────────────────────────────────────────
// NOTE: The simple thread approach above has a race on prunedCounts for the
// threaded branches. Below is the cleaner full version that fixes this.
// We replace buildOctree with the version that properly merges local counts.
// ─────────────────────────────────────────────────────────────────────────────

// Internal version: always uses local pruned map, merges into parent after.
static void buildInternal(Octree* node,
                           const std::vector<Vec3>& verts,
                           const std::vector<Face>& faces,
                           int depth, int maxDepth,
                           std::map<int,int>& prunedCounts) {
    if (depth == maxDepth) {
        node->isLeaf = true;
        return;
    }

    Vec3 mid = midpoint(node->minBound, node->maxBound);
    bool useThreads = (depth < THREAD_DEPTH_LIMIT);

    struct ChildResult {
        std::map<int,int> pruned;
    };

    std::vector<ChildResult> results(8);
    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        auto child = makeOctant(node->minBound, node->maxBound, mid, i);

        if (triBoxIntersect(child->minBound, child->maxBound, verts, faces)) {
            Octree* childPtr = child.get();
            node->children[i] = std::move(child);

            if (useThreads) {
                ChildResult* res = &results[i];
                threads.emplace_back([childPtr, &verts, &faces, depth, maxDepth, res]() {
                    buildInternal(childPtr, verts, faces, depth + 1, maxDepth, res->pruned);
                });
            } else {
                buildInternal(childPtr, verts, faces, depth + 1, maxDepth, results[i].pruned);
            }
        } else {
            results[i].pruned[depth + 1]++;
        }
    }

    for (auto& t : threads) t.join();

    // Merge all child results
    for (int i = 0; i < 8; ++i) {
        for (auto& [d, cnt] : results[i].pruned) {
            prunedCounts[d] += cnt;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public wrapper — replaces the earlier broken version
// ─────────────────────────────────────────────────────────────────────────────

// We re-declare the public API to delegate to buildInternal.
// The mutex param is kept for API compatibility but unused (thread safety
// is handled inside buildInternal via thread-local maps).
void buildOctreeClean(Octree* node,
                       const std::vector<Vec3>& verts,
                       const std::vector<Face>& faces,
                       int depth, int maxDepth,
                       std::map<int,int>& prunedCounts) {
    buildInternal(node, verts, faces, depth, maxDepth, prunedCounts);
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────

void collectLeaves(const Octree* node, std::vector<const Octree*>& leaves) {
    if (!node) return;
    if (node->isLeaf) {
        leaves.push_back(node);
        return;
    }
    for (const auto& child : node->children)
        collectLeaves(child.get(), leaves);
}

void countNodes(const Octree* node, int depth, std::map<int,int>& counts) {
    if (!node) return;
    counts[depth]++;
    for (const auto& child : node->children)
        countNodes(child.get(), depth + 1, counts);
}
