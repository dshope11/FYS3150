//
// Created by Nils Enric Canut Taugbøl on 07/11/2025.
//

#pragma once
#include "precomp.h"

namespace policy {  // we use a namespace to avoid collisions and to keep all policies under one group

    inline arma::vec2 baseline(const arma::vec2& position_original, const arma::vec2& position_other, const double& GM) {
        arma::vec2 direction = position_other - position_original;
        double dist = norm(direction);
        arma::vec2 total_force = GM / std::pow(dist, 2) * normalise(direction);
        return total_force;
    }

    inline arma::vec2 simplified(const arma::vec2& position_original, const arma::vec2& position_other, const double& GM) {
        // compared to baseline we reduce the amount of sqrt from 2 (norm and normalise) to 1
        // we drop the mass as we return an acceleration -this might have been handled by the compiler before anyway
        // we also effectively reduce the amount of divisions from 2 (normalise and the division you see) to 1
        arma::vec2 direction = position_other - position_original;
        double dist2 = direction[0]*direction[0] + direction[1]*direction[1];
        double dist = std::sqrt(dist2);

        double reciprocal = 1.0 / (dist2*dist);

        return direction * GM * reciprocal;
    }
}

// we add the flags to toggle the implementations we want
// during compilation you can flag -DCOST_FUNC=policy::FUNCTIONNAME where FUNCTIONNAME is the name of the function from the list above you want to use
#ifndef COST_FUNC
#define COST_FUNC policy::baseline
#endif