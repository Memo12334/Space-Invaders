#include "SFML/Graphics.hpp"
#include "invaders.h"



int main(int argc, char** argv)
{
    Invaders invaders;
    invaders.load_rom(argv[1]);

    sf::RenderWindow window(sf::VideoMode(800,600), "spaceinvaders");
    window.setFramerateLimit(60);
    sf::Event event;

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            invaders.execute_instruction();
            invaders.handle_event(event);
        }
        invaders.render(window);

    }
}