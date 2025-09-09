#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

int gScreenWidth = 800;
int gScreenHeight = 600;
GLFWwindow * window = nullptr;

// VAOname
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;

// Program object (for our shader)
GLuint gGraphicsPipelineShaderProgram = 0;

std::string loadShaderAsString(const std::string& filename) {
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()) {
        while (std::getline(myFile, line)) {
            result += line + '\n';
        }
        myFile.close();
    }

    return result;
}

void initialization() {
    if (!glfwInit()) return;
    window = glfwCreateWindow(800, 600, "Title", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
}


void vertexSpecification() {
    // Lives on the CPU
    std::vector<GLfloat> vertexPosition {
        -0.8f, -0.8f, 0.0f,
        0.8f, -0.8f, 0.0f,
        0.0f, 0.8f, 0.0f
    };

    // we start setting things up
    // on the GPU
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    // start generating our VBO
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                vertexPosition.size() * sizeof(GLfloat),
                vertexPosition.data(),
                GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0);
    glBindVertexArray(0);
    glDisableVertexAttribArray(0); 
}

GLuint CompileShader(GLuint type, const std::string& source) {
    GLuint shaderObject;

    if (type == GL_VERTEX_SHADER) {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    return programObject;
}

void createGraphicsPipeline() {
    std::string vertexShaderSource = loadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = loadShaderAsString("./shaders/frag.glsl");

    gGraphicsPipelineShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void PreDraw() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw() {

    glBindVertexArray(gVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void mainLoop() {
    while (!glfwWindowShouldClose(window))
    {
        // Handle Input
        glfwPollEvents(); 

        PreDraw();

        Draw();

        // Update the screen
        glfwSwapBuffers(window);
    }
}

void cleanUp() {
    glfwTerminate();
    return;
}


int main()
{
    initialization();

    vertexSpecification();

    createGraphicsPipeline();

    mainLoop();

    cleanUp();
}