using Voxelizer.Models;

namespace Voxelizer.Octree;


/// Struktur Octree untuk voxelization 3D.
/// Mengimplementasikan algoritma Divide and Conquer:
///   - Divide  : membagi ruang menjadi 8 oktant
///   - Conquer : setiap oktant menentukan apakah berpotongan dengan permukaan model

public class Octree
{
    public OctreeNode Root { get; }
    public int MaxDepth { get; }

    // Statistik
    public int[] NodeCountPerDepth    { get; }  // jumlah node terbentuk tiap depth
    public int[] SkippedCountPerDepth { get; }  // jumlah node tidak perlu ditelusuri tiap depth

    public Octree(ObjModel model, int maxDepth)
    {
        MaxDepth = maxDepth;
        NodeCountPerDepth    = new int[maxDepth + 1];
        SkippedCountPerDepth = new int[maxDepth + 1];

        var rootBounds = BoundingBox.FromModel(model);
        Root = new OctreeNode(rootBounds, 1, new List<Triangle>(model.Triangles));

        // Bangun tree secara rekursif (divide & conquer)
        Build(Root);
    }

    
    /// Rekursi utama: divide node hingga kedalaman maksimum.
    
    private void Build(OctreeNode node)
    {
        NodeCountPerDepth[node.Depth]++;

        // Base case: sudah di kedalaman maksimum → jadikan leaf (voxel)
        if (node.Depth >= MaxDepth)
            return;

        // Pruning: jika tidak ada segitiga, tidak perlu ditelusuri lebih lanjut
        if (node.Triangles == null || node.Triangles.Count == 0)
        {
            SkippedCountPerDepth[node.Depth]++;
            return;
        }

        // Divide: bagi node menjadi 8 anak
        node.Subdivide();

        // Conquer: rekursi ke setiap anak
        if (node.Children != null)
            foreach (var child in node.Children)
                Build(child);
    }

    
    /// Mengambil semua leaf node yang aktif (berpotongan dengan permukaan).
    /// Leaf aktif inilah yang akan menjadi voxel output.
    
    public List<OctreeNode> GetActiveLeaves()
    {
        var result = new List<OctreeNode>();
        CollectActiveLeaves(Root, result);
        return result;
    }

    private static void CollectActiveLeaves(OctreeNode node, List<OctreeNode> result)
    {
        if (node.IsLeaf)
        {
            if (node.IsActiveVoxel)
                result.Add(node);
            return;
        }

        if (node.Children != null)
            foreach (var child in node.Children)
                CollectActiveLeaves(child, result);
    }
}
