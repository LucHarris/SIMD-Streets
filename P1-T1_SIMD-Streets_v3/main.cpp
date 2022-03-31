#include <SFML/Graphics.hpp>
#include "Constants.h"
#include <cassert>
#include "SceneObjects.h"
#include <random>
int main()
{
    srand(time(0U));
    // create the window
    sf::RenderWindow window(sf::VideoMode(gc::WINDOW_WIDTH, gc::WINDOW_HEIGHT), gc::APP_NAME);
    sf::Clock clock;
    sf::Time elapsed;
    SceneObjects sceneObjects;

    

    // initialise
    {
        sceneObjects.Init();
    }
    // update/render
    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        elapsed = clock.restart();
        sceneObjects.Update(elapsed.asSeconds());

        // render
        window.clear(gc::WINDOW_FILL);
        sceneObjects.Draw(window);
        window.display();
    }

    return 0;
}