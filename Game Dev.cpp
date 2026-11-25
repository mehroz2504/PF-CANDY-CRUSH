#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    // Create game window
    sf::RenderWindow window(
        sf::VideoMode({ 800, 600 }),
        "My First SFML Game"
    );

    window.setFramerateLimit(60);

    // Game loop
    while (window.isOpen())
    {
        // Handle events
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        // Update game logic


        // Render game
        window.clear(sf::Color::Black);

        // Draw your game objects here


        window.display();
    }

    return 0;
}