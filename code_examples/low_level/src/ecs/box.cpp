//
// Created by Nils Enric Canut Taugbøl on 03/11/2025.
//

#include "ecs/box.h"

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

    for (int i = 0; i < N_HOLES; i++) {
        generateHole();
    }

    for (int i = 0; i < PARTICLE_WAVE; i++) {
        generateParticle();
    }

    // set up the screen
    window = sf::RenderWindow(sf::VideoMode({SCRNWIDTH, SCRNHEIGHT}), "Particle Simulation");
    for (EntityID i : world.shapes.entities) {
        window.draw(*world.shapes.get(i).shape);
    }

    window.display();
}

void Box::update(double dt) {

    // run all relevant systems
    resetAccelerations();
    gravitySystem(dt);
    integrateSystem(dt);
    eliminationSystem();
    spawnSystem();
    updateShapes();

}

void Box::render() {
    window.clear();

    for (EntityID i : world.shapes.entities) {
        window.draw(*world.shapes.get(i).shape);
    }

    window.display();
}

void Box::generateHole() {
    EntityID entity = world.entities.create();

    arma::vec2 start_pos = arma::vec2{posXDistribution(gen), posYDistribution(gen)};
    float radius = radiusDistribution(gen);
    double mass = massDistribution(gen);

    bool okay = false;
    int iteration = 0;

    // cannot generate a hole above another one
    // this is a terrible implementation
    while (!okay && entity > 0 && iteration < 10000) {
        // We find the present entities within the world positions and check that they indeed have a radius
        for (EntityID i : world.pos.entities) if (world.radius.has(i)) {
            if (norm(start_pos - world.pos.get(i).value) <= radius + world.radius.get(i).value) {
                okay = false;
                start_pos = arma::vec2{posXDistribution(gen), posYDistribution(gen)};
                break;
            }
            okay = true;
        }
        iteration++;
    }

    auto s = std::make_unique<sf::CircleShape>(radius);
    s->setOrigin({radius, radius});
    s->setPosition(sf::Vector2f{(float) (start_pos(0)), (float) (start_pos(1))});
    s->setFillColor(sf::Color::Transparent);
    s->setOutlineColor(sf::Color::Cyan);
    s->setOutlineThickness(1.);

    world.pos.add(entity, {start_pos});
    world.mass.add(entity, {mass});
    world.radius.add(entity, {radius});
    world.shapes.add(entity, {std::move(s)});
    world.hole.add(entity, {true});
}

void Box::generateParticle() {
    EntityID entity = world.entities.create();

    arma::vec2 start_pos = startSide(particleStartSide(gen));
    arma::vec2 start_vel = {velocityDistribution(gen), velocityDistribution(gen)};
    float radius = part_radius;
    double mass = part_mass;

    bool okay = false;
    int iteration = 0;

    // cannot generate a hole above another one
    // this is a terrible implementation
    while (!okay && entity > 0 && iteration < 10000) {
        // We find the present entities within the world positions and check that they indeed have a radius
        for (EntityID i : world.pos.entities) if (world.radius.has(i)) {
            if (arma::norm(start_pos - world.pos.get(i).value) <= radius + world.radius.get(i).value) {
                okay = false;
                start_pos = startSide(particleStartSide(gen));
                break;
            }
            okay = true;
        }
        iteration++;
    }

    auto s = std::make_unique<sf::CircleShape>(radius);
    s->setOrigin({radius, radius});
    s->setPosition(sf::Vector2f{(float) (start_pos(0)), (float) (start_pos(1))});
    s->setFillColor(sf::Color::Yellow);

    world.pos.add(entity, {start_pos});
    world.vel.add(entity, {start_vel});
    world.mass.add(entity, {mass});
    world.radius.add(entity, {radius});
    world.shapes.add(entity, {std::move(s)});
    world.particle.add(entity, {true});
    world.accelerationPost.add(entity, {arma::vec2{0., 0.}});
    world.accelerationPre.add(entity, {arma::vec2{0., 0.}});
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

void Box::resetAccelerations() {
    for (EntityID i : world.accelerationPost.entities) {
        world.accelerationPost.get(i).value = {0., 0.};
        world.accelerationPre.get(i).value = {0., 0.};
    }
}

void Box::gravitySystem(double dt) {
    // we'll loop through velocity but could just as well be particles or anything else
    // I'll try to make this one as general as possible, for each entity with velocity we iterate through all other entities with mass
    // it's slower now, but it is most general and correct implementation of a gravity system
    for (EntityID i : world.vel.entities) {
        for (EntityID ii : world.mass.entities) if (i != ii) {
            arma::vec2 direction = world.pos.get(ii).value - world.pos.get(i).value;
            double dist = norm(direction);
            arma::vec2 gravity = G * world.mass.get(ii).value * world.mass.get(i).value / std::pow(dist, 2) * normalise(direction);
            world.accelerationPre.get(i).value += gravity / world.mass.get(i).value;

            // and now for Heun prediction step
            // holes don't have velocities as of yet
            if (!world.vel.has(ii)) {
                direction = world.pos.get(ii).value - (world.pos.get(i).value + dt * world.vel.get(i).value);
                dist = norm(direction);
                gravity = G * world.mass.get(ii).value * world.mass.get(i).value / std::pow(dist, 2) * normalise(direction);
                world.accelerationPost.get(i).value += gravity / world.mass.get(i).value;
            }
            else {
                direction = (world.pos.get(ii).value + dt * world.vel.get(ii).value) - (world.pos.get(i).value + dt * world.vel.get(i).value);
                dist = norm(direction);
                gravity = G * world.mass.get(ii).value * world.mass.get(i).value / std::pow(dist, 2) * normalise(direction);
                world.accelerationPost.get(i).value += gravity / world.mass.get(i).value;
            }
        }
    }
}

void Box::integrateSystem(double dt) {
     for (EntityID i : world.vel.entities) {
         world.pos.get(i).value += (world.vel.get(i).value + dt * world.accelerationPre.get(i).value) * dt/2;
         world.vel.get(i).value += (world.accelerationPre.get(i).value + world.accelerationPost.get(i).value) * dt/2;
     }
}

void Box::eliminationSystem() {
#if WRAP_PARTICLES == 1
    for (EntityID i : world.particle.entities) {
        for (int ii : world.pos.get(i).value.n_elem) {
            if (world.pos.get(i).value(ii) < Box::lowerBounds(ii)) {
                world.pos.get(i).value(ii) = Box::upperBounds(ii) - fmod(Box::lowerBounds(ii) - world.pos.get(i).value(ii), Box::upperBounds(ii) - Box::lowerBounds(ii));
            }
            else if (world.pos.get(i).value(ii) >= Box::upperBounds(ii)) {
                world.pos.get(i).value(ii) = Box::lowerBounds(ii) + fmod(world.pos.get(i).value(ii) - Box::lowerBounds(ii), Box::upperBounds(ii) - Box::lowerBounds(ii));
            }
        }
    }
#else
    for (EntityID i : world.particle.entities) {
        if (any(world.pos.get(i).value < Box::lowerBounds || world.pos.get(i).value >= Box::upperBounds)) {
            world.pos.remove(i);
            world.vel.remove(i);
            world.mass.remove(i);
            world.radius.remove(i);
            world.shapes.remove(i);
            world.particle.remove(i);
            world.accelerationPost.remove(i);
            world.accelerationPre.remove(i);

            world.entities.destroy(i);
        }
    }
#endif

    for (EntityID i : world.particle.entities) {
        for (EntityID ii : world.hole.entities) {
            if (arma::norm(world.pos.get(ii).value - world.pos.get(i).value) <= world.radius.get(ii).value) {
                world.pos.remove(i);
                world.vel.remove(i);
                world.mass.remove(i);
                world.radius.remove(i);
                world.shapes.remove(i);
                world.particle.remove(i);
                world.accelerationPost.remove(i);
                world.accelerationPre.remove(i);

                world.entities.destroy(i);
            }
        }
    }
}

void Box::spawnSystem() {
    for (int i = 0; i < std::min(PARTICLE_WAVE, N_PARTICLES - (int) world.particle.entities.size()); i++) {
        generateParticle();
    }
}

void Box::updateShapes() {
    // update on the screen
    for (EntityID i : world.shapes.entities) if (world.pos.has(i)) {
        world.shapes.get(i).shape->setPosition(sf::Vector2f(world.pos.get(i).value(0), world.pos.get(i).value(1)));
    }
}
