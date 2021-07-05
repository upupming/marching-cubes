#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtConcurrent>
#include <array>
#include <iostream>

#include "marching_cubes.h"
#include "raw_reader.h"

void testMarchingCubes() {
    const int Z = 507, Y = 512, X = 512;
    RawReader rawReader("./data/cbct_sample_z=507_y=512_x=512.raw", Z, Y, X);
    const unsigned short *data = rawReader.data();

    std::array<int, 3> dim{Z, Y, X};
    std::array<double, 3> spacing{0.3, 0.3, 0.3};
    double isoValue = 800;
    MarchingCubes mc(data, dim, spacing);
    mc.runAlgorithm(isoValue);
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    qDebug() << "Hello world";
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

#if _DEBUG
    std::cout << "Debug mode" << std::endl;
#else
    std::cout << "Release mode" << std::endl;
#endif

    QtConcurrent::run(testMarchingCubes);

    return app.exec();
}
