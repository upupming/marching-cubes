#include "geometryengine.h"

// https://www.codenong.com/cs106436180/
static void GLClearError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}
static void GLCheckError() {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL Error(" << error << ")" << std::endl;
    }
}

//! [0]
GeometryEngine::GeometryEngine(const MarchingCubes* mc) : indexBuf(QOpenGLBuffer::IndexBuffer), mc(mc) {
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    initCubeGeometry();
}

GeometryEngine::~GeometryEngine() {
    arrayBuf.destroy();
}
//! [0]

void GeometryEngine::initCubeGeometry() {
    //! [1]
    // Transfer vertex data to VBO 0
    if (!arrayBuf.bind()) {
        std::cout << "arrayBuf bind failed" << std::endl;
    }
    arrayBuf.allocate(mc->getVertices().data(), mc->getVertices().size() * sizeof(Vertex));
    if (!indexBuf.bind()) {
        std::cout << "indexBuf bind failed" << std::endl;
    }
    indexBuf.allocate(mc->getTriangles().data(), mc->getTriangles().size() * sizeof(int) * 3);
    //! [1]
}

//! [2]
void GeometryEngine::drawCubeGeometry(QOpenGLShaderProgram* program) {
    // Tell OpenGL which VBOs to use
    if (!arrayBuf.bind()) {
        std::cout << "arrayBuf bind failed" << std::endl;
    }
    if (!indexBuf.bind()) {
        std::cout << "indexBuf bind failed" << std::endl;
    }

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(Vertex));

    // Tell OpenGL programmable pipeline how to locate vertex normal
    int normalLocation = program->attributeLocation("a_normal");
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, sizeof(Vertex) / 2, 3, sizeof(Vertex));

    // glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    GLCheckError();
    // type: Must be one of GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT.
    glDrawElements(GL_TRIANGLES, mc->getTriangles().size() * 3, GL_UNSIGNED_INT, nullptr);
    GLCheckError();
}
//! [2]
