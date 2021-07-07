#include "mainwidget.h"

#include <QMouseEvent>
#include <cmath>

#include "trackball.h"

MainWidget::~MainWidget() {
    makeCurrent();
    delete geometries;
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e) {
    // Save mouse press position
    prevMouse = e->localPos();
    if (e->button() == Qt::MouseButton::LeftButton) {
        mouseLeftPressed = true;
        trackball(prev_quat, 0.0, 0.0, 0.0, 0.0);
    } else if (e->button() == Qt::MouseButton::RightButton) {
        mouseRightPressed = true;
    } else if (e->button() == Qt::MouseButton::MiddleButton) {
        mouseMiddlePressed = true;
    }
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::MouseButton::LeftButton) {
        mouseLeftPressed = false;
    } else if (e->button() == Qt::MouseButton::RightButton) {
        mouseRightPressed = false;
    } else if (e->button() == Qt::MouseButton::MiddleButton) {
        mouseMiddlePressed = false;
    }
}
void MainWidget::mouseMoveEvent(QMouseEvent *e) {
    float rotScale = 1.0f;
    float transScale = 2.0f;

    auto mouse = e->localPos();
    // 左键旋转
    if (mouseLeftPressed) {
        trackball(
            prev_quat,
            rotScale * (2.0f * prevMouse.x() - width()) / (float)width(),
            rotScale * (height() - 2.0f * prevMouse.y()) / (float)height(),
            rotScale * (2.0f * mouse.x() - width()) / (float)width(),
            rotScale * (height() - 2.0f * mouse.y()) / (float)height());

        add_quats(prev_quat, curr_quat, curr_quat);
    }
    // 右键缩放
    else if (mouseRightPressed) {
        eye[0] -= transScale * (mouse.x() - prevMouse.x()) / (float)width();
        lookat[0] -= transScale * (mouse.x() - prevMouse.x()) / (float)width();
        eye[1] += transScale * (mouse.y() - prevMouse.y()) / (float)height();
        lookat[1] += transScale * (mouse.y() - prevMouse.y()) / (float)height();
    }
    // 中间键移动
    else if (mouseMiddlePressed) {
        eye[2] += transScale * (mouse.y() - prevMouse.y()) / (float)height();
        lookat[2] += transScale * (mouse.y() - prevMouse.y()) / (float)height();
    }
    prevMouse = mouse;
    // Request an update
    update();
}

void MainWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();

    //! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glDisable(GL_CULL_FACE);
    //! [2]

    geometries = new GeometryEngine(mc);
}

//! [3]
void MainWidget::initShaders() {
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/3.1.materials.vs"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/3.1.materials.fs"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();

    glm::vec3 lightColor = {1, 1, 1};
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);    // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);  // low influence

    program.setUniformValue("light.position", 1.2f, 1.0f, 2.0f);
    program.setUniformValue("light.ambient", 0.2f, 0.2f, 0.2f);
    program.setUniformValue("light.diffuse", 0.5f, 0.5f, 0.5f);
    program.setUniformValue("light.specular", 1.0f, 1.0f, 1.0f);
    // material properties
    program.setUniformValue("material.ambient", 1.0f, 0.5f, 0.31f);
    program.setUniformValue("material.diffuse", 1.0f, 0.5f, 0.31f);
    program.setUniformValue("material.specular", 0.5f, 0.5f, 0.5f);  // specular lighting doesn't have full effect on this object's material
    program.setUniformValue("material.shininess", 32.0f);
}
//! [3]

//! [5]
void MainWidget::resizeGL(int w, int h) {
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MainWidget::paintGL() {
    if (!geometries) return;

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //! [6]
    // Calculate model view transformation
    QMatrix4x4 model;

    // 对相机进行旋转，等价于对 model 进行逆旋转
    model.rotate(QQuaternion(curr_quat[3], -curr_quat[0], -curr_quat[1], -curr_quat[2]));

    model.scale(1.0 / mc->maxExtent);
    model.translate(
        -0.5 * (mc->bmax[0] + mc->bmin[0]),
        -0.5 * (mc->bmax[1] + mc->bmin[1]),
        -0.5 * (mc->bmax[2] + mc->bmin[2]));

    auto view = glm::lookAt(eye, lookat, up);
    // 可能内部数据存储的方式不一样，所以需要转置一下
    // https://stackoverflow.com/a/37588210/8242705
    // glm 内部存储是 column major 的，但是 qt 接受的是 row major 的数据
    // https://community.khronos.org/t/while-matrices-are-in-row-major-or-column-major-im-in-confunsion-major/74135/5
    auto p = glm::value_ptr(view);
    QMatrix4x4 view_m = QMatrix4x4(p).transposed();

    // Set modelview-projection matrix
    program.setUniformValue("model", model);
    program.setUniformValue("view", view_m);
    program.setUniformValue("projection", projection);
    program.setUniformValue("mvp_matrix", projection * model);
    program.setUniformValue("viewPos", eye[0], eye[1], eye[2]);

    //! [6]

    // Draw cube geometry
    geometries->drawCubeGeometry(&program);
}
