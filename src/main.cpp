#include <QApplication>
#include <QtConcurrent>
#include <array>
#include <iostream>

#include "mainwidget.h"
#include "marching_cubes.h"
#include "raw_reader.h"

void testMarchingCubes() {
    const int Z = 507, Y = 512, X = 512;
    RawReader rawReader("./data/cbct_sample_z=507_y=512_x=512.raw", Z, Y, X);
    const unsigned short *data = rawReader.data();

    std::array<int, 3> dim{Z, Y, X};
    std::array<float, 3> spacing{0.3, 0.3, 0.3};
    float isoValue = 800;
    MarchingCubes *mc = new MarchingCubes(data, dim, spacing, true);
    mc->runAlgorithm(isoValue);
    mc->saveObj("./data/cbct_isoValue=" + std::to_string(isoValue) + ".obj");

    MainWidget *widget = new MainWidget(mc);
    widget->show();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("cube");
    app.setApplicationVersion("0.1");

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
