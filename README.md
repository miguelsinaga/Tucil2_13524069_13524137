# Voxelizer 3000: Octree-Based 3D Voxelization

Program ini adalah aplikasi berbasis C++ yang mengonversi model 3D (format `.obj`) menjadi bentuk Voxel menggunakan algoritma Divide and Conquer dengan struktur data Octree. Program ini juga dilengkapi dengan *Custom 3D Software Renderer* untuk memvisualisasikan hasil konversi secara interaktif.

---

## Deskripsi Program
Aplikasi ini melakukan *voxelization* dengan membagi ruang 3D secara rekursif menjadi 8 bagian (*octants*). Setiap bagian diperiksa apakah berpotongan dengan poligon model aslinya. Jika mencapai kedalaman maksimum, bagian tersebut akan disimpan sebagai satu kubus (voxel).

### Fitur Utama & Bonus
* **Voxelization Engine**: Konversi akurat menggunakan pengecekan *AABB-Triangle Intersection*.
* **Concurrency (Bonus - 5 Poin)**: Mempercepat pembangunan Octree menggunakan `std::thread` dan pengamanan data statistik dengan `std::mutex`.
* **Interactive 3D Viewer (Bonus - 10 Poin)**: *Software renderer* murni menggunakan library **SFML** yang mencakup:
    * **Representasi Kamera**: Transformasi pandangan (*View Transform*).
    * **Proyeksi Perspektif**: Pemetaan 3D ke 2D (*Perspective Divide*).
    * **Viewport Mapping**: Pemetaan koordinat kamera ke piksel layar.
    * **Painter's Algorithm**: Pengurutan segitiga berdasarkan jarak (Z-depth) untuk rendering yang benar tanpa bantuan GPU.

---

## Requirement & Instalasi

### 1. Keselarasan Compiler (ABI Compatibility)
Agar library SFML dapat terbaca dan tidak terjadi error `Entry Point Not Found`, gunakan compiler yang sama dengan saat library dibuat:
* **Compiler**: MinGW-w64 (GCC) versi **13.1.0**.
* Pastikan folder `bin` dari compiler tersebut sudah masuk ke dalam *System Environment Variables (PATH)*.

### 2. Library SFML 2.6.1
* Pastikan folder library berada di path yang benar (misal: `C:\SFML\SFML-2.6.1`).
* Jika diletakkan di tempat lain, ubah flag `-I` (Include) dan `-L` (Library) pada perintah kompilasi di bawah.

---

## Cara Kompilasi

Buka terminal (PowerShell/CMD) di dalam folder `src`, lalu jalankan perintah berikut:

```powershell
g++ -O3 -std=c++17 -pthread main.cpp parser.cpp octree.cpp intersect.cpp voxel.cpp OBJviewer.cpp -o ../bin/voxelizer -I"C:\SFML\SFML-2.6.1\include" -L"C:\SFML\SFML-2.6.1\lib" -lsfml-graphics -lsfml-window -lsfml-system -static-libgcc -static-libstdc++ -static -lpthread
```

### Cara Menjalankan & Menggunakan
* eksekusi:
```powershell
..\bin\voxelizer.exe
```
* input user:
* masukkan nama file obj (misal: `cow.obj`)
* masukkan kedalaman maks

* interaksi viewer
  * rotasi model (ptch & yaw)
  * zoom in/out
  * exit (X)

### Struktur Direktori
```Plaintext
├── bin/       # File eksekusi (.exe) dan Library (.dll)
├── doc/       # Laporan Tugas (PDF)
├── src/       # Source code (.cpp dan .hpp)
├── test/      # Contoh model .obj untuk pengujian
└── README.md 
```
### Identitas Pembuat
* 13524069 - Miguel Rangga Deardo Sinaga
* 13524137 - Reysha Syafitri MR
