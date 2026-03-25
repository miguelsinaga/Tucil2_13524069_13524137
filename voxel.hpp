#pragma once
#include "../parser/parser.hpp"
#include "../octree/octree.hpp"
#include <string>
#include <vector>

struct VoxelStats {
    int numVoxels;
    int numVerts;
    int numFaces;
};

// Generate voxel .obj from leaf octree nodes.
// Each leaf box is emitted as 8 verts + 12 triangles (6 quad faces split into 2).
VoxelStats writeVoxelOBJ(const std::vector<const Octree*>& leaves,
                          const std::string& outputPath);
