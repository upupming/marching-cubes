﻿/*
Marching Cubes 算法实现
参考论文: efficient implementation of Marching Cubes’ cases with topological guarantees
*/
#pragma once
#include <array>
#include <vector>
struct Vertex {
    // 顶点坐标
    double x, y, z;
    // 法向量
    double nx, ny, nz;
    Vertex(double x, double y, double z, double nx, double ny, int nz) : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz) {}
};

template <typename T = unsigned short>
class MarchingCubes {
   public:
    MarchingCubes(const T* data, std::array<int, 3> dim, std::array<double, 3> spacing, bool reverseGradientDirection = false);
    /**
    * 运行算法，生成顶点（带法线）、三角形
    * \param isoValue 等值面大小
    **/
    void runAlgorithm(double isoValue);
    inline std::vector<Vertex> getVertices() { return vertices; }
    inline std::vector<std::array<int, 3>> getTriangles() { return triangles; }

   private:
    const T* data;
    std::array<int, 3> dim;
    std::array<double, 3> spacing{1, 1, 1};
    bool reverseGradientDirection = false;
    std::vector<Vertex> vertices;
    // 所有的三角形，其中每个三角形是 3 个 Vertex 在 vertices 中的索引下标
    std::vector<std::array<int, 3>> triangles;
    double isoValue;
    inline double getData(int i, int j, int k) {
        return data[i * dim[1] * dim[2] + j * dim[2] + k] - isoValue;
    }

    // xDirectionInterpolatedVertexIndex[i][j][k]
    // 表示以 (i, j, k) 点向 x 方向的边上的插值顶点
    // 注意对于两个点的正负性相同的边，中间是不需要插值顶点的
    // x 方向又叫 horizontal 方向
    // y 方向又叫 longitudinal 方向
    // z 方向又叫 vertical 方向
    std::vector<std::vector<std::vector<int>>> xDirectionInterpolatedVertexIndex, yDirectionInterpolatedVertexIndex, zDirectionInterpolatedVertexIndex;
    void computeInterpolatedVertices();

    // 梯度方向就是法向量方向
    inline double getXGradient(int i, int j, int k);
    inline double getYGradient(int i, int j, int k);
    inline double getZGradient(int i, int j, int k);
    inline std::array<double, 3> getNormal(int i, int j, int k);

    void processCube(int i, int j, int k, int configurationIndex);
};
