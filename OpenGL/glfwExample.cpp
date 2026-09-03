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
        "My Modified GLFW Example",
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

    GLint major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        /*
         * Make the background color change over time.
         * sin() produces smooth values between -1 and 1,
         * so (sin(...) + 1) / 2 converts them to 0 through 1.
         */
        float red = static_cast<float>((std::sin(currentTime) + 1.0) / 2.0);
        float green = static_cast<float>((std::sin(currentTime + 2.0) + 1.0) / 2.0);
        float blue = static_cast<float>((std::sin(currentTime + 4.0) + 1.0) / 2.0);

        glClearColor(red, green, blue, 1.0f);

        /* Clear the screen */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Render your objects here */

        /* Swap the front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        /* Press T to display FPS */
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            static double lastPrintTime = 0.0;

            if (currentTime - lastPrintTime > 0.25) {
                std::cout << "Time: " << currentTime << " seconds" << std::endl;
                lastPrintTime = currentTime;
            }
        }

        /* Press Escape to close */
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glfwTerminate();
    return 0;
}
