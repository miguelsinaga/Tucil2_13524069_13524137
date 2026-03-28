#include "OBJviewer.hpp"
using namespace std;

struct RenderedTriangle {
    sf::ConvexShape polygon;
    float zDepth;
};

void display3DModel(const string& objFilePath){
    OBJModel OBJModel;
    try{
        OBJModel = parseOBJ(objFilePath);
    } catch (const exception& e) {
        cerr << "Error parsing OBJ file: " << e.what() << endl;
        return;
    }


    // Hitung bounding box dan centroid dari model
    double xMin = OBJModel.verts[0].x, xMax = xMin;
    double yMin = OBJModel.verts[0].y, yMax = yMin;
    double zMin = OBJModel.verts[0].z, zMax = zMin;
    for (const auto& v : OBJModel.verts) {
        xMin = min(xMin, v.x); xMax = max(xMax, v.x);
        yMin = min(yMin, v.y); yMax = max(yMax, v.y);
        zMin = min(zMin, v.z); zMax = max(zMax, v.z);
    }
    Vec3 center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);

    // Geser semua vertex agar pusat objek = (0,0,0)
    for (auto& v : OBJModel.verts) {
        v.x -= center.x;
        v.y -= center.y;
        v.z -= center.z;
    }

    
    // Hitung diagonal bounding box (setelah centering) untuk menentukan skala
    double rangeX = xMax - xMin;
    double rangeY = yMax - yMin;
    double rangeZ = zMax - zMin;
    float modelSize = (float)max({rangeX, rangeY, rangeZ});

    // distance = 2.5x ukuran terbesar objek (agar objek masuk layar penuh)
    float distance = modelSize * 2.5f;
    // projectionScale disesuaikan agar objek mengisi ~60% layar
    float projectionScale = (300.0f / modelSize) * distance;

    // Batas zoom: antara 0.5x hingga 10x ukuran model
    float distMin = modelSize * 0.5f;
    float distMax = modelSize * 10.0f;
    // Sensitivitas scroll proporsional terhadap ukuran model
    float scrollSpeed = modelSize * 0.1f;

    sf::RenderWindow window(sf::VideoMode(800, 600), "3D Model Viewer (SFML)");
    window.setFramerateLimit(60);

    float pitch = 0.0f; // rotasi atas-bawah
    float yaw   = 0.0f; // rotasi kiri-kanan

    bool isDragging = false;
    sf::Vector2i lastMousePos;

    while(window.isOpen()){
        sf::Event userInput;
        while(window.pollEvent(userInput)){
            if(userInput.type == sf::Event::Closed)
                window.close();

            // logika rotasi mouse
            if (userInput.type == sf::Event::MouseButtonPressed && userInput.mouseButton.button == sf::Mouse::Left) {
                isDragging = true;
                lastMousePos = sf::Mouse::getPosition(window);
            }
            if (userInput.type == sf::Event::MouseButtonReleased && userInput.mouseButton.button == sf::Mouse::Left) {
                isDragging = false;
            }
            if (userInput.type == sf::Event::MouseMoved && isDragging) {
                sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                float deltaX = (float)(currentMousePos.x - lastMousePos.x);
                float deltaY = (float)(currentMousePos.y - lastMousePos.y);
                yaw   += deltaX * 0.01f;
                pitch += deltaY * 0.01f;
                lastMousePos = currentMousePos;
            }

            // logika zoom in / zoom out (scroll mouse)
            if (userInput.type == sf::Event::MouseWheelScrolled) {
                if (userInput.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    distance -= userInput.mouseWheelScroll.delta * scrollSpeed;
                    if (distance < distMin) distance = distMin;
                    if (distance > distMax) distance = distMax;
                }
            }
        }

        window.clear(sf::Color(30, 30, 30)); // latar gelap lebih baik dari Magenta

        vector<RenderedTriangle> renderedTriangle;

        float cosPitch = cos(pitch);
        float sinPitch = sin(pitch);
        float cosYaw   = cos(yaw);
        float sinYaw   = sin(yaw);

        for(const auto& face : OBJModel.faces){
            Vec3 v[3] = {OBJModel.verts[face.v1], OBJModel.verts[face.v2], OBJModel.verts[face.v3]};
            sf::Vector2f pixelCoords[3];
            float faceDepth = 0.0f;
            bool valid = true;

            for(int i = 0; i < 3; i++){
                // rotasi kiri kanan (yaw)
                float x1 = (float)(v[i].x * cosYaw - v[i].z * sinYaw);
                float z1 = (float)(v[i].x * sinYaw + v[i].z * cosYaw);
                float y1 = (float)v[i].y;

                // rotasi atas bawah (pitch)
                float y2 = y1 * cosPitch - z1 * sinPitch;
                float z2 = y1 * sinPitch + z1 * cosPitch;
                float x2 = x1;

                // dorong objek ke depan kamera
                z2 += distance;
                faceDepth += z2;

                // ─── FIX 3: Skip face jika z2 <= 0 (di belakang/di dalam kamera) ──
                if (z2 <= 0.001f) { valid = false; break; }

                pixelCoords[i].x = (x2 / z2) * projectionScale + 400.0f;
                pixelCoords[i].y = (-y2 / z2) * projectionScale + 300.0f;
            }

            if (!valid) continue;

            faceDepth /= 3.0f;

            sf::ConvexShape renderedFace(3);
            renderedFace.setPoint(0, pixelCoords[0]);
            renderedFace.setPoint(1, pixelCoords[1]);
            renderedFace.setPoint(2, pixelCoords[2]);
            renderedFace.setFillColor(sf::Color(180, 180, 180));
            renderedFace.setOutlineColor(sf::Color(50, 50, 50));
            renderedFace.setOutlineThickness(1.0f);

            renderedTriangle.push_back({renderedFace, faceDepth});
        }

        sort(renderedTriangle.begin(), renderedTriangle.end(), [](const RenderedTriangle& a, const RenderedTriangle& b){
            return a.zDepth > b.zDepth; // gambar yang lebih jauh dulu
        });

        for(const auto& tri : renderedTriangle){
            window.draw(tri.polygon);
        }

        window.display();
    }
}