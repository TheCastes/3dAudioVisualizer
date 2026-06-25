#define GLM_ENABLE_EXPERIMENTAL

#include "renderer/Trackball.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <cmath>


glm::vec3 Trackball::projectOnSphere(const float x, const float y, const int viewportWidth, const int viewportHeight) {
    // from  pixel to NDC [-1, 1]
    glm::vec3 p(
         (2.0f * x / static_cast<float>(viewportWidth))  - 1.0f,
        -(2.0f * y / static_cast<float>(viewportHeight)) + 1.0f,
        0.0f
    );

    float distanceFromOrigin = glm::length(glm::vec2(p));
    distanceFromOrigin = std::min(distanceFromOrigin, 1.0f); // clamp
    p.z = std::sqrt(1.0f - distanceFromOrigin * distanceFromOrigin); //project on front
    return glm::normalize(p);
}

void Trackball::mouseDown(const float x, const float y, const int viewportWidth, const int viewportHeight) {
    lastPoint = projectOnSphere(x, y, viewportWidth, viewportHeight);
    dragging = true;
}

void Trackball::mouseMove(const float x, const float y, const int viewportWidth, const int viewportHeight) {
    if (!dragging) {
        return;
    }

    const glm::vec3 currentPoint = projectOnSphere(x, y, viewportWidth, viewportHeight);
    const glm::vec3 rotationAxis = glm::cross(lastPoint, currentPoint);
    const float angle = std::asin(std::min(glm::length(rotationAxis), 1.0f));

    if (glm::length(rotationAxis) > 1e-6f) {
        const glm::quat drag = glm::angleAxis(angle, glm::normalize(rotationAxis));
        currentRotation = glm::normalize(drag * currentRotation);
    }
    lastPoint = currentPoint;
}

void Trackball::mouseUp() {
    dragging = false;
}

glm::mat4 Trackball::getRotationMatrix() const {
    return glm::toMat4(currentRotation);
}

void Trackball::applyRotation(const float angleDegrees, const glm::vec3& axis) {
    const glm::quat r = glm::angleAxis(glm::radians(angleDegrees), axis);
    currentRotation = glm::normalize(r * currentRotation);
}

void Trackball::scroll(const float delta) {
    zoom *= std::pow(0.9f, delta);
    zoom = std::clamp(zoom, 0.1f, 10.0f);
}

void Trackball::reset() {
    currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    zoom = 1.0f;
}