#pragma once

#include <QFuture>
#include <QMainWindow>

#include "mesh_view_widget.h"
class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    MainWindow();

   protected:
    void closeEvent(QCloseEvent *event) override;

   private:
    void runMarchingCubes(float isoValue);
    void readSettings();
    void writeSettings();
    MeshViewWidget *meshViewWidget = nullptr;
    MarchingCubes *mc = nullptr;
    float currentIsoValue = 0;
    QFuture<void> mcProcess;
    // https://forum.qt.io/topic/52989/solved-accessing-ui-from-qtconcurrent-run/4
   signals:
    void marchingCubesFinished();
   public slots:
    void updateMeshView();
    void updateIsoValue(float isoValue);
};
