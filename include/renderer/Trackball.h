#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Trackball {
public:
    Trackball() = default;

    void mouseDown(float x, float y, int viewportWidth, int viewportHeight);
    void mouseMove(float x, float y, int viewportWidth, int viewportHeight);
    void mouseUp();

    glm::mat4 rotationMatrix() const;
    void applyRotation(float angleDegrees, const glm::vec3& axis);
private:
    glm::quat currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    glm::vec3 lastPoint{};
    bool dragging = false;

    static glm::vec3 projectOnSphere(float x, float y, int viewportWidth, int viewportHeight);
};