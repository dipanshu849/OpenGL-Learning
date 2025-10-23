#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include "../glm/ext/matrix_transform.hpp"

class Camera
{
  private:
    glm::vec3 m_eye;
    glm::vec3 m_targetPosition;
    glm::vec3 m_upDirection;
    glm::vec2 m_lastMousePosition;
    float yaw;
    float pitch;

  public:
    Camera();
    glm::mat4 getViewMatrix();
    glm::vec3 getViewPos();
    void moveForward(float);
    void moveBackward(float);
    void moveLeft(float);
    void moveRight(float);
    void moveUp(float);
    void moveDown(float);
    void mouseLook(float, float);
};
#endif
