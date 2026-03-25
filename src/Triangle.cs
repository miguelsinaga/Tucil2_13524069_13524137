namespace Voxelizer.Models;

/// <summary>
/// Segitiga yang terbentuk dari 3 vertex (face pada .obj).
/// </summary>
public class Triangle
{
    public Vector3 A, B, C;

    public Triangle(Vector3 a, Vector3 b, Vector3 c)
    {
        A = a; B = b; C = c;
    }

    /// <summary>
    /// Normal segitiga (tidak dinormalisasi).
    /// </summary>
    public Vector3 Normal => Vector3.Cross(B - A, C - A);

    /// <summary>
    /// Titik tengah (centroid) segitiga.
    /// </summary>
    public Vector3 Centroid => (A + B + C) / 3.0;
}
