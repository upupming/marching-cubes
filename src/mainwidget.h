#ifndef MAINWIDGET_H
#define MAINWIDGET_H
#include <QBasicTimer>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QQuaternion>
#include <QVector2D>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geometryengine.h"

class GeometryEngine;

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    using QOpenGLWidget::QOpenGLWidget;
    ~MainWidget();
    MainWidget(MarchingCubes *mc) : mc(mc){};

   protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();

   private:
    const MarchingCubes *mc;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries = nullptr;

    QMatrix4x4 projection;

    QPointF prevMouse;
    qreal zNear = 0.1, zFar = 100.0, fov = 45.0;

    bool mouseLeftPressed = false, mouseRightPressed = false, mouseMiddlePressed = false;
    float curr_quat[4] = {0, 0, 0, 1};
    float prev_quat[4] = {0, 0, 0, 1};
    glm::vec3 eye = {0, 0, 3}, lookat = {0, 0, 0}, up = {0, 1, 0};
};

#endif  // MAINWIDGET_H