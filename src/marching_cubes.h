/*
Marching Cubes 算法实现
参考论文: efficient implementation of Marching Cubes’ cases with topological guarantees
*/
#pragma once
#include <omp.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>
struct Vertex {
    // 顶点坐标
    double x, y, z;
    // 法向量
    double nx, ny, nz;
    Vertex(double x, double y, double z, double nx, double ny, int nz) : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz) {
        normalizeNormal();
    }
    Vertex& operator+=(const Vertex& rhs) {
        x += rhs.x, y += rhs.y, z += rhs.z;
        nx += rhs.nx, ny += rhs.ny, nz += rhs.nz;
        return *this;
    }
    Vertex& operator/=(const int n) {
        x /= n, y /= n, z /= n;
        nx /= n, ny /= n, nz /= n;
        return *this;
    }
    void normalizeNormal() {
        double len2 = nx * nx + ny * ny + nz * nz;
        double len = sqrt(len2);
        nx /= len, ny /= len, nz /= len;
    }
};

class MarchingCubes {
   public:
    MarchingCubes(const unsigned short* data, std::array<int, 3> dim, std::array<double, 3> spacing, bool reverseGradientDirection = false);
    MarchingCubes::~MarchingCubes();
    /**
    * 运行算法，生成顶点（带法线）、三角形
    * \param isoValue 等值面大小
    **/
    void runAlgorithm(double isoValue);
    inline std::vector<Vertex> getVertices() { return vertices; }
    inline std::vector<std::array<int, 3>> getTriangles() { return triangles; }
    void saveObj(std::string filename);

   private:
    omp_lock_t vertexLock, triangleLock;
    const unsigned short* data;
    std::array<int, 3> dim;
    std::array<double, 3> spacing{1, 1, 1};
    bool reverseGradientDirection = false;
    std::vector<Vertex> vertices;
    // 所有的三角形，其中每个三角形是 3 个 Vertex 在 vertices 中的索引下标
    std::vector<std::array<int, 3>> triangles;
    double isoValue;
    inline double getData(int i, int j, int k) {
        double val = data[i * dim[1] * dim[2] + j * dim[2] + k] - isoValue;
        // 如果返回 0 的话，后面计算边的插值点的时候会出问题（要么插值就是 cube 顶点，要么不插值，都是不对的，前者会造成三角形塌陷成两个点，后者会造成没有顶点用来构成三角形）
        if (abs(val) < FLT_EPSILON) {
            val = FLT_EPSILON;
        }
        return val;
    }

    // xDirectionInterpolatedVertexIndex[i][j][k]
    // 表示以 (i, j, k) 点向 x 方向的边上的插值顶点
    // 注意对于两个点的正负性相同的边，中间是不需要插值顶点的
    // x 方向又叫 horizontal 方向
    // y 方向又叫 longitudinal 方向
    // z 方向又叫 vertical 方向
    // 对于有一些为了解决内部歧义的情况（例如 6.1.2），需要在 cube 正中间插值算一个顶点，这个顶点的标号为 12，存在 centerInterpolatedVertexIndex 里面，由于并不是所有 cube 都有 12，因此在 processCube 实际用到的时候才去添加
    std::vector<std::vector<std::vector<int>>> xDirectionInterpolatedVertexIndex, yDirectionInterpolatedVertexIndex, zDirectionInterpolatedVertexIndex,
        centerInterpolatedVertexIndex;
    void computeInterpolatedVertices();
    /**
     * \brief 在 cube 正中心生成一个 vertex 并放入 centerInterpolatedVertexIndex 中
     */
    void addCenterVertex(int i, int j, int k);
    // 给定 cube 坐标和 edge 编号，求出 vertex 编号
    int getCubeVertexIndex(int i, int j, int k, int edgeIdx);

    // 梯度方向就是法向量方向
    inline double getXGradient(int i, int j, int k);
    inline double getYGradient(int i, int j, int k);
    inline double getZGradient(int i, int j, int k);
    inline std::array<double, 3> getNormal(int i, int j, int k);

    void processCube(int i, int j, int k, int configurationIndex, const std::vector<double>& cube);
    // 根据 tiling 数组里面的需要连接的边，连接对应的三角形
    void addTriangle(int i, int j, int k, std::vector<char> edges);
    double testFace(int i, int j, int k, const std::vector<double>& cube, int f);
    /**
     * \brief 测试内部
     * \param caseIdx 论文中规定的 0-14 种 case
     * \param alongEdgeIdx 表示插值平面 P 的方向, the direction of P is encoded as an edge
     * e inside one particular sequence of the tiling table，主要还是关心这个值得正负性，在结果上会乘上这个
     * \param edgeIdx 表示这条边的两个点当做两个 A0 和 A1 点
     */
    double testInterior(int i, int j, int k, const std::vector<double>& cube, int caseIdx, int alongEdgeIdx, int edgeIdx);
};
