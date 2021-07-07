#include "marching_cubes.h"

#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>

#include "LookUpTable.h"

MarchingCubes::MarchingCubes(const unsigned short* data, std::array<int, 3> dim, std::array<float, 3> spacing, bool reverseGradientDirection) {
    this->data = data;
    this->dim = dim;
    this->spacing = spacing;
    this->reverseGradientDirection = reverseGradientDirection;
    omp_init_lock(&vertexLock);
    omp_init_lock(&triangleLock);
}

MarchingCubes::~MarchingCubes() {
    omp_destroy_lock(&vertexLock);
    omp_destroy_lock(&triangleLock);
}

void MarchingCubes::runAlgorithm(float isoValue) {
    clock_t time = clock();

    bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
    bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

    this->isoValue = isoValue;
    // 计算所有插值顶点
    computeInterpolatedVertices();

// 运行 marching cubes 算法，marching 并逐个处理 cube
#pragma omp parallel for collapse(4)
    for (int i = 0; i < dim[0] - 1; i++) {
        for (int j = 0; j < dim[1] - 1; j++) {
            for (int k = 0; k < dim[2] - 1; k++) {
                std::vector<float> cube(8);
                // 计算 configuration 编号
                int configurationIndex = 0;
                for (int l = 0; l < 8; l++) {
                    cube[l] =
                        getData(
                            // 编号 1, 2, 5, 6 的话 i 需要 + 1，这些数的后两位异或为 1
                            i + ((l ^ (l >> 1)) & 1),
                            // 编号 2, 3, 6, 7 的话 j 需要 + 1，这些数的倒数第 2 位为 1
                            j + ((l >> 1) & 1),
                            // 编号 4, 5, 6, 7 的话 k 需要 + 1，这些数的倒数第 3 为为 1
                            k + ((l >> 2) & 1));
                    if (cube[l] > 0)
                        configurationIndex |= 1 << l;
                }
                processCube(i, j, k, configurationIndex, cube);
            }
        }
    }

    maxExtent = 0.5 * (bmax[0] - bmin[0]);
    if (maxExtent < 0.5 * (bmax[1] - bmin[1])) {
        maxExtent = 0.5 * (bmax[1] - bmin[1]);
    }
    if (maxExtent < 0.5 * (bmax[2] - bmin[2])) {
        maxExtent = 0.5 * (bmax[2] - bmin[2]);
    }

    printf("Marching Cubes ran in %lf secs.\n", (float)(clock() - time) / CLOCKS_PER_SEC);
}

void MarchingCubes::processCube(int i, int j, int k, int configurationIndex, const std::vector<float>& cube) {
    if (i % 100 == 0 && j == 0 && k == 0)
        std::cout << "processCube: " << i << " " << j << " " << k << std::endl;

    // 注意对于有一些为了解决内部歧义的情况（例如 6.1.2），需要在 cube 正中间插值算一个顶点，这个顶点的标号为 12
    // 原作者是主动创建点 12，我是放在了 `getCubeVertexIndex` 函数里面如果需要才创建，稍微简洁一些
    int caseIdx = cases[configurationIndex][0];
    int configurationIndexInCase = cases[configurationIndex][1];
    // subconfig 由多个 face 测试的二进制表示
    int subconfig = 0, subconfig13Value;
    // 参考 Table 1: A reduced representation of the test table. Case 13 has 45 entries to map the results of all the possible tests to the right subcase.
    switch (caseIdx) {
        // 0: 不需要三角形
        case 0:
            break;
        // 1: 需要 1 个三角形
        case 1:
            addTriangle(i, j, k, {tiling1[configurationIndexInCase], tiling1[configurationIndexInCase] + 1 * 3});
            break;
        // 2: 需要 2 个三角形
        case 2:
            addTriangle(i, j, k, {tiling2[configurationIndexInCase], tiling2[configurationIndexInCase] + 2 * 3});
            break;
        // 3. 需要对 1 个面进行测试，- 使用 3.1，+ 使用 3.2
        case 3:
            if (testFace(i, j, k, cube, test3[configurationIndexInCase]) < 0) {
                // 3.1: 需要 2 个三角形
                addTriangle(i, j, k, {tiling3_1[configurationIndexInCase], tiling3_1[configurationIndexInCase] + 2 * 3});
            } else {
                // 3.2: 需要 4 个三角形
                addTriangle(i, j, k, {tiling3_2[configurationIndexInCase], tiling3_2[configurationIndexInCase] + 4 * 3});
            }
            break;
        // 4. 需要对 1 个 interior 进行测试，- 使用 4.1，+ 使用 4.2 （论文的  table 1 写错了，应该写在 interior 的测试写在了 face 上）
        case 4:
            // 4 的 edgeIdx 不重要，因为其强的对称性
            if (testInterior(i, j, k, cube, caseIdx, test4[configurationIndexInCase], 1) < 0) {
                // 4.1: 需要 2 个三角形
                addTriangle(i, j, k, {tiling4_1[configurationIndexInCase], tiling4_1[configurationIndexInCase] + 2 * 3});
            } else {
                // 4.2: 需要 6 个三角形
                addTriangle(i, j, k, {tiling4_2[configurationIndexInCase], tiling4_2[configurationIndexInCase] + 6 * 3});
            }
            break;
        // 5: 需要 3 个三角形
        case 5:
            addTriangle(i, j, k, {tiling5[configurationIndexInCase], tiling5[configurationIndexInCase] + 3 * 3});
            break;
        // 6: 需要测试 1 个面，同时测试 1 个 interior
        case 6:
            if (testFace(i, j, k, cube, test6[configurationIndexInCase][0]) < 0) {
                // 6.1
                if (testInterior(i, j, k, cube, caseIdx, test6[configurationIndexInCase][1], test6[configurationIndexInCase][2]) < 0) {
                    // 6.1.1: 3 个三角形
                    addTriangle(i, j, k, {tiling6_1_1[configurationIndexInCase], tiling6_1_1[configurationIndexInCase] + 3 * 3});
                } else {
                    // 6.1.2: 9 个三角形（论文里面错写成 7 了）
                    addTriangle(i, j, k, {tiling6_1_2[configurationIndexInCase], tiling6_1_2[configurationIndexInCase] + 9 * 3});
                }
            } else {
                // 6.2: 5 个三角形
                addTriangle(i, j, k, {tiling6_2[configurationIndexInCase], tiling6_2[configurationIndexInCase] + 5 * 3});
            }
            break;
        // 7: 需要测试 3 个面，同时测试 1 个 interior
        case 7:
            if (testFace(i, j, k, cube, test7[configurationIndexInCase][0]) < 0) {
                subconfig += 1;
            }
            if (testFace(i, j, k, cube, test7[configurationIndexInCase][1]) < 0) {
                subconfig += 2;
            }
            if (testFace(i, j, k, cube, test7[configurationIndexInCase][2]) < 0) {
                subconfig += 4;
            }
            switch (subconfig) {
                case 0:
                    // 7.1: 3 个三角形
                    addTriangle(i, j, k, {tiling7_1[configurationIndexInCase], tiling7_1[configurationIndexInCase] + 3 * 3});
                    break;
                case 1:
                    // 7.2: 5 个三角形
                    addTriangle(i, j, k, {tiling7_2[configurationIndexInCase][0], tiling7_2[configurationIndexInCase][0] + 5 * 3});
                    break;
                case 2:
                    // 7.2: 5 个三角形
                    addTriangle(i, j, k, {tiling7_2[configurationIndexInCase][1], tiling7_2[configurationIndexInCase][1] + 5 * 3});
                    break;
                case 3:
                    // 7.3: 9 个三角形
                    addTriangle(i, j, k, {tiling7_3[configurationIndexInCase][0], tiling7_3[configurationIndexInCase][0] + 9 * 3});
                    break;
                case 4:
                    // 7.2: 5 个三角形
                    addTriangle(i, j, k, {tiling7_2[configurationIndexInCase][2], tiling7_2[configurationIndexInCase][2] + 5 * 3});
                    break;
                case 5:
                    // 7.3: 9 个三角形
                    addTriangle(i, j, k, {tiling7_3[configurationIndexInCase][1], tiling7_3[configurationIndexInCase][1] + 9 * 3});
                    break;
                case 6:
                    // 7.3: 9 个三角形
                    // v12
                    addTriangle(i, j, k, {tiling7_3[configurationIndexInCase][2], tiling7_3[configurationIndexInCase][2] + 9 * 3});
                    break;
                case 7:
                    if (testInterior(i, j, k, cube, caseIdx, test7[configurationIndexInCase][3], test7[configurationIndexInCase][4]) > 0) {
                        // 7.4.1: 5 个三角形（论文里面写成 9 是错了）
                        addTriangle(i, j, k, {tiling7_4_1[configurationIndexInCase], tiling7_4_1[configurationIndexInCase] + 5 * 3});
                    } else {
                        // 7.4.2: 9 个三角形
                        addTriangle(i, j, k, {tiling7_4_2[configurationIndexInCase], tiling7_4_2[configurationIndexInCase] + 9 * 3});
                    }
                    break;
                default:
                    assert(false);
            }
            break;
        // 8: 2 个三角形
        case 8:
            addTriangle(i, j, k, {tiling8[configurationIndexInCase], tiling8[configurationIndexInCase] + 2 * 3});
            break;
        // 9: 4 个三角形
        case 9:
            addTriangle(i, j, k, {tiling9[configurationIndexInCase], tiling9[configurationIndexInCase] + 4 * 3});
            break;
        // 10: 测试 2 个面，1 个 interior
        case 10:
            if (testFace(i, j, k, cube, test10[configurationIndexInCase][0]) > 0) {
                subconfig += 1;
            }
            if (testFace(i, j, k, cube, test10[configurationIndexInCase][1]) > 0) {
                subconfig += 2;
            }
            switch (subconfig) {
                case 0:
                    // 10 的 edgeIdx 不重要，因为其强的对称性
                    if (testInterior(i, j, k, cube, caseIdx, test10[configurationIndexInCase][2], 1) < 0) {
                        // 10.1.1： 4 个三角形
                        addTriangle(i, j, k, {tiling10_1_1[configurationIndexInCase], tiling10_1_1[configurationIndexInCase] + 4 * 3});
                    } else {
                        // 10.1.2： 8 个三角形
                        addTriangle(i, j, k, {tiling10_1_2[configurationIndexInCase], tiling10_1_2[configurationIndexInCase] + 8 * 3});
                    }
                    break;
                case 1:
                    // 10.2: 8 个三角形
                    // v12
                    addTriangle(i, j, k, {tiling10_2[configurationIndexInCase], tiling10_2[configurationIndexInCase] + 8 * 3});
                    break;
                case 2:
                    // 10.2: 8 个三角形
                    // v12
                    addTriangle(i, j, k, {tiling10_2_[configurationIndexInCase], tiling10_2_[configurationIndexInCase] + 8 * 3});
                    break;
                case 3:
                    // 10.1.1: 4 个三角形
                    addTriangle(i, j, k, {tiling10_1_1_[configurationIndexInCase], tiling10_1_1_[configurationIndexInCase] + 4 * 3});
                    break;
                default:
                    assert(false);
            }
            break;
        // 11: 4 个三角形
        case 11:
            addTriangle(i, j, k, {tiling11[configurationIndexInCase], tiling11[configurationIndexInCase] + 4 * 3});
            break;
        // 12: 跟 10 一样的套路
        case 12:
            if (testFace(i, j, k, cube, test12[configurationIndexInCase][0]) > 0) {
                subconfig += 1;
            }
            if (testFace(i, j, k, cube, test12[configurationIndexInCase][1]) > 0) {
                subconfig += 2;
            }
            switch (subconfig) {
                case 0:
                    // 12 的 alongEdge 需要
                    if (testInterior(i, j, k, cube, caseIdx, test12[configurationIndexInCase][2], test12[configurationIndexInCase][3]) < 0) {
                        // 12.1.1： 4 个三角形
                        addTriangle(i, j, k, {tiling12_1_1[configurationIndexInCase], tiling12_1_1[configurationIndexInCase] + 4 * 3});
                    } else {
                        // 12.1.2： 8 个三角形
                        addTriangle(i, j, k, {tiling12_1_2[configurationIndexInCase], tiling12_1_2[configurationIndexInCase] + 8 * 3});
                    }
                    break;
                case 1:
                    // 12.2: 8 个三角形
                    // v12
                    addTriangle(i, j, k, {tiling12_2[configurationIndexInCase], tiling12_2[configurationIndexInCase] + 8 * 3});
                    break;
                case 2:
                    // 12.2: 8 个三角形
                    // v12
                    addTriangle(i, j, k, {tiling12_2_[configurationIndexInCase], tiling12_2_[configurationIndexInCase] + 8 * 3});
                    break;
                case 3:
                    // 12.1.1: 4 个三角形
                    addTriangle(i, j, k, {tiling12_1_1_[configurationIndexInCase], tiling12_1_1_[configurationIndexInCase] + 4 * 3});
                    break;
                default:
                    assert(false);
            }
            break;
        // 13: 最为特殊的例子，有一个专门的 subconfig13 来存储配置映射信息，需要测试 6 个面和 1 个 interior
        case 13:
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][0]) > 0) {
                subconfig += 1;
            }
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][1]) > 0) {
                subconfig += 2;
            }
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][2]) > 0) {
                subconfig += 4;
            }
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][3]) > 0) {
                subconfig += 8;
            }
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][4]) > 0) {
                subconfig += 16;
            }
            if (testFace(i, j, k, cube, test13[configurationIndexInCase][5]) > 0) {
                subconfig += 32;
            }
            subconfig13Value = subconfig13[subconfig];
            // subconfig13 中其实负数出现，表示这种情况是一定不会出现的
            if (subconfig13Value == 0) {
                // 13.1
                addTriangle(i, j, k, {tiling13_1[configurationIndexInCase], tiling13_1[configurationIndexInCase] + 4 * 3});
            } else if (subconfig13Value <= 6) {
                // 13.2
                addTriangle(i, j, k, {tiling13_2[configurationIndexInCase][subconfig13Value - 1], tiling13_2[configurationIndexInCase][subconfig13Value - 1] + 6 * 3});
            } else if (subconfig13Value <= 18) {
                // 13.3
                addTriangle(i, j, k, {tiling13_3[configurationIndexInCase][subconfig13Value - 7], tiling13_3[configurationIndexInCase][subconfig13Value - 7] + 10 * 3});
            } else if (subconfig13Value <= 22) {
                // 13.4
                addTriangle(i, j, k, {tiling13_4[configurationIndexInCase][subconfig13Value - 19], tiling13_4[configurationIndexInCase][subconfig13Value - 19] + 12 * 3});
            } else if (subconfig13Value <= 26) {
                // 13.5
                // 这个的 edgeIdx 比较特殊，是从 tiling13_5_1 里面拿的
                if (testInterior(i, j, k, cube, caseIdx, test13[configurationIndexInCase][6], tiling13_5_1[configurationIndexInCase][subconfig13Value][0]) < 0) {
                    addTriangle(i, j, k, {tiling13_5_1[configurationIndexInCase][subconfig13Value - 23], tiling13_5_1[configurationIndexInCase][subconfig13Value - 23] + 6 * 3});
                } else {
                    addTriangle(i, j, k, {tiling13_5_2[configurationIndexInCase][subconfig13Value - 23], tiling13_5_2[configurationIndexInCase][subconfig13Value - 23] + 10 * 3});
                }
            } else if (subconfig13Value <= 38) {
                // 13.3
                addTriangle(i, j, k, {tiling13_3_[configurationIndexInCase][subconfig13Value - 27], tiling13_3_[configurationIndexInCase][subconfig13Value - 27] + 10 * 3});
            } else if (subconfig13Value <= 44) {
                // 13.2
                addTriangle(i, j, k, {tiling13_2_[configurationIndexInCase][subconfig13Value - 39], tiling13_2_[configurationIndexInCase][subconfig13Value - 39] + 6 * 3});
            } else if (subconfig13Value == 45) {
                // 13.1
                addTriangle(i, j, k, {tiling13_1_[configurationIndexInCase], tiling13_1_[configurationIndexInCase] + 6 * 3});
            } else {
                assert(false);
            }
            break;
        case 14:
            // 14: 4 个三角形
            addTriangle(i, j, k, {tiling14[configurationIndexInCase], tiling14[configurationIndexInCase] + 4 * 3});
            break;
        default:
            assert(false);
    }
}

void MarchingCubes::addTriangle(int i, int j, int k, std::vector<char> edges) {
#pragma omp parallel for
    for (int l = 0; l < edges.size(); l += 3) {
        int a = getCubeVertexIndex(i, j, k, edges[l]);
        int b = getCubeVertexIndex(i, j, k, edges[l + 1]);
        int c = getCubeVertexIndex(i, j, k, edges[l + 2]);
        if (a == -1 || b == -1 || c == -1) {
            assert(false);
        }
        if (a == b || b == c || a == c) {
            assert(false);
        }
        omp_set_lock(&triangleLock);  //获得互斥器
        triangles.push_back({a, b, c});
        omp_unset_lock(&triangleLock);  //释放互斥器
    }
}

float MarchingCubes::testFace(int i, int j, int k, const std::vector<float>& cube, int f) {
    // 渐近线测试所用的四个参数
    int A, B, C, D;
    // 注意有的面需要将结果取反，这个时候 f 是负数
    // 参考 Figure 6 第 3 个图，这个顺序 ABCD 的应该是作者自己确定的，论文里面没写，这个顺序跟测试是否带正负号相关
    switch (f) {
        case 1:
        case -1:
            A = 0, B = 4, C = 5, D = 1;
            break;
        case 2:
        case -2:
            A = 1, B = 5, C = 6, D = 2;
            break;
        case 3:
        case -3:
            A = 2, B = 6, C = 7, D = 3;
            break;
        case 4:
        case -4:
            A = 3, B = 7, C = 4, D = 0;
            break;
        case 5:
        case -5:
            A = 0, B = 3, C = 2, D = 1;
            break;
        case 6:
        case -6:
            A = 4, B = 7, C = 6, D = 5;
            break;
        default:
            assert(false);
    }

    return f * cube[A] * (cube[A] * cube[C] - cube[B] * cube[D]);
}

float MarchingCubes::testInterior(int i, int j, int k, const std::vector<float>& cube, int caseIdx, int alongEdgeIdx, int edgeIdx) {
    float t, At = 0, Bt = 0, Ct = 0, Dt = 0, a, b;
    switch (caseIdx) {
        // 强对称性，直接计算
        case 4:
        case 10:
            a = (cube[4] - cube[0]) * (cube[6] - cube[2]) - (cube[7] - cube[3]) * (cube[5] - cube[1]);
            b = cube[2] * (cube[4] - cube[0]) + cube[0] * (cube[6] - cube[2]) - cube[1] * (cube[7] - cube[3]) - cube[3] * (cube[5] - cube[1]);
            t = -b / (2 * a);
            if (t > 0 || t < 1) return -alongEdgeIdx;
            At = cube[0] + (cube[4] - cube[0]) * t;
            Bt = cube[3] + (cube[7] - cube[3]) * t;
            Ct = cube[2] + (cube[6] - cube[2]) * t;
            Dt = cube[1] + (cube[5] - cube[1]) * t;
            break;
        // 没有强对称性，根据 edgeIdx 计算
        case 6:
        case 7:
        case 12:
        case 13:
            switch (edgeIdx) {
                case 0:
                    t = cube[0] / (cube[0] - cube[1]);
                    At = 0;
                    Bt = cube[3] + (cube[2] - cube[3]) * t;
                    Ct = cube[7] + (cube[6] - cube[7]) * t;
                    Dt = cube[4] + (cube[5] - cube[4]) * t;
                    break;
                case 1:
                    t = cube[1] / (cube[1] - cube[2]);
                    At = 0;
                    Bt = cube[0] + (cube[3] - cube[0]) * t;
                    Ct = cube[4] + (cube[7] - cube[4]) * t;
                    Dt = cube[5] + (cube[6] - cube[5]) * t;
                    break;
                case 2:
                    t = cube[2] / (cube[2] - cube[3]);
                    At = 0;
                    Bt = cube[1] + (cube[0] - cube[1]) * t;
                    Ct = cube[5] + (cube[4] - cube[5]) * t;
                    Dt = cube[6] + (cube[7] - cube[6]) * t;
                    break;
                case 3:
                    t = cube[3] / (cube[3] - cube[0]);
                    At = 0;
                    Bt = cube[2] + (cube[1] - cube[2]) * t;
                    Ct = cube[6] + (cube[5] - cube[6]) * t;
                    Dt = cube[7] + (cube[4] - cube[7]) * t;
                    break;
                case 4:
                    t = cube[4] / (cube[4] - cube[5]);
                    At = 0;
                    Bt = cube[7] + (cube[6] - cube[7]) * t;
                    Ct = cube[3] + (cube[2] - cube[3]) * t;
                    Dt = cube[0] + (cube[1] - cube[0]) * t;
                    break;
                case 5:
                    t = cube[5] / (cube[5] - cube[6]);
                    At = 0;
                    Bt = cube[4] + (cube[7] - cube[4]) * t;
                    Ct = cube[0] + (cube[3] - cube[0]) * t;
                    Dt = cube[1] + (cube[2] - cube[1]) * t;
                    break;
                case 6:
                    t = cube[6] / (cube[6] - cube[7]);
                    At = 0;
                    Bt = cube[5] + (cube[4] - cube[5]) * t;
                    Ct = cube[1] + (cube[0] - cube[1]) * t;
                    Dt = cube[2] + (cube[3] - cube[2]) * t;
                    break;
                case 7:
                    t = cube[7] / (cube[7] - cube[4]);
                    At = 0;
                    Bt = cube[6] + (cube[5] - cube[6]) * t;
                    Ct = cube[2] + (cube[1] - cube[2]) * t;
                    Dt = cube[3] + (cube[0] - cube[3]) * t;
                    break;
                case 8:
                    t = cube[0] / (cube[0] - cube[4]);
                    At = 0;
                    Bt = cube[3] + (cube[7] - cube[3]) * t;
                    Ct = cube[2] + (cube[6] - cube[2]) * t;
                    Dt = cube[1] + (cube[5] - cube[1]) * t;
                    break;
                case 9:
                    t = cube[1] / (cube[1] - cube[5]);
                    At = 0;
                    Bt = cube[0] + (cube[4] - cube[0]) * t;
                    Ct = cube[3] + (cube[7] - cube[3]) * t;
                    Dt = cube[2] + (cube[6] - cube[2]) * t;
                    break;
                case 10:
                    t = cube[2] / (cube[2] - cube[6]);
                    At = 0;
                    Bt = cube[1] + (cube[5] - cube[1]) * t;
                    Ct = cube[0] + (cube[4] - cube[0]) * t;
                    Dt = cube[3] + (cube[7] - cube[3]) * t;
                    break;
                case 11:
                    t = cube[3] / (cube[3] - cube[7]);
                    At = 0;
                    Bt = cube[2] + (cube[6] - cube[2]) * t;
                    Ct = cube[1] + (cube[5] - cube[1]) * t;
                    Dt = cube[0] + (cube[4] - cube[0]) * t;
                    break;
                default:
                    assert(false);
                    break;
            }
            break;
        default:
            assert(false);
    }

    int test = 0;
    if (At >= 0) test++;
    if (Bt >= 0) test += 2;
    if (Ct >= 0) test += 4;
    if (Dt >= 0) test += 8;
    if (test <= 4)
        return -alongEdgeIdx;
    else if (test == 5)
        return (At * Ct - Bt * Dt) * alongEdgeIdx;
    else if (test == 6)
        return -alongEdgeIdx;
    else if (test == 7)
        return alongEdgeIdx;
    else if (test <= 9)
        return -alongEdgeIdx;
    else if (test == 10)
        return -(At * Ct - Bt * Dt) * alongEdgeIdx;
    else if (test == 11)
        return alongEdgeIdx;
    else if (test == 12)
        return -alongEdgeIdx;
    else if (test <= 15)
        return alongEdgeIdx;
    else {
        assert(false);
        return -1;
    }
}
