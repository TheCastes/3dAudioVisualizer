#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Trackball {
public:
    Trackball() = default;

    void mouseDown(float x, float y, int viewportWidth, int viewportHeight);
    void mouseMove(float x, float y, int viewportWidth, int viewportHeight);
    void mouseUp();
    void scroll(float delta);
    void reset();

    glm::mat4 rotationMatrix() const;
    void applyRotation(float angleDegrees, const glm::vec3& axis);

    float getZoom() const { return zoom; }
private:
    glm::quat currentRotation = glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

    glm::vec3 lastPoint{};
    bool dragging = false;
    float zoom = 1.0f;

    static glm::vec3 projectOnSphere(float x, float y, int viewportWidth, int viewportHeight);
};