#include <GLFW/glfw3.h>
#include <iostream>

int gScreenWidth = 800;
int gScreenHeight = 600;
GLFWwindow * window = nullptr;

void initialization() {
    if (!glfwInit()) return;
    window = glfwCreateWindow(800, 600, "Title", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
}

void mainLoop() {
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

void cleanUp() {
    glfwTerminate();
    return;
}


int main()
{
    initialization();

    mainLoop();

    cleanUp();
}