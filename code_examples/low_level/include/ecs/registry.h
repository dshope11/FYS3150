//
// Created by Nils Enric Canut Taugbøl on 03/11/2025.
//

#ifndef BOXED_PARTICLES_REGISTRY_H
#define BOXED_PARTICLES_REGISTRY_H

#pragma once
#include "components.h"
#include "storage.h"

// We can use uint32_t without a problem as the maximum number will then be 2^(31)-1 which is HUGE
// so maybe this can be a source for optimisation later -a little treat if you will-

struct Registry {
    EntityID next{0};   // fancy modern C++ means initialise first instance as 0
    EntityID create() {return next++; } // here we update the identifier for each call to the create member function
};

struct WorldRegistry {
    Registry entities;

    Storage<Position> pos;
    Storage<Velocity> vel;
    Storage<Mass>     mass;
    Storage<Radius>   radius;
    Storage<Shape> shapes;
    Storage<AccelerationPre> accelerationPre;
    Storage<AccelerationPost> accelerationPost;

    Storage<ParticleTag> particle;
    Storage<HoleTag>     hole;
};

#endif //BOXED_PARTICLES_REGISTRY_H