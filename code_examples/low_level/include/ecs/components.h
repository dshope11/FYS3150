//
// Created by Nils Enric Canut Taugbøl on 03/11/2025.
//

#ifndef BOXED_PARTICLES_COMPONENTS_H
#define BOXED_PARTICLES_COMPONENTS_H

#pragma once
#include "precomp.h"

struct Position {arma::vec2 value; };
struct Velocity {arma::vec2 value; };
struct Mass {double value; };
struct Radius {float value; };
struct Shape {std::unique_ptr<sf::CircleShape> shape; };
struct AccelerationPre {arma::vec2 value; };
struct AccelerationPost {arma::vec2 value; };

// We differentiate between the entities - in strict ECS this would most likely be unnecessary -
struct ParticleTag {bool value; };
struct HoleTag {bool value; };

#endif //BOXED_PARTICLES_COMPONENTS_H