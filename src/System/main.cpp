#include "SFML/Graphics.hpp"
#include "invaders.h"



int main(int argc, char** argv)
{
    Invaders invaders;
    invaders.load_rom(argv[1]);

    sf::RenderWindow window(sf::VideoMode(420,480), "spaceinvaders");
    window.setFramerateLimit(60);
    window.setPosition(sf::Vector2i(500, 250));
    sf::Event event;

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == event.Closed)
                window.close();
           
            invaders.handle_event(event);
        }

        invaders.execute_instruction();        
        invaders.render(window);

    }
}