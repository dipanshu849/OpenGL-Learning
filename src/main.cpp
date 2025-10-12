    // color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
/*
    g++ src/*.cpp glad/glad.c -o prog -I./glad/ -lGL -lglfw -ldl
*/

// Third Party Libraries
#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include "../glm/ext/matrix_transform.hpp"
#include "../glm/ext/matrix_clip_space.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
  GLuint mPositionVertexBufferObject = 0;
  GLuint mUvVertexBufferObject = 0;
  GLuint mTextureObject = 0;
  
  // we can use glfoat or glm::vec3 direct
  std::vector<T> mVertexData; 
  std::vector<T> mUvData; // T can cause problem here

  GLfloat mOffset = 0;
  GLfloat mRotate = 0;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
App gApp;
// Mesh3D<glm::vec3> gMesh1;
Mesh3D<GLfloat> gMesh2;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GLOBALS END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



// ~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR HANDLING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static void GLClearAllErrors()
{
  while(glGetError() != GL_NO_ERROR);
}


static bool GLCheckErrorStatus(const char* function, int line)
{
  while(GLenum error = glGetError())
  {
    std::cout << "OpenGL Error: " << error
              << "\tLine: " << line
              << "\tfunction: " << function << std::endl;
    return true;
  }
  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

// ~~~~~~~~~~~~~~~~~~~~~~~~~ ERROR HANDLING END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


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


template <typename T>
bool meshCreate(const char* path, Mesh3D<T> *mesh)
{
  std::vector<float> vertexData;
  std::vector<float> uvData;
  std::vector<float> normalData;

  if(loadObj(path, vertexData, uvData, normalData) == false)
  {
    std::cout << "Problem occured in loading model" << std::endl;
    return false;
  }

  mesh->mVertexData = vertexData;
  mesh->mUvData = uvData;
  return true;
}


template <typename T>
bool loadTexture(const char* path, Mesh3D<T> *mesh)
{
  glGenTextures(1, &mesh->mTextureObject);
  glBindTexture(GL_TEXTURE_2D, mesh->mTextureObject);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  stbi_set_flip_vertically_on_load(true); // This line fixed a bug which was so annoying  

  int width, height, nChannels;
  unsigned char* data = stbi_load(path, &width, &height, &nChannels, 0);
  
  if(stbi_failure_reason())
  {
    std::cout << stbi_failure_reason() << std::endl;
  }
  
  if (data)
  {
    std::cout << "Width of texture: " << width << std::endl;
    std::cout << "Height of texture: " << height << std::endl;

    GLCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);)
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data);

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
  glGenBuffers(1, &mesh->mPositionVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionVertexBufferObject);
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

  glGenBuffers(1, &mesh->mUvVertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->mUvVertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER,
              mesh->mUvData.size() * sizeof(T),
              mesh->mUvData.data(),
              GL_STATIC_DRAW);


  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        false,
                        0,
                        (void*)0);

  glBindVertexArray(0);
  glDisableVertexAttribArray(0); 
  glDisableVertexAttribArray(1);
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
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

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
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mesh->mTextureObject);
  glBindVertexArray(mesh->mVertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, mesh->mVertexData.size() / 3);
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

  if(
     !meshCreate("Models/BenchTextured.obj", &gMesh2) || 
     !loadTexture("Models/textures/combinedBenchTexture.png", &gMesh2))
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
