//
// Created by Nils Enric Canut Taugbøl on 24/10/2025.
//

#pragma once
#include "precomp.h"
#include "particle.h"
#include "hole.h"

class Box
{
public:
    void Init();
    void update(double dt);
    void render();

    static arma::vec2 startSide(int start);

    void generateParticle();
    void generateHole();

    void updateParticles(double dt);
    void deleteParticle(int index);

    Particles particles;
    Holes holes;

    // static data, for global access
    static inline arma::vec2 upperBounds = {SCRNWIDTH, SCRNHEIGHT};
    static inline arma::vec2 lowerBounds = {0., 0.};
    static inline sf::RenderWindow window;
};
