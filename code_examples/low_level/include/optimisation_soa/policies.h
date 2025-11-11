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
        // by precomputing the reciprocal we should save some time too although usually the compiler does computes the reciprocal anyway
        // the only way to know the improvement each change does is it run the code for each change (and profile)
        arma::vec2 direction = position_other - position_original;
        double dist2 = direction[0]*direction[0] + direction[1]*direction[1];
        double dist = std::sqrt(dist2);

        double reciprocal = 1.0 / (dist2*dist);

        return direction * GM * reciprocal;
    }

    inline void braid2baseline(const arma::vec2& position_original1, const arma::vec2& position_other1, const double& GM1, arma::vec2& acceleration1,
                        const arma::vec2& position_original2, const arma::vec2& position_other2, const double& GM2, arma::vec2& acceleration2) {
        arma::vec2 direction_1 = position_other1 - position_original1;
        arma::vec2 direction_2 = position_other2 - position_original2;


        double dist_1 = arma::norm(direction_1);
        double dist_2 = arma::norm(direction_2);

        acceleration1 = GM1 / std::pow(dist_1, 2) * arma::normalise(direction_1);
        acceleration2 = GM2 / std::pow(dist_2, 2) * arma::normalise(direction_2);

        return;
    }

    inline void braid2simplified(const arma::vec2& position_original1, const arma::vec2& position_other1, const double& GM1, arma::vec2& acceleration1,
                        const arma::vec2& position_original2, const arma::vec2& position_other2, const double& GM2, arma::vec2& acceleration2) {
        arma::vec2 direction_1 = position_other1 - position_original1;
        arma::vec2 direction_2 = position_other2 - position_original2;

        double dist2_1 = direction_1[0]*direction_1[0] + direction_1[1]*direction_1[1];
        double dist2_2 = direction_2[0]*direction_2[0] + direction_2[1]*direction_2[1];

        double dist_1 = std::sqrt(dist2_1);
        double dist_2 = std::sqrt(dist2_2);

        double reciprocal_1 = 1.0 / (dist2_1*dist_1);
        double reciprocal_2 = 1.0 / (dist2_2*dist_2);

        acceleration1 += direction_1 * GM1 * reciprocal_1;
        acceleration2 += direction_2 * GM2 * reciprocal_2;

        return;
    }
}

// we add the flags to toggle the implementations we want
// during compilation you can flag -DCOST_FUNC=FUNCTIONNAME where FUNCTIONNAME is the name of the function from the list above you want to use
#ifndef COST_FUNC
#define COST_FUNC baseline
#endif
#ifndef ILP
#define ILP NONE
#endif