//
// Created by Nils Enric Canut Taugbøl on 03/11/2025.
//

#ifndef BOXED_PARTICLES_BOX_H
#define BOXED_PARTICLES_BOX_H

#include "precomp.h"
#include "registry.h"

class Box
{
public:
       void Init();
       void update(double dt);
       void render();

       static arma::vec2 startSide(int start);

       WorldRegistry world;

       // updates accelerations - may have to separate for current acceleration and predicted acceleration
       // it would be the case if Heun's method is not applied everywhere in the simulation
       void resetAccelerations();
       void gravitySystem(double dt);
       void integrateSystem(double dt);
       void eliminationSystem();
       void spawnSystem();
       void generateParticle();
       void generateHole();
       void updateShapes();

       // static data, for global access
       static inline arma::vec2 upperBounds = {SCRNWIDTH, SCRNHEIGHT};
       static inline arma::vec2 lowerBounds = {0., 0.};
       static inline sf::RenderWindow window;
};

#endif //BOXED_PARTICLES_BOX_H