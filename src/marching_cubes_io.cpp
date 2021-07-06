#include <fstream>

#include "marching_cubes.h"

void MarchingCubes::saveObj(std::string filename) {
    clock_t time = clock();

    std::ofstream objFile(filename);

    std::string s;

    if (!objFile.is_open()) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }

    for (int i = 0; i < vertices.size(); i++) {
        auto &v = vertices[i];

        std::string tmp = "v " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + "\n";
        tmp += "vn " + std::to_string(v.nx) + " " + std::to_string(v.ny) + " " + std::to_string(v.nz) + "\n";

        s += tmp;
    }
    for (int i = 0; i < triangles.size(); i++) {
        auto &t = triangles[i];

        std::string tmp = "f " +
                          std::to_string(t[0] + 1) + "//" + std::to_string(t[0] + 1) + " " +
                          std::to_string(t[1] + 1) + "//" + std::to_string(t[1] + 1) + " " +
                          std::to_string(t[2] + 1) + "//" + std::to_string(t[2] + 1) + " " + "\n";

        s += tmp;
    }
    objFile << s;
    objFile.close();

    printf("OBJ file saved in %lf secs.\n", (double)(clock() - time) / CLOCKS_PER_SEC);
}
