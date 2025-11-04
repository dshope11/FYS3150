//
// Created by Nils Enric Canut Taugbøl on 07/01/2025.
//

#include "precomp.h"
#include "../../include/aos/actor.h"
#include "box.h"

// Hole //

Hole::Hole(const arma::vec2& pos, const double mass, const float radius) {
    this->pos = pos;
    this->mass = mass;
    this->radius = radius;
    vel = {0., 0.};

    // preparing the sprite for drawing
    shape = std::make_unique<sf::CircleShape>(radius);
    shape->setOrigin({radius, radius});
    // SFML works with their own types which are floats while armadillo works with doubles
    shape->setPosition(sf::Vector2f{(float)(pos(0)), (float)(pos(1))});
    shape->setFillColor(sf::Color::Transparent);
    shape->setOutlineColor(sf::Color::Cyan);
    shape->setOutlineThickness(1.);
}

Hole::Hole(const arma::vec2& pos, const arma::vec2& vel, const double mass, const float radius) {
    this->pos = pos;
    this->mass = mass;
    this->radius = radius;
    this->vel = vel;

    // preparing the sprite for drawing
    shape = std::make_unique<sf::CircleShape>(radius);
    shape->setOrigin({radius, radius});
    shape->setPosition(sf::Vector2f{(float)(pos(0)), (float)(pos(1))});
    shape->setFillColor(sf::Color::Transparent);
    shape->setOutlineColor(sf::Color::Cyan);
    shape->setOutlineThickness(1.);
}

bool Hole::update(double dt) {
    // nothing to update here yet, they stay put
    return true;
}


// Particle //

Particle::Particle(const arma::vec2& pos, const arma::vec2& vel, double mass) {
    this->pos = pos;
    this->vel = vel;
    this->mass = mass;

    float radius = part_radius;
    // preparing the sprite for drawing
    shape = std::make_unique<sf::CircleShape>(radius);
    shape->setOrigin({radius, radius});
    // SFML works with their own types which are floats while armadillo works with doubles
    shape->setPosition(sf::Vector2f{(float)(pos(0)), (float)(pos(1))});
    shape->setFillColor(sf::Color::Yellow);
}

bool Particle::update(double dt) {
    // update position using Heun's method
    arma::vec2 accelerationPre = acceleration();
    arma::vec2 accelerationPost = acceleration(pos + dt * vel);
    pos += dt/2. * ( vel + vel + dt*accelerationPre );
    vel = vel + dt/2. * (accelerationPre + accelerationPost);

    // if the particle is outside of the box wrap it back in on the opposite side
    // TODO: add WRAP_PARTICLES as a possible definition for cmake in the README
#if WRAP_PARTICLES == 1
    for (int i = 0; i < pos.n_elem; i++) {
        if (pos[i] < Box::lowerBounds[i]) {
            pos(i) = Box::upperBounds[i] - fmod(Box::lowerBounds[i] - pos[i], Box::upperBounds[i] - Box::lowerBounds[i]);
        }
        else if (pos[i] >= Box::upperBounds[i]) {
            pos[i] = Box::lowerBounds[i] + fmod(pos[i] - Box::lowerBounds[i], Box::upperBounds[i] - Box::lowerBounds[i]);
        }
    }
#else
    if (any(pos < Box::lowerBounds) || any(pos >= Box::upperBounds)) {
        return false;
    }
#endif

    // if the particle is in a hole -> delete and generate new particle with new conditions
    for (const auto& hole : Box::holes) {
        double dist = arma::norm(hole->pos - pos);
        if (dist <= hole->radius) {
            return false;
        }
    }

    // we update to the new position on the screen
    shape->setPosition(sf::Vector2f{(float) pos(0), (float) pos(1)});

    // pos now holds the old position which will be used by the other actors to update their position
    return true;
}

arma::vec2 Particle::acceleration(const arma::vec2& positionOverride) {
    arma::vec2 position = positionOverride.is_empty() ? pos : positionOverride;

    arma::vec2 acceleration = {0., 0.};

    // find the force from the holes and other particles onto the particle
    for (const auto& actor : Box::actorPool) {
        arma::vec2 direction = actor->pos - position;
        double dist = norm(direction);
        arma::vec2 total_force = G * actor->mass * mass / std::pow(dist, 2) * normalise(direction);
        acceleration += total_force /  mass;
    }

    return acceleration;
}



