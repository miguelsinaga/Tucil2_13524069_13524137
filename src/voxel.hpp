#pragma once
#include "parser.hpp"
#include "octree.hpp"
#include <string>
#include <vector>

struct VoxelStats {
    int numVoxels;
    int numVerts;
    int numFaces;
};

// Generate voxel .obj from leaf octree nodes.
VoxelStats writeVoxelOBJ(const std::vector<const Octree*>& leaves,
                          const std::string& outputPath);
