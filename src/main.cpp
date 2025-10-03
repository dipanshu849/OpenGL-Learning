/*
    g++ src/*.cpp glad/glad.c -o prog -I./glad/ -lGL -lglfw -ldl
*/

// Third Party Libraries
#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"

// Standard Libraries 
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// My libraries
#include "camera.hpp"

int gScreenWidth = 800;
int gScreenHeight = 600;
GLFWwindow * window = nullptr;

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;

// Program object (for our shader)
GLuint gGraphicsPipelineShaderProgram = 0;

// Offset to move the triangle
GLfloat g_offset = 0;
GLfloat g_rotate = 0;
Camera g_camera(gScreenWidth, gScreenHeight);
GLfloat g_cameraSpeed = 5.0f;
GLfloat g_deltaTime = 0;
GLfloat g_lastFrame = glfwGetTime();

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
  // 
  if(action != GLFW_REPEAT && action != GLFW_PRESS) return; 
  
  float cameraSpeed = g_cameraSpeed * g_deltaTime;

  switch (key)
  {
    case GLFW_KEY_W:
      g_camera.moveForward(cameraSpeed);
      break;
  
    case GLFW_KEY_S:
      g_camera.moveBackward(cameraSpeed);
      break;

    case GLFW_KEY_D:
      g_camera.moveRight(cameraSpeed);
      break;

    case GLFW_KEY_A:
      g_camera.moveLeft(cameraSpeed);
      break;

    case GLFW_KEY_UP:
      g_camera.moveUp(cameraSpeed);
      break;

    case GLFW_KEY_DOWN:
      g_camera.moveDown(cameraSpeed);
      break;
  }
}


void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos)
{
  g_camera.mouseLook(xPos, yPos);
}


void initialization() 
{
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback); 
    glfwSetCursorPosCallback(window, cursorPosition_callback);
}


void vertexSpecification() 
{
    // Lives on the CPU
    std::vector<GLfloat> vertexData {
        -0.8f, -0.8f, 0.0f,
        1.0f, 0.0f, 0.0f, // color
        0.8f, -0.8f, 0.0f,
        0.0f, 1.0f, 0.0f, // color
        0.0f, 0.8f, 0.0f,
        0.0f, 0.0f, 1.0f  // color 
    };


    // we start setting things up
    // on the GPU
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    // start generating our VBO
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                vertexData.size() * sizeof(GLfloat),
                vertexData.data(),
                GL_STATIC_DRAW);


    // Linking the attrib in VAO
    // POSITION
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT) * 6, // IMPORTANT DEFN
                          (void*)0);

    // COLOR
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT) * 6,
                          (GLvoid*)(sizeof(GL_FLOAT) * 3));

    glBindVertexArray(0);
    glDisableVertexAttribArray(0); 
    glDisableVertexAttribArray(1);
}

GLuint CompileShader(GLuint type, const std::string& source) 
{
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

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) 
{
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

void createGraphicsPipeline() 
{
    std::string vertexShaderSource = loadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = loadShaderAsString("./shaders/frag.glsl");

    gGraphicsPipelineShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}


void Input()
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}


void PreDraw() 
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glViewport(0, 0, gScreenWidth, gScreenHeight);
  glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

  glUseProgram(gGraphicsPipelineShaderProgram);


  // Local to world
  GLint location = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_offset));
  model = glm::rotate(model, glm::radians(g_rotate), glm::vec3(0.0f, 1.0f, 0.0f));
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);


  // World to camera
  glm::mat4 cameraSpace = g_camera.getViewMatrix(); 
  location = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_view");
  glUniformMatrix4fv(location, 1, GL_FALSE, &cameraSpace[0][0]);    

  // Real screen view
  location    = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_projection");
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth/gScreenHeight, 0.1f, 10.0f);
  glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
}


void Draw() 
{
  glBindVertexArray(gVertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}


void mainLoop() 
{
  while (!glfwWindowShouldClose(window))
  {
    float currentTime = glfwGetTime();
    g_deltaTime = currentTime - g_lastFrame;
    g_lastFrame = currentTime;
  
    Input();

    PreDraw();

    Draw();

    // Update the screen
    glfwPollEvents(); 
    glfwSwapBuffers(window);
  }
}


void cleanUp() 
{
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

  return 0;
}
