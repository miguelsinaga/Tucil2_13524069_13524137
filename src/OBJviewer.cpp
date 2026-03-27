#include "OBJviewer.hpp"
using namespace std;

struct RenderedTriangle {
    sf::ConvexShape polygon;
    float zDepth;
};

void display3DModel(const string& objFilePath){
    OBJModel OBJModel;
    try{
        OBJModel = parseOBJ (objFilePath);
    } catch (const exception& e) {
        cerr << "Error parsing OBJ file: " << e.what() << endl;
        return;
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "3D Model Viewer (SFML)");
    window.setFramerateLimit(60);

    float pitch = 0.0f; //rotasi atas-bawah
    float yaw = 0.0f;   //rotasi kiri-kanan
    float distance = 15.0f; //jarak kamera ke objek
    float projectionScale = 400.0f; //faktor perbesaran proyeksi perspektif
    
    bool isDragging = false; //untuk interaksi mouse
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
                float deltaX = currentMousePos.x - lastMousePos.x;
                float deltaY = currentMousePos.y - lastMousePos.y;
                
                yaw += deltaX * 0.01f; //putar sumbu Y berdasarkan geseran X
                pitch += deltaY * 0.01f; //putar sumbu X berdasarkan geseran Y
                
                lastMousePos = currentMousePos;
            }

            // logika zoom in / zoom out (scroll mouse)
            if (userInput.type == sf::Event::MouseWheelScrolled) {
                if (userInput.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    distance -= userInput.mouseWheelScroll.delta * 1.5f; //scroll atas positif, scroll bawah negatif
                    if (distance < 2.0f) distance = 2.0f; //batasan agar kamera tidak menembus objek
                    if (distance > 50.0f) distance = 50.0f; //batasan agar kamera tidak terlalu jauh
                }
            }
        }
        window.clear(sf::Color::Magenta);

        vector<RenderedTriangle> renderedTriangle;

        float cosPitch = cos(pitch);
        float sinPitch = sin(pitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);

        for(const auto& face : OBJModel.faces){
            Vec3 v[3] = {OBJModel.verts[face.v1], OBJModel.verts[face.v2], OBJModel.verts[face.v3]};
            sf::Vector2f pixelCoords[3];
            float faceDepth = 0.0f;

            for(int i=0; i<3; i++){
                // rotasi kiri kanan (yaw)
                float x1 = v[i].x * cosYaw - v[i].z * sinYaw;
                float z1 = v[i].x * sinYaw + v[i].z * cosYaw;
                float y1 = v[i].y;

                // rotasi atas bawah (pitch)
                float y2 = y1 * cosPitch - z1 * sinPitch;
                float z2 = y1 * sinPitch + z1 * cosPitch;
                float x2 = x1;

                // mendorong objek menjauh dari bidang proyeksi
                z2 += distance;
                faceDepth += z2;

                // mengubah 3d jadi 2d
                if(z2 != 0){
                    pixelCoords[i].x = (x2 / z2) * projectionScale + 400.0f;
                    pixelCoords[i].y = (-y2 / z2) * projectionScale + 300.0f; 
                }
            }
            faceDepth /= 3.0f; //rata-rata kedalaman segitiga

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
            return a.zDepth > b.zDepth; //gambar yang lebih jauh dulu
        });

        for(const auto& tri : renderedTriangle){
            window.draw(tri.polygon);
        }

        window.display();
    }
    
}