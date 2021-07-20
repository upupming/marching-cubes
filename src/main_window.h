#pragma once

#include <QFuture>
#include <QMainWindow>
#include <QtConcurrent>
#include <QtWidgets>

#include "mesh_view_widget.h"
#include "raw_reader.h"
class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    /**
    * \param autoTest 不断循环测试不同的 isoValue
    */
    MainWindow(bool autoTest = false);
    ~MainWindow();

   protected:
    void closeEvent(QCloseEvent *event) override;

   private:
    void runMarchingCubes(float isoValue);
    void readData();
    void readSettings();
    void writeSettings();
    MeshViewWidget *meshViewWidget = nullptr;
    QSlider *slider = nullptr;
    MarchingCubes *mc = nullptr;
    float currentIsoValue = -1;
    QFuture<void> mcProcess, readDataProcess;
    RawReader *rawReader;
    const int Z = 507, Y = 512, X = 512;
    const int MAX_ISO_VALUE = 4000;
    bool autoTest = false;
    // https://forum.qt.io/topic/52989/solved-accessing-ui-from-qtconcurrent-run/4
   signals:
    void marchingCubesFinished();
   public slots:
    void updateMeshView();
    void updateIsoValue(float isoValue);
};
