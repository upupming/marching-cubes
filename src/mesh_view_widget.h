#ifndef MeshViewWidget_H
#define MeshViewWidget_H
#include <QBasicTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QQuaternion>
#include <QVector2D>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "marching_cubes.h"

class MeshViewWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

   public:
    using QOpenGLWidget::QOpenGLWidget;
    ~MeshViewWidget();
    MeshViewWidget(MarchingCubes *mc = nullptr);
    void setMarchingCubes(MarchingCubes *mc);

   protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();

   private:
    MarchingCubes *mc = nullptr;
    QOpenGLShaderProgram program;

    QMatrix4x4 projection;

    QPointF prevMouse;
    qreal zNear = 0.1, zFar = 100.0, fov = 45.0;

    bool mouseLeftPressed = false, mouseRightPressed = false, mouseMiddlePressed = false;
    float curr_quat[4] = {0, 0, 0, 1};
    float prev_quat[4] = {0, 0, 0, 1};
    // model 放在原点并且缩放到 [0, 1] 区间，我们从 z=3 往 -z 方向看，确保能够看到 model 整体全貌，不管它有多大
    glm::vec3 eye = {0, 0, 3}, lookat = {0, 0, 0}, up = {0, 1, 0};

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};

#endif  // MeshViewWidget_H
