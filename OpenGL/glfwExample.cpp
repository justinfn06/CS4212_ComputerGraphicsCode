#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLSL.h"

int CheckGLErrors(const char *s)
{
    int errCount = 0;
    return errCount;
}

int main(void)
{
    /* Initialize the library */
    if (!glfwInit()) {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    int winWidth = 1000;
    float aspectRatio = 1.0;
    int winHeight = winWidth / aspectRatio;

    GLFWwindow* window = glfwCreateWindow(
        winWidth,
        winHeight,
        "GLFW Example",
        NULL,
        NULL
    );

    if (!window) {
        std::cerr << "GLFW did not create a window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    

    if (err != GLEW_OK) {
        std::cerr << "GLEW Error! glewInit failed, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    GLuint m_triangleVBO[1];

    glGenBuffers(1, m_triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);

    std::vector<float> host_VertexBuffer{
        // Position               // normal
        -3.0f, -3.0f, 0.0f,        0.0f, 0.0f, 1.0f,
         3.0f, -3.0f, 0.0f,        0.0f, 0.0f,  1.0f,
         0.0f,  3.0f, 0.0f,        0.0f, 0.0f,  1.0f
    };

    int numBytes = host_VertexBuffer.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, numBytes, host_VertexBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint m_VAO;
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    sivelab::GLSLObject shader;

    shader.addShader("vertexShader_PrepForPerFragment.glsl", sivelab::GLSLObject::VERTEX_SHADER);

    shader.addShader("fragmentShader_BlinnPhong.glsl", sivelab::GLSLObject::FRAGMENT_SHADER);

    if (shader.createProgram() == 0) {
        std::cerr << "Shader program linking failed.\n";
        return -1;
    }

    GLint projMatrixID = shader.createUniform("projMatrix");
    GLint viewMatrixID = shader.createUniform("viewMatrix");
    GLint modelMatrixID = shader.createUniform("modelMatrix");
    GLint normalMatrixID = shader.createUniform("normalMatrix");
    GLint lightPosWorldID = shader.createUniform("lightPosWorld");
    GLint diffuseComponentID = shader.createUniform("diffuseComponent");
    GLint specularComponentID = shader.createUniform("specularComponent");
    GLint phongExponentID = shader.createUniform("phongExponent");
    GLint viewDirWorldID = shader.createUniform("viewDirWorld");
    shader.createUniform("viewDirWorld");



    // orthographic
    float halfWidth = 15.0 / 2.0;
    float halfHeight = halfWidth;

    float left = -halfWidth;
    float right = halfWidth;

    float bottom = -halfHeight;
    float top = halfHeight;

    float near = 0.1f;
    float far = 30.0f;

    glm::mat4 M_ortho = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);

    glm::vec3 m_pos(0,0,10), m_viewDir(0,0,-1);
    glm::vec3 m_U(1,0,0), m_V(0,1,0), m_W(0,0,1);

    double timeDiff = 0.0, startFrameTime = 0.0, endFrameTime = 0.0;
    

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        startFrameTime = glfwGetTime();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 M_view = glm::lookAt(m_pos, m_pos - m_W, m_V);

        shader.activate();

        glm::mat4 M_model(1.0f);
        glm::mat4 M_normal = glm::transpose(glm::inverse(M_model));

        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(M_ortho));

        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(M_view));

        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(M_model));

        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(M_normal));

        glUniform4f(lightPosWorldID, 0.0f, 0.0f, 2.0f, 1.0f);
        glUniform3f(diffuseComponentID, 0.5f, 0.15f, 0.05f);
        glUniform3f(specularComponentID, 0.7f, 0.7f, 0.7f);
        glUniform1f(phongExponentID, 32.0f);
        glUniform4f(viewDirWorldID,m_W.x, m_W.y, m_W.z, 0.0f);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        shader.deactivate();

        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        float moveRatePerFrame = 0.005f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            m_pos = m_pos + -m_W * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            m_pos = m_pos - m_U * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            m_pos = m_pos + m_W * moveRatePerFrame;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            m_pos = m_pos + m_U * moveRatePerFrame;
        }

        /* Press T to display FPS */
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            std::cout << "fps: " << 1.0 / timeDiff << std::endl;
        }

        /* Press Escape to close */
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
        endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime;
    }

    glfwTerminate();
    return 0;
}
