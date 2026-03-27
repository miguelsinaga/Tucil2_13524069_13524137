#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "parser.hpp"
#include "octree.hpp" 
#include "voxel.hpp"
#include "OBJviewer.hpp"
using namespace std;

int main() {
    string pathInput;
    int maxDepth;
    cout << "Masukkan path file .obj: ";
    cin >> pathInput;
    cout << "Masukkan kedalaman maksimum: ";
    cin >> maxDepth;

    try{
        auto startTime = chrono::high_resolution_clock::now();

        //parsing
        OBJModel ObjModel = parseOBJ(pathInput);

        //bounding box
        auto [minB, maxB] = boundingBox(ObjModel.verts);

        //build octree
        auto rootNode = make_unique<Octree>();
        rootNode->minBound = minB;
        rootNode->maxBound = maxB;
        map<int, int> ignoredNodes;
        mutex mtx;

        buildOctree(rootNode.get(), ObjModel.verts, ObjModel.faces, 0, maxDepth, ignoredNodes, mtx);
        
        //tulis output ke .obj
        vector<const Octree*> leafNodes;
        collectLeaves(rootNode.get(), leafNodes);
        string outputPath = "output_voxelized.obj";
        VoxelStats voxelResult = writeVoxelOBJ(leafNodes, outputPath);

        auto endTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = endTime - startTime;

        cout << "hasil konversi: "<<endl;
        cout << "Banyaknya voxel yang terbentuk: " << voxelResult.numVoxels << endl;
        cout << "Banyaknya vertex yang terbentuk: " << voxelResult.numVerts << endl;
        cout << "Banyaknya faces yang terbentuk: " << voxelResult.numFaces << endl;
        cout << "Statistik node octree: " << endl;
        map<int, int> nodeStats;
        countNodes(rootNode.get(), 0, nodeStats);
        for (int d = 1; d <= maxDepth; ++d) {
            cout << d << ": " << nodeStats[d] << endl;
        }

        cout <<"Statistik node yang tidak perlu ditelusuri: "<<endl;
        for (int d = 1; d <= maxDepth; ++d) {
            cout << d << ": " << ignoredNodes[d] << endl;
        }

        cout << "Kedalaman octree: " << maxDepth << endl;
        cout << "Waktu eksekusi: " << elapsed.count() << " detik" << endl;
        cout << "Output file disimpan di: " << outputPath << endl;

        std::cout << "\nMembuka 3D Viewer interaktif...\n";
        display3DModel("output_voxelized.obj");
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;


}