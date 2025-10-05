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
#include "loadModel.hpp"

struct App
{
  int mScreenWidth = 800;
  int mScreenHeight = 600;
  
  const char* mTitle = "CL-3";

  GLFWwindow * mWindow = nullptr;
  GLuint mGraphicsPipelineShaderProgram = 0;

  Camera mCamera;
  GLfloat mCameraSpeed = 5.0f;

  GLfloat mDeltaTime = 0;
  GLfloat mLastFrame = glfwGetTime();
};


template <typename T>
struct Mesh3D
{
  GLuint mVertexArrayObject = 0;
  GLuint mVertexBufferObject = 0;  
  
  // we can use glfoat or glm::vec3 direct
  std::vector<T> mVertexData;

  GLfloat mOffset = 0;
  GLfloat mRotate = 0;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
App gApp;
Mesh3D<glm::vec3> gMesh1;
Mesh3D<GLfloat> gMesh2;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// Handle Keyboard inputs
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
  if(action != GLFW_REPEAT && action != GLFW_PRESS) return; 
  
  float cameraSpeed = gApp.mCameraSpeed * gApp.mDeltaTime;

  switch (key)
  {
    case GLFW_KEY_W:
      gApp.mCamera.moveForward(cameraSpeed);
      break;
  
    case GLFW_KEY_S:
      gApp.mCamera.moveBackward(cameraSpeed);
      break;

    case GLFW_KEY_D:
      gApp.mCamera.moveRight(cameraSpeed);
      break;

    case GLFW_KEY_A:
      gApp.mCamera.moveLeft(cameraSpeed);
      break;

    case GLFW_KEY_UP:
      gApp.mCamera.moveUp(cameraSpeed);
      break;

    case GLFW_KEY_DOWN:
      gApp.mCamera.moveDown(cameraSpeed);
      break;
  }
}


// Handle mouse inputs
void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos)
{
  gApp.mCamera.mouseLook(xPos, yPos);
}


// Get things ready
void initialization(App* app) 
{ 
    if (!glfwInit()) return;
    app->mWindow = glfwCreateWindow(app->mScreenWidth, app->mScreenHeight, app->mTitle, NULL, NULL);
    if (!app->mWindow)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(app->mWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glfwSetKeyCallback(app->mWindow, key_callback); 
    glfwSetInputMode(app->mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(app->mWindow, cursorPosition_callback);
}


bool meshCreate()
{

  // Lives on the CPU
  //std::vector<GLfloat> vertexData {
  //    -0.8f, -0.8f, 0.0f,
  //    1.0f, 0.0f, 0.0f, // color
  //    0.8f, -0.8f, 0.0f,
  //    0.0f, 1.0f, 0.0f, // color
  //    0.0f, 0.8f, 0.0f,
  //    0.0f, 0.0f, 1.0f  // color 
  // };
  std::vector<float> vertexData;
  std::vector<glm::vec2> uvData;
  std::vector<glm::vec3> normalData;

  if(loadObj("Models/Bench.obj", vertexData, uvData, normalData) == false)
  {
    return false;
  }

  gMesh2.mVertexData = vertexData;
  return true;
}


// Sets up mesh data from CPU to GPU 
template <typename T>
void meshCTGdataTransfer(Mesh3D<T>* mesh) 
{
    // we start setting things up
    // on the GPU
    glGenVertexArrays(1, &mesh->mVertexArrayObject);
    glBindVertexArray(mesh->mVertexArrayObject);

    // start generating our VBO
    glGenBuffers(1, &mesh->mVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                mesh->mVertexData.size() * sizeof(T),
                mesh->mVertexData.data(),
                GL_STATIC_DRAW);


    // Linking the attrib in VAO
    // POSITION
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 
                          3,
                          GL_FLOAT,
                          false,
                          0, 
                          (void*)0);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0); 
}


// ~~~~~~~~~~~~~~~~~~ Graphics Pipline Setup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


// Converts GLSL files to strings
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


void createGraphicsPipeline(App* app) 
{
    std::string vertexShaderSource = loadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource = loadShaderAsString("./shaders/frag.glsl");

    app->mGraphicsPipelineShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
}
// ~~~~~~~~~~~~~~~~~~ Graphics Pipline Setup END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Input(App* app)
{
  if(glfwGetKey(app->mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(app->mWindow, true);
}


template <typename T>
void PreDraw(App* app, Mesh3D<T>* mesh) 
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glViewport(0, 0, app->mScreenWidth, app->mScreenHeight);
  glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

  glUseProgram(app->mGraphicsPipelineShaderProgram);


  // Local to world
  GLint location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, mesh->mOffset));
  model = glm::rotate(model, glm::radians(mesh->mRotate), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
  glUniformMatrix4fv(location, 1, GL_FALSE, &model[0][0]);


  // World to camera
  glm::mat4 cameraSpace = app->mCamera.getViewMatrix(); 
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_view");
  glUniformMatrix4fv(location, 1, GL_FALSE, &cameraSpace[0][0]);    

  // Real screen view
  location = glGetUniformLocation(app->mGraphicsPipelineShaderProgram, "u_projection");
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)app->mScreenWidth/app->mScreenHeight, 0.1f, 10.0f);
  glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
}


template <typename T>
void Draw(Mesh3D<T>* mesh) 
{
  glBindVertexArray(mesh->mVertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, mesh->mVertexData.size());
}


template <typename T>
void mainLoop(App* app, Mesh3D<T>* mesh) 
{
  while (!glfwWindowShouldClose(app->mWindow))
  {
    float currentTime = glfwGetTime();
    app->mDeltaTime = currentTime - app->mLastFrame;
    app->mLastFrame = currentTime;
  
    Input(app);

    PreDraw(app, mesh);

    Draw(mesh);

    // Update the screen
    glfwPollEvents(); 
    glfwSwapBuffers(app->mWindow);
  }
}


void cleanUp() 
{
  glfwTerminate();
  return;
}


int main()
{
  initialization(&gApp);

  if(!meshCreate())
  {
    cleanUp();
    return 0;
  };

  meshCTGdataTransfer(&gMesh2);
  createGraphicsPipeline(&gApp);

  mainLoop(&gApp, &gMesh2);
  cleanUp();

  return 0;
}
