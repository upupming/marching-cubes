#include "marching_cubes.h"

#include <cstring>
#include <ctime>
#include <iostream>

#include "LookUpTable.h"

template <typename T>
MarchingCubes<T>::MarchingCubes(const T* data, std::array<int, 3> dim, std::array<double, 3> spacing, bool reverseGradientDirection) {
    this->data = data;
    this->dim = dim;
    this->spacing = spacing;
    this->reverseGradientDirection = reverseGradientDirection;

    // 初始为 -1 表示没有插值
    xDirectionInterpolatedVertexIndex.resize(
        dim[0],
        std::vector<std::vector<int>>(
            dim[1],
            std::vector<int>(dim[2], -1)));
    yDirectionInterpolatedVertexIndex.resize(
        dim[0],
        std::vector<std::vector<int>>(
            dim[1],
            std::vector<int>(dim[2], -1)));
    zDirectionInterpolatedVertexIndex.resize(
        dim[0],
        std::vector<std::vector<int>>(
            dim[1],
            std::vector<int>(dim[2], -1)));
}

template <typename T>
void MarchingCubes<T>::runAlgorithm(double isoValue) {
    clock_t time = clock();

    this->isoValue = isoValue;
    // 计算所有插值顶点
    computeInterpolatedVertices();

    // 运行 marching cubes 算法，marching 并逐个处理 cube
    for (int i = 0; i + 1 < dim[0]; i++) {
        for (int j = 0; j + 1 < dim[1]; j++) {
            for (int k = 0; k + 1 < dim[2]; k++) {
                // 计算 configuration 编号
                int configurationIndex = 0;
                for (int l = 0; l < 8; l++) {
                    bool val =
                        getData(
                            // 编号 1, 2, 5, 6 的话 i 需要 + 1，这些数的后两位异或为 1
                            i + ((l ^ (l >> 1)) & 1),
                            // 编号 2, 3, 6, 7 的话 j 需要 + 1，这些数的倒数第 2 位为 1
                            j + ((l >> 1) & 1),
                            // 编号 4, 5, 6, 7 的话 k 需要 + 1，这些数的倒数第 3 为为 1
                            k + ((l >> 2) & 1)) > 0;
                    if (val > 0)
                        configurationIndex |= 1 << l;
                }
                processCube(i, j, k, configurationIndex);
            }
        }
    }

    printf("Marching Cubes ran in %lf secs.\n", (double)(clock() - time) / CLOCKS_PER_SEC);
}

template <typename T>
void MarchingCubes<T>::processCube(int i, int j, int k, int configurationIndex) {
    // 对于有一些为了解决内部歧义的情况（例如 6.1.2），需要在 cube 正中间插值算一个顶点，这个顶点的标号为 12
    int v12 = -1;
    // int caseIdx = cases[]
}

template class MarchingCubes<unsigned short>;
