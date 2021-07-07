#include <QApplication>
#include <QMainWindow>
#include <QtConcurrent>
#include <array>
#include <iostream>

#include "marching_cubes.h"
#include "mesh_view_widget.h"
#include "raw_reader.h"

void testMarchingCubes() {
    const int Z = 507, Y = 512, X = 512;
    RawReader rawReader("../../data/cbct_sample_z=507_y=512_x=512.raw", Z, Y, X);
    const unsigned short *data = rawReader.data();

    std::array<int, 3> dim{Z, Y, X};
    std::array<float, 3> spacing{0.3, 0.3, 0.3};
    float isoValue = 800;
    MarchingCubes *mc = new MarchingCubes(data, dim, spacing, true);
    mc->runAlgorithm(isoValue);
    // mc->saveObj("../../data/cbct_isoValue=" + std::to_string(isoValue) + ".obj");

    MeshViewWidget *widget = new MeshViewWidget(mc);
    widget->show();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("MarchingCubes, LEWINER 2003");
    app.setApplicationVersion("1.0");

#if _DEBUG
    std::cout << "Debug mode" << std::endl;
#else
    std::cout << "Release mode" << std::endl;
#endif

#ifndef QT_NO_OPENGL
    testMarchingCubes();
    // QtConcurrent::run(testMarchingCubes);
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}
