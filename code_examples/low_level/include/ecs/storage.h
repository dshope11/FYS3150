//
// Created by Nils Enric Canut Taugbøl on 03/11/2025.
//

#ifndef BOXED_PARTICLES_STORAGE_H
#define BOXED_PARTICLES_STORAGE_H

#pragma once
#include "ecs/registry.h"

template<class T>
struct Storage {
    std::vector<T> data;
    std::vector<EntityID> entities; // the entities which have the relevant component
    std::vector<int> mapToEntityIdx;

    bool has(EntityID entity) {
        bool possible = entity < mapToEntityIdx.size();
        bool not_removed = mapToEntityIdx[entity] != -1;

        return possible && not_removed;
    }

    T &get(EntityID entity) {
            return data[mapToEntityIdx[entity]];
        }

    void add(EntityID entity, T component) {
        entities.push_back(entity);
        data.push_back(std::move(component));

        // since the entity ID keeps growing (a potential but unlikely problem) we need to adapt the mapping
        if (entity >= mapToEntityIdx.size()) {
            mapToEntityIdx.resize(entity + 1, -1);
        }
        mapToEntityIdx[entity] = (int) entities.size() - 1;
    }

    void remove(EntityID entity) {
        int idx = mapToEntityIdx[entity];
        int lastIdx = entities.size() - 1;

        // we only need to move things within THIS storage -it's quite neat-
        if (idx != lastIdx) {
            EntityID lastEntity = entities[lastIdx];
            data[idx] = std::move(data[lastIdx]);
            entities[idx] = lastEntity;
            mapToEntityIdx[lastEntity] = idx;
        }

        data.pop_back();
        entities.pop_back();
        mapToEntityIdx[entity] = -1;
    }
};

#endif //BOXED_PARTICLES_STORAGE_H