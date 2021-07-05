#include <fstream>
#include <sstream>

#include "marching_cubes.h"

void MarchingCubes::saveObj(std::string filename) {
    clock_t time = clock();

    std::ofstream objFile(filename);
    std::stringstream ss;

    if (!objFile.is_open()) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }
    for (auto &v : vertices) {
        ss << "v " << std::to_string(v.x) << " " << std::to_string(v.y) << " " << std::to_string(v.z) << std::endl;
        ss << "vn " << std::to_string(v.nx) << " " << std::to_string(v.ny) << " " << std::to_string(v.nz) << std::endl;
    }
    for (auto &t : triangles) {
        ss << "f "
           << std::to_string(t[0] + 1) << "//" << std::to_string(t[0] + 1) << " "
           << std::to_string(t[1] + 1) << "//" << std::to_string(t[1] + 1) << " "
           << std::to_string(t[2] + 1) << "//" << std::to_string(t[2] + 1) << " "
           << std::endl;
    }
    objFile << ss.str();
    objFile.close();

    printf("OBJ file saved in %lf secs.\n", (double)(clock() - time) / CLOCKS_PER_SEC);
}
