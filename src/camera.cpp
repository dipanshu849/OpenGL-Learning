#include "camera.hpp"
#include "../glm/ext/matrix_transform.hpp"
#include <iostream>
#include <cmath>
/*
  Due to work done by GLM, I can just care about how my camera is moiving in space,
  Instead of thinking about the reality that the world is moving around camera and inverse transformations are being applied
*/

Camera::Camera(int width, int height)
{
  m_eye            = glm::vec3(0.0f, 0.0f, 0.0f);
  // NOTE:
  /*
    m_targetPosition acts like the inital looking postion for out camera
    But, when we are moving the camera the would be m_eye + m_targetPosition
  */
  m_targetPosition = glm::vec3(0.0f, 0.0f, -1.0f);
  m_upDirection    = glm::vec3(0.0f, 1.0f, 0.0f); 

  yaw = -90.0f;
  pitch = 0.0f;
  m_lastMousePosition.x = width / 2;
  m_lastMousePosition.y = height / 2;
}

glm::mat4 Camera::getViewMatrix()
{
  return glm::lookAt(m_eye, m_eye +  m_targetPosition, m_upDirection);
}

void Camera::moveForward(float speed)
{
  // m_targetPosition helps to get the direction of movement
  // as the direction is not constant when we rotation
  // hence can't always relay on x, y, z axis for movement
  m_eye += speed * m_targetPosition;
}

void Camera::moveBackward(float speed)
{
  m_eye -= speed * m_targetPosition;
}

void Camera::moveLeft(float speed)
{

}

void Camera::moveRight(float speed)
{

}

void Camera::mouseLook(float xPos, float yPos)
{
  // std::cout << "Mouse: " << xPos << ", " << yPos << std::endl;
  static bool firstMouse = true;

  if (firstMouse)
  {
    m_lastMousePosition.x = xPos;
    m_lastMousePosition.y = yPos;
    firstMouse = false;
  }

  float xOffset = xPos - m_lastMousePosition.x;
  float yOffset = m_lastMousePosition.y - yPos;
  m_lastMousePosition.x = xPos;
  m_lastMousePosition.y = yPos;

  float sensitiviy = 0.1f;
  xOffset *= sensitiviy;
  yOffset *= sensitiviy;

  yaw += xOffset;
  pitch += yOffset;

  if (pitch > 89.0f) pitch = 89.0f;
  else if(pitch < -89.0f) pitch = -89.0f;

  // std::cout << "Angle: " << yaw << ", " << pitch << std::endl;

  glm::vec3 direction;
  direction.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
  direction.y = std::sin(glm::radians(pitch));
  direction.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));

  m_targetPosition = glm::normalize(direction);
}
