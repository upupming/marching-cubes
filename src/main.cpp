#include <QApplication>
#include <QMainWindow>
#include <QtConcurrent>
#include <array>
#include <iostream>

#include "main_window.h"
#include "marching_cubes.h"
#include "mesh_view_widget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QCoreApplication::setOrganizationName("upupming");
    app.setApplicationName("MarchingCubes, LEWINER 2003");
    app.setApplicationVersion(QT_VERSION_STR);

#if _DEBUG
    std::cout << "Debug mode" << std::endl;
#else
    std::cout << "Release mode" << std::endl;
#endif

#ifndef QT_NO_OPENGL
    // 改为 true 可以开启无限循环自动测试
    MainWindow mainWin(false);
    mainWin.show();
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}
