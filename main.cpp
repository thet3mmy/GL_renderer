/*
    OpenGL Renderer
    Tommy Wilson (github thet3mmy)

    November 19 2021
    Based on code snippets from previous projects
    Some code sourced from https://www.opengl-tutorial.org
*/

#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "util/shader.cpp"
#include "util/controls.cpp"
#include "util/texture.cpp"
#include "util/objloader.cpp"

/*
*/
GLFWwindow *window;
GLuint vertexarrayid;
GLuint vertexbuffer;
GLuint uvtable;
GLuint programid;
GLuint matrixid;
/*
*/

namespace RenderSystem
{
    class Matrix
    {
    public:
        glm::mat4 Model;
        glm::mat4 View;
        glm::mat4 Projection;
        glm::mat4 MVP;
    };

    class RenderObject
    {
    public:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
    };

    int init(void)
    {
        glewExperimental = true;
        if (!glfwInit())
        {
            return -1;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1024, 768, "RendererSystem", NULL, NULL);
        if (window == NULL)
        {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK)
        {
            return -1;
        }
        return 0;
    }

    void prepareBuffers(RenderSystem::RenderObject *r)
    {
        glGenVertexArrays(1, &vertexarrayid);
        glBindVertexArray(vertexarrayid);

        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     r->vertices.size() * sizeof(glm::vec3),
                     &r->vertices[0],
                     GL_STATIC_DRAW);

        glGenBuffers(2, &uvtable);
        glBindBuffer(GL_ARRAY_BUFFER, uvtable);
        glBufferData(GL_ARRAY_BUFFER,
                     r->uvs.size() * sizeof(glm::vec2),
                     &r->uvs[0],
                     GL_STATIC_DRAW);
    }
}

/*
*/
RenderSystem::Matrix matrix;
std::vector<RenderSystem::RenderObject> render;
/*
*/

int main(int argc, char *argv[])
{
    int init_status = RenderSystem::init();
    if (init_status == -1)
        return -1;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    programid = LoadShaders("shader/TransformTexVertex.glsl", "shader/TransformTexFragment.glsl");
    matrixid = glGetUniformLocation(programid, "MVP");

    glClearColor(0.25f, 0.35f, 0.8f, 0.9f);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    RenderSystem::RenderObject myr;
    loadOBJ("models/ico.obj", myr.vertices, myr.uvs, myr.normals);
    render.push_back(myr);

    RenderSystem::RenderObject myr2;
    loadOBJ("models/cube2.obj", myr2.vertices, myr2.uvs, myr2.normals);
    render.push_back(myr2);

    GLuint tex = loadBMP_custom("graphics/rock.bmp");

    while (1 > 0)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programid);

        // run inputs
        computeMatricesFromInputs(window, false);
        matrix.Projection = getProjectionMatrix();
        matrix.View = getViewMatrix();
        matrix.Model = glm::mat4(1.0f);
        matrix.MVP = matrix.Projection * matrix.View * matrix.Model;

        // send MVP to shader
        glUniformMatrix4fv(matrixid, 1, GL_FALSE, &matrix.MVP[0][0]);

        for (int drawObj = 0; drawObj < render.size(); drawObj++)
        {
            // load buffer data
            RenderSystem::prepareBuffers(&render[drawObj]);

            // draw
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvtable);
            glVertexAttribPointer(
                1,
                2,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void *)0);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            glDrawArrays(GL_TRIANGLES, 0, render[drawObj].vertices.size());
            glTranslatef(10, 10, 10);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}