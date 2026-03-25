namespace Voxelizer.Models;


/// Axis-Aligned Bounding Box (AABB).
/// Setiap node octree merepresentasikan satu AABB kubik.

public class BoundingBox
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }

    public BoundingBox(Vector3 min, Vector3 max)
    {
        Min = min;
        Max = max;
    }

    /// Titik tengah AABB
    public Vector3 Center => (Min + Max) / 2.0;

    /// Ukuran AABB per sumbu
    public Vector3 Size => Max - Min;

    
    /// Membuat AABB kubik dari model — sisi terpanjang dijadikan acuan
    /// agar semua voxel nanti berbentuk kubus sempurna.
    
    public static BoundingBox FromModel(ObjModel model)
    {
        var verts = model.Vertices;
        if (verts.Count == 0)
            return new BoundingBox(new Vector3(0, 0, 0), new Vector3(1, 1, 1));

        var minV = verts[0];
        var maxV = verts[0];
        foreach (var v in verts)
        {
            minV = Vector3.Min(minV, v);
            maxV = Vector3.Max(maxV, v);
        }

        // Buat kubus: ambil sisi terpanjang
        double dx = maxV.X - minV.X;
        double dy = maxV.Y - minV.Y;
        double dz = maxV.Z - minV.Z;
        double side = Math.Max(dx, Math.Max(dy, dz));

        // Tambah sedikit padding agar permukaan tidak tepat di tepi
        side *= 1.001;

        // Pusatkan bounding box terhadap model
        var center = (minV + maxV) / 2.0;
        var half = new Vector3(side / 2.0, side / 2.0, side / 2.0);

        return new BoundingBox(center - half, center + half);
    }

    
    /// Mengecek apakah titik berada di dalam AABB (inklusif).
    
    public bool Contains(Vector3 p) =>
        p.X >= Min.X && p.X <= Max.X &&
        p.Y >= Min.Y && p.Y <= Max.Y &&
        p.Z >= Min.Z && p.Z <= Max.Z;

    
    /// Mengecek apakah segitiga berpotongan dengan AABB.
    /// Menggunakan algoritma SAT (Separating Axis Theorem).
    
    public bool IntersectsTriangle(Triangle tri)
    {
        // Terjemahkan segitiga ke pusat AABB
        var c = Center;
        var e = Size / 2.0; // half-extents

        var v0 = tri.A - c;
        var v1 = tri.B - c;
        var v2 = tri.C - c;

        // Edge vectors segitiga
        var f0 = v1 - v0;
        var f1 = v2 - v1;
        var f2 = v0 - v2;

        // --- Uji 9 axis: cross(ei_aabb, fj_tri) ---
        Vector3[] axes =
        [
            new(0, -f0.Z,  f0.Y), new(0, -f1.Z,  f1.Y), new(0, -f2.Z,  f2.Y),
            new( f0.Z, 0, -f0.X), new( f1.Z, 0, -f1.X), new( f2.Z, 0, -f2.X),
            new(-f0.Y,  f0.X, 0), new(-f1.Y,  f1.X, 0), new(-f2.Y,  f2.X, 0),
        ];

        foreach (var axis in axes)
        {
            double p0 = Vector3.Dot(v0, axis);
            double p1 = Vector3.Dot(v1, axis);
            double p2 = Vector3.Dot(v2, axis);
            double r  = e.X * Math.Abs(axis.X)
                      + e.Y * Math.Abs(axis.Y)
                      + e.Z * Math.Abs(axis.Z);
            double mn = Math.Min(p0, Math.Min(p1, p2));
            double mx = Math.Max(p0, Math.Max(p1, p2));
            if (mn > r || mx < -r) return false;
        }

        // --- Uji 3 axis AABB (sumbu x, y, z) ---
        // X
        { double mn = Math.Min(v0.X, Math.Min(v1.X, v2.X));
          double mx = Math.Max(v0.X, Math.Max(v1.X, v2.X));
          if (mn > e.X || mx < -e.X) return false; }
        // Y
        { double mn = Math.Min(v0.Y, Math.Min(v1.Y, v2.Y));
          double mx = Math.Max(v0.Y, Math.Max(v1.Y, v2.Y));
          if (mn > e.Y || mx < -e.Y) return false; }
        // Z
        { double mn = Math.Min(v0.Z, Math.Min(v1.Z, v2.Z));
          double mx = Math.Max(v0.Z, Math.Max(v1.Z, v2.Z));
          if (mn > e.Z || mx < -e.Z) return false; }

        // --- Uji 1 axis: normal segitiga ---
        var normal = tri.Normal;
        double d  = Vector3.Dot(normal, v0);
        double re = e.X * Math.Abs(normal.X)
                  + e.Y * Math.Abs(normal.Y)
                  + e.Z * Math.Abs(normal.Z);
        if (d > re || d < -re) return false;

        return true;
    }
}
