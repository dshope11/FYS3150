//
// Created by Nils Enric Canut Taugbøl on 08/01/2025.
//

#include "precomp.h"
#include "include/box.h"

int main() {
    Box box;
    box.Init();

    Timer t;
    double accTime = 0.0;
    float frameTimeAvg = 10.0f; // estimate of average time spent per frame in ms

    while (Box::window.isOpen()) {

        while (const std::optional event = Box::window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                Box::window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    Box::window.close();
            }
        }
        accTime = 0.0;
        while (accTime <= 1000/60.) {
            t.reset();
            box.update(dt);

            const float time = t.elapsed();
            accTime += time*1000;    // time elapsed in ms

            frameTimeAvg = 0.925f * frameTimeAvg + 0.075f * time * 1000;
            printf( "update time: %5.2fms\n", frameTimeAvg );
        }

        box.render();
    }
}
