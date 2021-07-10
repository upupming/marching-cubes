
#include <cassert>
#include <iostream>

#include "marching_cubes.h"

void MarchingCubes::computeInterpolatedVertices() {
#pragma omp parallel for
    for (int i = 0; i < dim[0]; i++) {
        for (int j = 0; j < dim[1]; j++) {
            for (int k = 0; k < dim[2]; k++) {
                std::vector<float> cube(8);
                std::vector<std::array<float, 3>> normal(8);
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
                    float ratio = cube[0] / (cube[0] - cube[1]);
                    std::array<float, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[1][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        (i + ratio) * spacing[0], j * spacing[1], k * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);

                    xDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)] = addVertex(v);
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
                    float ratio = cube[0] / (cube[0] - cube[3]);
                    std::array<float, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[3][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        i * spacing[0], (j + ratio) * spacing[1], k * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);
                    yDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)] = addVertex(v);
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
                    float ratio = cube[0] / (cube[0] - cube[4]);
                    std::array<float, 3> normal_interpolated;
                    for (int idx = 0; idx < 3; idx++) {
                        normal_interpolated[idx] = normal[0][idx] + ratio * (normal[4][idx] - normal[0][idx]);
                    }
                    Vertex v(
                        i * spacing[0], j * spacing[1], (k + ratio) * spacing[2],
                        normal_interpolated[0],
                        normal_interpolated[1],
                        normal_interpolated[2]);

                    zDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)] = addVertex(v);
                }
            }
        }
    }
}

float MarchingCubes::getXGradient(int i, int j, int k) {
    if (i == 0) return (getData(i + 1, j, k) - getData(i, j, k)) / spacing[0];
    if (i == dim[0] - 1) return (getData(i, j, k) - getData(i - 1, j, k)) / spacing[0];
    return (getData(i + 1, j, k) - getData(i - 1, j, k)) / (2 * spacing[0]);
}

float MarchingCubes::getYGradient(int i, int j, int k) {
    if (j == 0) return (getData(i, j + 1, k) - getData(i, j, k)) / spacing[1];
    if (j == dim[1] - 1) return getData(i, j, k) - getData(i, j - 1, k) / spacing[1];
    return (getData(i, j + 1, k) - getData(i, j - 1, k)) / (2 * spacing[1]);
}

float MarchingCubes::getZGradient(int i, int j, int k) {
    if (k == 0) return (getData(i, j, k + 1) - getData(i, j, k)) / spacing[2];
    if (k == dim[2] - 1) return (getData(i, j, k) - getData(i, j, k - 1)) / spacing[2];
    return (getData(i, j, k + 1) - getData(i, j, k - 1)) / (2 * spacing[2]);
}

std::array<float, 3> MarchingCubes::getNormal(int i, int j, int k) {
    int d = reverseGradientDirection ? -1 : 1;
    return {getXGradient(i, j, k) * d, getYGradient(i, j, k) * d, getZGradient(i, j, k) * d};
}

int MarchingCubes::getCubeVertexIndex(int i, int j, int k, int edgeIdx) {
    switch (edgeIdx) {
        case 0:
            return xDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)];
        case 1:
            return yDirectionInterpolatedVertexIndex[glm::ivec3(i + 1, j, k)];
        case 2:
            return xDirectionInterpolatedVertexIndex[glm::ivec3(i, j + 1, k)];
        case 3:
            return yDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)];
        case 4:
            return xDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k + 1)];
        case 5:
            return yDirectionInterpolatedVertexIndex[glm::ivec3(i + 1, j, k + 1)];
        case 6:
            return xDirectionInterpolatedVertexIndex[glm::ivec3(i, j + 1, k + 1)];
        case 7:
            return yDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k + 1)];
        case 8:
            return zDirectionInterpolatedVertexIndex[glm::ivec3(i, j, k)];
        case 9:
            return zDirectionInterpolatedVertexIndex[glm::ivec3(i + 1, j, k)];
        case 10:
            return zDirectionInterpolatedVertexIndex[glm::ivec3(i + 1, j + 1, k)];
        case 11:
            return zDirectionInterpolatedVertexIndex[glm::ivec3(i, j + 1, k)];
        case 12:
            // 如果之前没有创建过，在这里创建这个 12 的点
            if (!centerInterpolatedVertexIndex.count(glm::ivec3(i, j, k))) {
                addCenterVertex(i, j, k);
            }
            return centerInterpolatedVertexIndex[glm::ivec3(i, j, k)];
    }
    std::cerr << "wrong edgeIdx: " << edgeIdx << std::endl;
    assert(false);
    return -1;
}

void MarchingCubes::addCenterVertex(int i, int j, int k) {
    Vertex center(0, 0, 0, 0, 0, 0);
    int cnt = 0;

    // 4 条 x 方向的边
    for (int s = 0; s < 2; s++) {
        for (int t = 0; t < 2; t++) {
            if (xDirectionInterpolatedVertexIndex.count(glm::ivec3(i, j + s, k + t))) {
                auto vid = xDirectionInterpolatedVertexIndex[glm::ivec3(i, j + s, k + t)];
                center += vertices[vid];
                cnt++;
            }
        }
    }
    // 4 条 y 方向的边
    for (int s = 0; s < 2; s++) {
        for (int t = 0; t < 2; t++) {
            if (yDirectionInterpolatedVertexIndex.count(glm::ivec3(i + s, j, k + t))) {
                auto vid = yDirectionInterpolatedVertexIndex[glm::ivec3(i + s, j, k + t)];
                center += vertices[vid];
                cnt++;
            }
        }
    }
    // 4 条 z 方向的边
    for (int s = 0; s < 2; s++) {
        for (int t = 0; t < 2; t++) {
            if (zDirectionInterpolatedVertexIndex.count(glm::ivec3(i + s, j + t, k))) {
                auto vid = zDirectionInterpolatedVertexIndex[glm::ivec3(i + s, j + t, k)];
                center += vertices[vid];
                cnt++;
            }
        }
    }

    // 既然要插值中间 vertex，肯定不可能边上没有 vertex 的
    if (cnt == 0) {
        std::cout << "No neighboor to compute center vertex inside cube" << std::endl;
        assert(false);
    }
    center /= cnt;
    center.normalizeNormal();
    centerInterpolatedVertexIndex[glm::ivec3(i, j, k)] = addVertex(center);
}
