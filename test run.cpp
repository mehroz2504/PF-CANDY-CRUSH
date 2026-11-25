#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML Test");
    window.setFramerateLimit(60);

    sf::CircleShape player(25.f);
    player.setFillColor(sf::Color::Green);
    player.setPosition({ 375.f, 275.f });

    sf::RectangleShape coin({ 20.f, 20.f });
    coin.setFillColor(sf::Color::Yellow);
    coin.setPosition({ 100.f, 100.f });

    int score = 0;
    const float speed = 5.f;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        using K = sf::Keyboard::Key;
        if (sf::Keyboard::isKeyPressed(K::Left) || sf::Keyboard::isKeyPressed(K::A)) player.move({ -speed, 0.f });
        if (sf::Keyboard::isKeyPressed(K::Right) || sf::Keyboard::isKeyPressed(K::D)) player.move({ speed, 0.f });
        if (sf::Keyboard::isKeyPressed(K::Up) || sf::Keyboard::isKeyPressed(K::W)) player.move({ 0.f, -speed });
        if (sf::Keyboard::isKeyPressed(K::Down) || sf::Keyboard::isKeyPressed(K::S)) player.move({ 0.f, speed });

        if (player.getGlobalBounds().findIntersection(coin.getGlobalBounds())) {
            score++;
            coin.setPosition({ static_cast<float>(std::rand() % 760),
                     static_cast<float>(std::rand() % 560) });