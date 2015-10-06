// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Ci mancano ancora due importanti elementi: il "paddle"
// e i mattoncini. Inizieremo implementando un paddle controllato
// dal giocatore in questo segmento di codice.

#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

class Ball
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{8.f};

    sf::CircleShape shape;
    sf::Vector2f velocity{-defVelocity, -defVelocity};

    Ball(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    void update()
    {
        shape.move(velocity);
        solveBoundCollisions();
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float left() const noexcept { return x() - shape.getRadius(); }
    float right() const noexcept { return x() + shape.getRadius(); }
    float top() const noexcept { return y() - shape.getRadius(); }
    float bottom() const noexcept { return y() + shape.getRadius(); }

private:
    // Un semplice refactoring ("extract function") migliora sempre
    // la leggibilità del codice.
    void solveBoundCollisions() noexcept
    {
        if(left() < 0 || right() > wndWidth) velocity.x *= -1.f;
        if(top() < 0 || bottom() > wndHeight) velocity.y *= -1.f;
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

// Come la pallina, la classe `Paddle` rappresenterà un game object,
// con i suoi metodi `update` e `draw`.
class Paddle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{75.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    // Questa volta utilizzeremo un `sf::RectangleShape` come forma.
    sf::RectangleShape shape;
    sf::Vector2f velocity;

    // Come per la pallina, costruiamo il paddle passando la
    // posizione iniziale ed inizializziando la forma SFML.
    Paddle(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update()
    {
        // Prima di muovere il paddle, processeremo l'input del
        // giocatore, che modificherà la velocità.
        processPlayerInput();
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }

private:
    void processPlayerInput()
    {
        // Modificheremo la velocità del paddle in base ai tasti
        // correntemente pressati dall'utente:
        // * Se la freccia sinistra è pressata, settiamo la velocità
        //   X ad un valore negativo.
        // * Se la freccia destra è pressata, settiamo la velocità X
        //   ad un valore positivo.
        // * Se nessuna delle due freccie è pressata, settiamo la
        //   velocità X a zero.

        // Non applicheremo alcun cambio di velocità se potrebbe
        // spingere il paddle "fuori dalla finestra".

        // Quindi, se l'utente sta cercando di muovere il paddle
        // verso destra, ma esso è già fuori in quella direzione,
        // eviteremo di modificare la velocità.

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
        {
            velocity.x = -defVelocity;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) &&
                right() < wndWidth)
        {
            velocity.x = defVelocity;
        }
        else
        {
            velocity.x = 0;
        }
    }
};

const sf::Color Paddle::defColor{sf::Color::Red};

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};

    // Creiamo un istanza di `Paddle`.
    Paddle paddle{wndWidth / 2, wndHeight - 50};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 4"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        // ...non dimentichiamoci di chiamare `update()` e `draw()`.

        ball.update();
        paddle.update();

        ball.draw(window);
        paddle.draw(window);

        window.display();
    }

    return 0;
}