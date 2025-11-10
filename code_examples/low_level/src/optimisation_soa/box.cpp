//
// Created by Nils Enric Canut Taugbøl on 24/10/2025.
//

#include "optimisation_soa/box.h"
#include "optimisation_soa/policies.h"

// for the generation of particles and holes
std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution massDistribution(MIN_MASS, MAX_MASS);
std::uniform_int_distribution particleStartSide(0, 3);
std::uniform_real_distribution startParticleXDistribution(0.f, (float) SCRNWIDTH);
std::uniform_real_distribution startParticleYDistribution(0.f, (float) SCRNHEIGHT);
std::uniform_real_distribution posXDistribution(0.1f * (float) SCRNWIDTH, 0.9f * (float) SCRNWIDTH);
std::uniform_real_distribution posYDistribution(0.1f * (float) SCRNHEIGHT, 0.9f * (float) SCRNHEIGHT);
std::uniform_real_distribution radiusDistribution(MIN_RADIUS, MAX_RADIUS);
std::uniform_real_distribution velocityDistribution(MIN_START_VEL, MAX_START_VEL);

void Box::Init() {

    // generate holes <- this is a redundant comment
    for (int i = 0; i < N_HOLES; i++) {
        generateHole();
    }

    for (int i = 0; i < PARTICLE_WAVE; i++) {
        generateParticle();
    }

    // set up the view of the screen
    window = sf::RenderWindow(sf::VideoMode({SCRNWIDTH, SCRNHEIGHT}), "Particle Simulation");
    // draw the starting screen
    for (const std::unique_ptr<sf::CircleShape> & shape : holes.shapes) {
        window.draw(*shape);
    }
    for (const std::unique_ptr<sf::CircleShape> & shape : particles.shapes) {
        window.draw(*shape);
    }

    window.display();
}

void Box::update(double dt) {
    // update only particles as the holes do nothing yet!
    updateParticles(dt);

    // new wave of particles
    for (int i = 0; i < std::min(PARTICLE_WAVE, N_PARTICLES - (int) particles.size()); i++) {
        generateParticle();
    }
}

void Box::render() {
    window.clear();

    // draw!
    for (int i = 0; i < holes.size(); i++) {
        window.draw(*holes.shapes[i]);
    }
    for (int i = 0; i < particles.size(); i++) {
        window.draw(*particles.shapes[i]);
    }

    window.display();
}

void Box::generateHole() {
    arma::vec2 start_pos = arma::vec2{posXDistribution(gen), posYDistribution(gen)};
    float radius = radiusDistribution(gen);
    double mass = massDistribution(gen);

    bool okay = false;
    int iteration = 0;

    // cannot generate a hole above another one
    // this is a terrible implementation
    while (!okay && holes.size() > 0 && iteration < 10000) {
        for (int i = 0; i < holes.size(); i++) {
            if (norm(start_pos - holes.pos[i]) <= radius + holes.radius[i]) {
                okay = false;
                start_pos = arma::vec2{posXDistribution(gen), posYDistribution(gen)};
                break;
            }
            okay = true;
        }
        iteration++;
    }

    return holes.add(start_pos, mass, radius);
}

void Box::generateParticle() {
    arma::vec2 start_pos = startSide(particleStartSide(gen));
    arma::vec2 start_vel = {velocityDistribution(gen), velocityDistribution(gen)};

    int iteration = 0;
    bool okay = false;

    while (!okay && particles.size() >  0 && iteration < 10000) {
        for (int i = 0; i < particles.size(); i++) {
            if (norm(start_pos - particles.pos[i]) <= part_radius*2) {
                okay = false;
                start_pos = startSide(particleStartSide(gen));
                break;
            }
            okay = true;
        }
        iteration++;
    }
    return particles.add(start_pos, start_vel, part_mass, part_radius);
}

arma::vec2 Box::startSide(const int start) {
    if (start == 0) {
        // start left side
        return arma::vec2 {0.0, startParticleYDistribution(gen)};
    }
    if (start == 1) {
        // start top
        return arma::vec2 {startParticleXDistribution(gen), 0.0};
    }
    if (start == 2) {
        // start right side
        return arma::vec2 {(double) SCRNWIDTH-1., startParticleYDistribution(gen)};
    }
    if (start == 3) {
        // bottom start
        return arma::vec2 {startParticleXDistribution(gen), (double) SCRNHEIGHT - 1.};
    }

    return arma::vec2 {0.0, 0.0};
}

void Box::updateParticles(double dt) {
    // clear the previous acclerations
    for (int i = 0; i < particles.size(); i++) {
        particles.accelerationPre[i] = {0., 0.};
        particles.accelerationPost[i] = {0., 0.};
    }

    // OPT: we can precompute G * M for all the holes
    std::vector<double> GM_holes(holes.size());
    for (int i = 0; i < holes.size(); i++) {
        GM_holes[i] = G * holes.mass[i];
    }

#if ILP == NONE
    // we update the particles through Heun's method
    // we find the acceleration of the particles
    for (int i = 0; i < particles.size(); i++) {
        // start by finding the acceleration from holes
        for (int ii = 0; ii < holes.size(); ii++) {
            // OPT: we implement the function we want through macros
            particles.accelerationPre[i] += COST_FUNC(particles.pos[i], holes.pos[ii], GM_holes[ii]);
        }

        // from the particles without double counting
        for (int ii = i+1; ii < particles.size(); ii++) {
            arma::vec2 acceleration = COST_FUNC(particles.pos[i], particles.pos[ii], G * particles.mass[ii]);
            particles.accelerationPre[i] += acceleration;
            particles.accelerationPre[ii] -= acceleration * particles.mass[i] / particles.mass[ii];

        }
    }

    // to update the velocity now we need the acceleration at the new point
    for (int i = 0; i < particles.size(); i++) {
        // start by finding the acceleration from holes
        for (int ii = 0; ii < holes.size(); ii++) {
            particles.accelerationPost[i] += COST_FUNC(particles.pos[i] + dt * particles.vel[i], holes.pos[ii], GM_holes[ii]);
        }

        // from the particles without double counting
        for (int ii = i+1; ii < particles.size(); ii++) {
            arma::vec2 acceleration = COST_FUNC(particles.pos[i] + dt * particles.vel[i], particles.pos[ii] + dt * particles.vel[ii], G * particles.mass[ii]);
            particles.accelerationPost[i] += acceleration;
            particles.accelerationPost[ii] -= acceleration * particles.mass[i] /  particles.mass[ii];
        }
    }

    // we can now update the position and velocities according to Heun's method
    for (int i = 0; i < particles.size(); i++) {
        particles.pos[i] += (particles.vel[i] + particles.vel[i] + dt * particles.accelerationPre[i]) * dt/2;
    }

    for (int i = 0; i < particles.size(); i++) {
        particles.vel[i] += (particles.accelerationPre[i] + particles.accelerationPost[i]) * dt/2;
    }
#elif ILP == BRAID2
    // we update the particles through Heun's method
    // we find the acceleration of the particles
    for (int i = 0; i < particles.size(); i++) {
        // start by finding the acceleration from holes
        int ii = 0;
        for (; ii + 1 < holes.size(); ii+=2) {
            // OPT: we implement the function we want through macros
            arma::vec2 acc_1 = {0., 0.};
            arma::vec2 acc_2 = {0., 0.};
            policy::braid2simplified(particles.pos[i], holes.pos[ii], GM_holes[ii], acc_1,
                                     particles.pos[i], holes.pos[ii+1], GM_holes[ii+1], acc_2);

            particles.accelerationPre[i] += acc_1 + acc_2;
        }
        // in case the number of holes is odd
        if (ii < holes.size()) {
            particles.accelerationPre[i] += COST_FUNC(particles.pos[i], holes.pos[ii], GM_holes[ii]);
        }

        ii = i+1;
        // from the particles without double counting
        for (; ii + 1 < particles.size(); ii+=2) {
            arma::vec2 acc_1 = {0., 0.};
            arma::vec2 acc_2 = {0., 0.};
            policy::braid2simplified(particles.pos[i], particles.pos[ii], G * particles.mass[ii], acc_1,
                                     particles.pos[i], particles.pos[ii+1], G * particles.mass[ii+1], acc_2);

            particles.accelerationPre[i] += acc_1 + acc_2;
            particles.accelerationPre[ii] -= acc_1 * particles.mass[i] / particles.mass[ii];
            particles.accelerationPre[ii+1] -= acc_2 * particles.mass[i] / particles.mass[ii+1];
        }
        // in case the number of holes is odd
        if (ii < particles.size()) {
            arma::vec2 acceleration = COST_FUNC(particles.pos[i], particles.pos[ii], G * particles.mass[ii]);
            particles.accelerationPre[i] += acceleration;
            particles.accelerationPre[ii] -= acceleration * particles.mass[i] / particles.mass[ii];
        }
    }

    // to update the velocity now we need the acceleration at the new point
    for (int i = 0; i < particles.size(); i++) {
        // start by finding the acceleration from holes
        int ii = 0;
        for (; ii + 1 < holes.size(); ii+=2) {
            // OPT: we implement the function we want through macros
            arma::vec2 acc_1 = {0., 0.};
            arma::vec2 acc_2 = {0., 0.};
            policy::braid2simplified(particles.pos[i] + dt * particles.vel[i], holes.pos[ii], GM_holes[ii], acc_1,
                                     particles.pos[i] + dt * particles.vel[i], holes.pos[ii+1], GM_holes[ii+1], acc_2);

            particles.accelerationPre[i] += acc_1 + acc_2;
        }
        // in case the number of holes is odd
        if (ii < holes.size()) {
            particles.accelerationPre[i] += COST_FUNC(particles.pos[i] + dt * particles.vel[i], holes.pos[ii], GM_holes[ii]);
        }

        ii = i+1;
        // from the particles without double counting
        for (; ii + 1 < particles.size(); ii+=2) {
            arma::vec2 acc_1 = {0., 0.};
            arma::vec2 acc_2 = {0., 0.};
            policy::braid2simplified(particles.pos[i] + dt * particles.vel[i], particles.pos[ii] + dt * particles.vel[ii], G * particles.mass[ii], acc_1,
                                     particles.pos[i] + dt * particles.vel[i], particles.pos[ii+1] + dt * particles.vel[ii+1], G * particles.mass[ii+1], acc_2);

            particles.accelerationPre[i] += acc_1 + acc_2;
            particles.accelerationPre[ii] -= acc_1 * particles.mass[i] / particles.mass[ii];
            particles.accelerationPre[ii+1] -= acc_2 * particles.mass[i] / particles.mass[ii+1];
        }
        // in case the number of holes is odd
        if (ii < particles.size()) {
            arma::vec2 acceleration = COST_FUNC(particles.pos[i] + dt * particles.vel[i], particles.pos[ii] + dt * particles.vel[ii], G * particles.mass[ii]);
            particles.accelerationPre[i] += acceleration;
            particles.accelerationPre[ii] -= acceleration * particles.mass[i] / particles.mass[ii];
        }
    }

    // we can now update the position and velocities according to Heun's method
    for (int i = 0; i < particles.size(); i++) {
        particles.pos[i] += (particles.vel[i] + particles.vel[i] + dt * particles.accelerationPre[i]) * dt/2;
    }

    for (int i = 0; i < particles.size(); i++) {
        particles.vel[i] += (particles.accelerationPre[i] + particles.accelerationPost[i]) * dt/2;
    }
#endif
    // if the particle is outside of the box wrap it back in on the opposite side
    // TODO: add WRAP_PARTICLES as a possible definition for cmake in the README
#if WRAP_PARTICLES == 1
    for (int i = 0; i < particles.size(); i++) {
        for (int ii = 0; ii < particles.pos[i].n_elem; ii++) {
            if (particles.pos[i](ii) < Box::lowerBounds(ii)) {
                particles.pos[i](ii) = Box::upperBounds(ii) - fmod(Box::lowerBounds(ii) - particles.pos[i](ii), Box::upperBounds(ii) - Box::lowerBounds(ii));
            }
            else if (particles.pos[i](ii) >= Box::upperBounds(ii)) {
                particles.pos[i](ii) = Box::lowerBounds(ii) + fmod(particles.pos[i](ii) - Box::lowerBounds(ii), Box::upperBounds(ii) - Box::lowerBounds(ii));
            }
        }
    }
#else
    for (int i = 0; i < particles.size(); i++) {
        if (any(particles.pos[i] < Box::lowerBounds) || any(particles.pos[i] >= Box::upperBounds)) {
            deleteParticle(i);
            i--;
        }
    }
#endif

    // delete particle if it fell into a hole
    for (int i = 0; i < particles.size(); i++) {
        for (int ii = 0; ii < holes.size(); ii++) {
            if (arma::norm(holes.pos[ii] - particles.pos[i]) <= holes.radius[ii]) {
                deleteParticle(i);
            }
        }
    }

    // update on the screen
    for (int i = 0; i < particles.size(); i++) {
        particles.shapes[i]->setPosition(sf::Vector2f(particles.pos[i](0), particles.pos[i](1)));
    }
}

void Box::deleteParticle(int index) {
    size_t last = particles.size() - 1;

    if (index != last) {
        particles.pos[index] = std::move(particles.pos[last]);
        particles.vel[index] = std::move(particles.vel[last]);
        particles.accelerationPre[index] = std::move(particles.accelerationPre[last]);
        particles.accelerationPost[index] = std::move(particles.accelerationPost[last]);
        particles.mass[index] = std::move(particles.mass[last]);
        particles.radius[index] = std::move(particles.radius[last]);
        particles.shapes[index] = std::move(particles.shapes[last]);
    }

    particles.pos.pop_back();
    particles.vel.pop_back();
    particles.accelerationPre.pop_back();
    particles.accelerationPost.pop_back();
    particles.mass.pop_back();
    particles.radius.pop_back();
    particles.shapes.pop_back();
}