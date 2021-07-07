#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "marching_cubes.h"

class GeometryEngine : protected QOpenGLFunctions {
   public:
    GeometryEngine(const MarchingCubes* mc);
    virtual ~GeometryEngine();

    void drawCubeGeometry(QOpenGLShaderProgram* program);

   private:
    void initCubeGeometry();

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    const MarchingCubes* mc;
};

#endif  // GEOMETRYENGINE_H
