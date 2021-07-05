
#include <iostream>

#include "marching_cubes.h"

template <typename T>
void MarchingCubes<T>::computeInterpolatedVertices() {
    for (int i = 0; i < dim[0]; i++) {
        for (int j = 0; j < dim[1]; j++) {
            for (int k = 0; k < dim[2]; k++) {
                std::vector<double> cube(8);
                std::vector<std::array<double, 3>> normal(8);
                if (i % 100 == 0 && j == 0 && k == 0)
                    std::cout << "computeInterpolatedVertices: " << i << " " << j << " " << k << std::endl;
                cube[0] = getData(i, j, k);
                normal[0] = getNormal(i, j, k);

                // x 方向
                if (i + 1 < dim[0]) {
                    cube[1] = getData(i + 1, j, k);
                    normal[1] = getNormal(i + 1, j, k);
                } else {
                    cube[1] = cube[0];
                    normal[1] = normal[0];
                }
                if (cube[0] * cube[1] < 0) {
                    double ratio = cube[0] / (cube[0] - cube[1]);
                    std::array<double, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[1][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        (i + ratio) * spacing[0], j * spacing[1], k * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);
                    vertices.push_back(v);
                    xDirectionInterpolatedVertexIndex[i][j][k] = vertices.size() - 1;
                }

                // y 方向
                if (j + 1 < dim[1]) {
                    cube[3] = getData(i, j + 1, k);
                    normal[3] = getNormal(i, j + 1, k);
                } else {
                    cube[3] = cube[0];
                    normal[3] = normal[0];
                }
                if (cube[0] * cube[3] < 0) {
                    double ratio = cube[0] / (cube[0] - cube[3]);
                    std::array<double, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[3][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        i * spacing[0], (j + ratio) * spacing[1], k * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);
                    vertices.push_back(v);
                    yDirectionInterpolatedVertexIndex[i][j][k] = vertices.size() - 1;
                }

                // z 方向
                if (k + 1 < dim[2]) {
                    cube[4] = getData(i, j, k + 1);
                    normal[4] = getNormal(i, j, k + 1);
                } else {
                    cube[4] = cube[0];
                    normal[4] = normal[0];
                }
                if (cube[0] * cube[4] < 0) {
                    double ratio = cube[0] / (cube[0] - cube[4]);
                    std::array<double, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[4][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        i * spacing[0], j * spacing[1], (k + ratio) * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);
                    vertices.push_back(v);
                    zDirectionInterpolatedVertexIndex[i][j][k] = vertices.size() - 1;
                }
            }
        }
    }
}

template <typename T>
double MarchingCubes<T>::getXGradient(int i, int j, int k) {
    if (i == 0) return (getData(i + 1, j, k) - getData(i, j, k)) / spacing[0];
    if (i == dim[0] - 1) return (getData(i, j, k) - getData(i - 1, j, k)) / spacing[0];
    return (getData(i + 1, j, k) - getData(i - 1, j, k)) / (2 * spacing[0]);
}

template <typename T>
double MarchingCubes<T>::getYGradient(int i, int j, int k) {
    if (j == 0) return (getData(i, j + 1, k) - getData(i, j, k)) / spacing[1];
    if (j == dim[1] - 1) return getData(i, j, k) - getData(i, j - 1, k) / spacing[1];
    return (getData(i, j + 1, k) - getData(i, j - 1, k)) / (2 * spacing[1]);
}

template <typename T>
double MarchingCubes<T>::getZGradient(int i, int j, int k) {
    if (k == 0) return (getData(i, j, k + 1) - getData(i, j, k)) / spacing[2];
    if (k == dim[2] - 1) return (getData(i, j, k) - getData(i, j, k - 1)) / spacing[2];
    return (getData(i, j, k + 1) - getData(i, j, k - 1)) / (2 * spacing[2]);
}

template <typename T>
std::array<double, 3> MarchingCubes<T>::getNormal(int i, int j, int k) {
    return {getXGradient(i, j, k), getYGradient(i, j, k), getZGradient(i, j, k)};
}

template class MarchingCubes<unsigned short>;
