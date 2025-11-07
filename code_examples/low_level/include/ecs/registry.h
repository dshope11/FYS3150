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
    std::vector<EntityID> freeIDs;   // LIFO stack

    EntityID create() {
        if (!freeIDs.empty()) {
            const EntityID entity = freeIDs.back();
            freeIDs.pop_back();
            return entity;
        }
        return next++;
    }

    void destroy(const EntityID entity) {
        freeIDs.push_back(entity);
    }
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