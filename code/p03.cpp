// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// La pallina si muove! ...dobbiamo tuttavia trovare un modo
// per evitare che esca fuori dalla finestra.

// Controllando se la coordinata X della pallina sia maggiore
// della lunghezza della finestra o se sia inferiore a `0` possiamo
// verificare che la pallina abbia lasciato la finestra 
// orizzontalmente. Lo stesso principio si può applicare per le
// coordinate verticali.
// {Info: ball vs window collision}

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

    // Definiamo dei metodi getter di "shortcut" per calcolare le
    // coordinate sx/dx/sopra/sotto della pallina che utilizzeremo
    // frequentemente.
    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float left() const noexcept     { return x() - shape.getRadius(); }
    float right() const noexcept    { return x() + shape.getRadius(); }
    float top() const noexcept      { return y() - shape.getRadius(); }
    float bottom() const noexcept   { return y() + shape.getRadius(); }

    void update()
    {
        // Dobbiamo riuscire a mantenere la pallina "dentro la 
        // finestra".
        // Il modo più comune (e probabilmente migliore) per farlo, 
        // e per gestire qualsiasi tipo di "collision detection", è 
        // prima muovere l'oggetto e poi controllare se è avvenuta 
        // un intersezione.
        // Se il test è positivo, rispondiamo alla collisione
        // alterando la posizione e/o la velocità dell'oggetto.

        // Iniziamo, quindi, muovendo la pallina.
        shape.move(velocity);

        // Dopo che la pallina è stata spostata, potrebbe trovarsi
        // "fuori dalla finestra". Dobbiamo controllare tale 
        // eventualità in ogni direzione e rispondere adeguatamente
        // modificando la velocità.

        // Se la pallina sta uscendo fuori orizzontalmente, basterà 
        // invertire la sua velocità orizzontale.        
        if(left() < 0 || right() > wndWidth) velocity.x *= -1.f;
        
        // La stessa idea può essere applicata per le collisioni 
        // verticali.
        if(top() < 0 || bottom() > wndHeight) velocity.y *= -1.f;        
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }
};

const sf::Color Ball::defColor{sf::Color::Red};

int main() 
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    
    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 3"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
            break;

        ball.update();
        ball.draw(window);

        window.display();
    }   

    return 0;
}