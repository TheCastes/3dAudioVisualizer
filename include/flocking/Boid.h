#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "BoidConfig.h"

struct Boid;

// behaviors
class BoidAlign {
public:
    float weight = 1.0f;
    glm::vec3 GetDirection(const Boid& self, const std::vector<Boid*>& neighbors);
};

class BoidCohesion {
public:
    float weight = 1.0f;
    glm::vec3 GetDirection(const Boid& self, const std::vector<Boid*>& neighbors);
};

class BoidSeparation {
public:
    float weight = 1.0f;
    glm::vec3 GetDirection(const Boid& self, const std::vector<Boid*>& neighbors);
};


// actual class
struct Boid {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);

    BoidAlign align;
    BoidCohesion cohesion;
    BoidSeparation separation;

    void Update(const BoidConfig& config, float deltaTime, const std::vector<Boid*>& allBoids) {
        std::vector<Boid*> neighbors;
        
        // TODO optimize (no need to check all boids)
        for (Boid* other : allBoids) {
            if (other == this) continue;
            float dist = glm::length(other->position - position);
            if (dist <= config.fov) {
                neighbors.push_back(other);
            }
        }

        glm::vec3 globalDirection = glm::vec3(0.0f);
        globalDirection += align.GetDirection(*this, neighbors);
        globalDirection += cohesion.GetDirection(*this, neighbors);
        globalDirection += separation.GetDirection(*this, neighbors);

        if (globalDirection != glm::vec3(0.0f)) {
            glm::vec3 blended = glm::normalize(globalDirection + direction);
            direction = blended;
        }

        position += direction * config.speed * deltaTime;
    }
};


// behaviors inline implementation
// avoids function call overhead
// single TU, can optimize full chain 
inline glm::vec3 BoidAlign::GetDirection(const Boid& self, const std::vector<Boid*>& neighbors) {
    glm::vec3 alignment = glm::vec3(0.0f);
    for (const Boid* neighbor : neighbors) {
        alignment += neighbor->direction;
    }
    if (glm::length(alignment) > 0.0f) {
        alignment = glm::normalize(alignment);
    }
    return alignment * weight;
}

inline glm::vec3 BoidCohesion::GetDirection(const Boid& self, const std::vector<Boid*>& neighbors) {
    glm::vec3 cohesion = glm::vec3(0.0f);
    float counter = 0.0f;
    for (const Boid* neighbor : neighbors) {
        cohesion += neighbor->position;
        counter += 1.0f;
    }
    if (counter > 0.0f) {
        cohesion /= counter;
    }
    cohesion -= self.position;
    if (glm::length(cohesion) > 0.0f) {
        cohesion = glm::normalize(cohesion);
    }
    return cohesion * weight;
}

inline glm::vec3 BoidSeparation::GetDirection(const Boid& self, const std::vector<Boid*>& neighbors) {
    glm::vec3 separation = glm::vec3(0.0f);
    for (const Boid* neighbor : neighbors) {
        glm::vec3 tmp = self.position - neighbor->position;
        float mag = glm::length(tmp);
        if (mag > 0.0f) {
            separation += glm::normalize(tmp) / (mag + 0.0001f);
        }
    }
    if (glm::length(separation) > 0.0f) {
        separation = glm::normalize(separation);
    }
    return separation * weight;
}

/*
    explanation (TODO remove)

    Flocking::update() 
        → BoidSpawner::Update()
            → Boid::Update()            [called 10,000 times per frame]
            → align.GetDirection()      [called 10,000 × neighbors times]
            → cohesion.GetDirection()   [called 10,000 × neighbors times]
            → separation.GetDirection() [called 10,000 × neighbors times]

    When the methods are inline in the header, every TU that includes the header sees the full function body. The compiler can then:
    - Inline the call - replaces it with the actual code, zero call overhead
    - Optimize across boundaries - see the full picture of Update() → GetDirection() → neighbor loop as one unit
    - Auto-vectorize - with full visibility, the compiler can optimize the inner loops more aggressively

    everything is called from boidspawner, that TU sees everything through boid.h
*/
