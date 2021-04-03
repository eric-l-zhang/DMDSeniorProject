#pragma once

#include "openglcontext.h"
#include "utils.h"
#include "shaderprogram.h"
#include "FileReader.h"
#include "Camera.h"
#include <scene/line.h>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


class MyGL
    : public OpenGLContext
{
private:
    ShaderProgram prog_flat;// A shader program that uses "flat" reflection (no shadowing at all)

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.

    // Knot stuff
    FileReader fileReader;
    uPtr<Rope> rope;
    Camera cam;
    glm::vec3 lightPosition, lightColor, eyePos;
    Node* currentNode;
    vector<Node*> nodeList;
    int numSpheres, index;
    glm::vec3 tempColor;
    clock_t old_time;
    float rotation;


    unsigned int vboPos;
    unsigned int vboCol;
    unsigned int vboNor;
    unsigned int vboIdx;


public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();


    void initKnotObjs();
    void traverse(Node* N, glm::mat4 T);
    void uploadGeometry(Geometry* geometry);

    Line line;

protected:
    void keyPressEvent(QKeyEvent *e);
};

