#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

OBJModel parseOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    OBJModel model;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;

        // trim leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            double x, y, z;
            if (!(ss >> x >> y >> z))
                throw std::runtime_error("Line " + std::to_string(lineNum) + ": 'v' needs 3 coords");
            model.verts.push_back(Vec3(x, y, z));

        } else if (token == "f") {
            // Support both "f i j k" and "f i/t/n j/t/n k/t/n" formats
            auto parseIndex = [&](const std::string& s) -> int {
                // take only the part before '/'
                size_t slash = s.find('/');
                std::string idx = (slash == std::string::npos) ? s : s.substr(0, slash);
                int i = std::stoi(idx);
                if (i < 1)
                    throw std::runtime_error("Line " + std::to_string(lineNum) + ": face index must be >= 1");
                return i - 1; // convert to 0-indexed
            };

            std::string s1, s2, s3;
            if (!(ss >> s1 >> s2 >> s3))
                throw std::runtime_error("Line " + std::to_string(lineNum) + ": 'f' needs 3 indices");

            Face f;
            f.v1 = parseIndex(s1);
            f.v2 = parseIndex(s2);
            f.v3 = parseIndex(s3);
            model.faces.push_back(f);
        }
        // ignore other tokens (vn, vt, etc.)
    }

    // Validate face indices
    for (int i = 0; i < (int)model.faces.size(); ++i) {
        const Face& f = model.faces[i];
        if (f.v1 >= (int)model.verts.size() || f.v2 >= (int)model.verts.size() || f.v3 >= (int)model.verts.size())
            throw std::runtime_error("Face " + std::to_string(i + 1) + " references vertex out of range");
    }

    if (model.verts.empty())
        throw std::runtime_error("No vertices found in file");
    if (model.faces.empty())
        throw std::runtime_error("No faces found in file");

    return model;
}
