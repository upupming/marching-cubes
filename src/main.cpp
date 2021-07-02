#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>

#include "marching_cubes.h"
#include "raw_reader.h"

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

    const int Z = 507, Y = 512, X = 512;
    RawReader rawReader("./data/cbct_sample_z=507_y=512_x=512.raw", Z, Y, X);
    const unsigned short *data = rawReader.data();

    MarchingCubes mc(data, 0.3, 0.3, 0.3);

    return app.exec();
}
