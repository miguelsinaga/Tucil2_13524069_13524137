using Voxelizer.Models;

namespace Voxelizer.Octree;


/// Satu node dalam struktur Octree.
/// Setiap node merepresentasikan sebuah kubus (AABB) dalam ruang 3D.

public class OctreeNode
{
    /// Bounding box kubik yang diwakili node ini.
    public BoundingBox Bounds { get; }

    /// Kedalaman node di dalam pohon (root = 1).
    public int Depth { get; }

    
    /// Daftar segitiga yang berpotongan dengan bounds node ini.
    /// Null setelah node di-subdivide (segitiga sudah diteruskan ke anak).
     
    public List<Triangle>? Triangles { get; private set; }

    /// 8 anak node. Null selama node masih berupa leaf.
    public OctreeNode[]? Children { get; private set; }

    /// True jika node ini adalah leaf (tidak punya anak).
    public bool IsLeaf => Children == null;

    /// 
    /// True jika node ini adalah voxel aktif:
    /// leaf yang berpotongan dengan setidaknya satu segitiga permukaan.
    /// 
    public bool IsActiveVoxel => IsLeaf && (Triangles?.Count ?? 0) > 0;

    public OctreeNode(BoundingBox bounds, int depth, List<Triangle> triangles)
    {
        Bounds    = bounds;
        Depth     = depth;
        Triangles = triangles;
    }

    /// 
    /// Membagi node ini menjadi 8 anak (Divide step).
    /// Setiap anak mendapat subset segitiga yang berpotongan dengannya.
    /// 
    public void Subdivide()
    {
        if (!IsLeaf || Triangles == null || Triangles.Count == 0) return;

        var center = Bounds.Center;
        var min    = Bounds.Min;
        var max    = Bounds.Max;

        // 8 oktant berdasarkan pembagian di titik tengah
        BoundingBox[] childBoxes =
        [
            new(min,                                                         center),
            new(new(center.X, min.Y,    min.Z),    new(max.X, center.Y, center.Z)),
            new(new(min.X,    center.Y, min.Z),    new(center.X, max.Y, center.Z)),
            new(new(center.X, center.Y, min.Z),    new(max.X,    max.Y, center.Z)),
            new(new(min.X,    min.Y,    center.Z), new(center.X, center.Y, max.Z)),
            new(new(center.X, min.Y,    center.Z), new(max.X,    center.Y, max.Z)),
            new(new(min.X,    center.Y, center.Z), new(center.X, max.Y,   max.Z)),
            new(center,                                                      max),
        ];

        Children = new OctreeNode[8];

        for (int i = 0; i < 8; i++)
        {
            // Conquer: filter segitiga yang berpotongan dengan anak ini
            var childTriangles = Triangles
                .Where(t => childBoxes[i].IntersectsTriangle(t))
                .ToList();

            Children[i] = new OctreeNode(childBoxes[i], Depth + 1, childTriangles);
        }

        // Setelah subdivide, segitiga disimpan di anak, bukan di node ini
        Triangles = null;
    }
}
