/*
Marching Cubes 算法实现
参考论文: efficient implementation of Marching Cubes’ cases with topological guarantees
*/
#pragma once

#include <concurrent_unordered_map.h>
#include <omp.h>

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
struct Vertex {
    // 顶点坐标
    float x, y, z;
    // 法向量
    float nx, ny, nz;
    Vertex(float x, float y, float z, float nx, float ny, int nz) : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz) {
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
        float len2 = nx * nx + ny * ny + nz * nz;
        float len = sqrt(len2);
        nx /= len, ny /= len, nz /= len;
    }
};

class MarchingCubes {
   public:
    MarchingCubes(const unsigned short* data, std::array<int, 3> dim, std::array<float, 3> spacing, bool reverseGradientDirection = false);
    MarchingCubes::~MarchingCubes();
    /**
    * 运行算法，生成顶点（带法线）、三角形
    * \param isoValue 等值面大小
    **/
    void runAlgorithm(float isoValue);
    // bounding box
    float bmax[3], bmin[3], maxExtent;
    inline const std::vector<Vertex>& getVertices() const {
        return vertices;
    }
    inline const std::vector<std::array<int, 3>>& getTriangles() const {
        return triangles;
    }
    void saveObj(std::string filename);

   private:
    omp_lock_t vertexLock, triangleLock;
    const unsigned short* data;
    std::array<int, 3> dim;
    std::array<float, 3> spacing{1.f, 1.f, 1.f};
    bool reverseGradientDirection = false;
    std::vector<Vertex> vertices;
    inline void addVertex(int i, int j, int k, concurrency::concurrent_unordered_map<glm::ivec3, int>& vertexIndex, const Vertex& v) {
        omp_set_lock(&vertexLock);
        vertices.push_back(v);
        bmin[0] = std::min(bmin[0], v.x), bmax[0] = std::max(bmax[0], v.x);
        bmin[1] = std::min(bmin[1], v.y), bmax[1] = std::max(bmax[1], v.y);
        bmin[2] = std::min(bmin[2], v.z), bmax[2] = std::max(bmax[2], v.z);
        vertexIndex[glm::ivec3(i, j, k)] = vertices.size() - 1;
        omp_unset_lock(&vertexLock);
    };
    // 所有的三角形，其中每个三角形是 3 个 Vertex 在 vertices 中的索引下标
    std::vector<std::array<int, 3>> triangles;
    inline int addTriangle(const std::array<int, 3>& t) {
        omp_set_lock(&triangleLock);
        triangles.push_back(t);
        omp_unset_lock(&triangleLock);
        return triangles.size() - 1;
    }
    float isoValue;
    inline float getData(int i, int j, int k) {
        float val = data[i * dim[1] * dim[2] + j * dim[2] + k] - isoValue;
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
    concurrency::concurrent_unordered_map<glm::ivec3, int>
        xDirectionInterpolatedVertexIndex,
        yDirectionInterpolatedVertexIndex,
        zDirectionInterpolatedVertexIndex,
        centerInterpolatedVertexIndex;
    void computeInterpolatedVertices();
    /**
     * \brief 在 cube 正中心生成一个 vertex 并放入 centerInterpolatedVertexIndex 中
     */
    void addCenterVertex(int i, int j, int k);
    // 给定 cube 坐标和 edge 编号，求出 vertex 编号
    int getCubeVertexIndex(int i, int j, int k, int edgeIdx);

    // 梯度方向就是法向量方向
    inline float getXGradient(int i, int j, int k);
    inline float getYGradient(int i, int j, int k);
    inline float getZGradient(int i, int j, int k);
    inline std::array<float, 3> getNormal(int i, int j, int k);

    void processCube(int i, int j, int k, int configurationIndex, const std::vector<float>& cube);
    // 根据 tiling 数组里面的需要连接的边，连接对应的三角形
    void addTriangle(int i, int j, int k, std::vector<char> edges);
    float testFace(int i, int j, int k, const std::vector<float>& cube, int f);
    /**
     * \brief 测试内部
     * \param caseIdx 论文中规定的 0-14 种 case
     * \param alongEdgeIdx 表示插值平面 P 的方向, the direction of P is encoded as an edge
     * e inside one particular sequence of the tiling table，主要还是关心这个值得正负性，在结果上会乘上这个
     * \param edgeIdx 表示这条边的两个点当做两个 A0 和 A1 点
     */
    float testInterior(int i, int j, int k, const std::vector<float>& cube, int caseIdx, int alongEdgeIdx, int edgeIdx);
};
