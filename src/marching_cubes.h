/*
Marching Cubes 算法实现
传入一个体数据
返回所有顶点、所有三角形（点用索引表示）、法线
*/
class MarchingCubes {
   public:
    MarchingCubes(const unsigned short* data, double x_spacing, double y_spacing, double z_spacing);

   private:
    unsigned short* data;
    double x_spacing, y_spacing, z_spacing;
};
