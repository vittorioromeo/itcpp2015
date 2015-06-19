// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In questo segmento di codice inizieremo ad implementare il
// primo "game object": la pallina. Creeremo una classe per 
// lei e impareremo ad usare le classi "shape" di SFML per 
// renderizzarla e muoverla.

#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

// Definiamo una classe che rappresenti l'entità pallina.
// La classe avrà il compito di gestire sia la logica che la
// grafica della pallina.
class Ball
{
public:
    // Definiamo alcune costanti statiche. Usiamo `constexpr`
    // dove possibile.
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{1.f};

    // `sf::CircleShape` è una classe SFML che rappresenta una
    // forma circolare. Specificando un raggio ed una posizione,
    // è possibile renderizzare tale forma su un "render target".
    sf::CircleShape shape;

    // Avremo bisogno di un vettore bidimensionale per conservare
    // e modificare la velocità corrente della pallina.
    // Sarà inizializzato con la velocità di default.
    sf::Vector2f velocity{-defVelocity, -defVelocity};

    // Costruttore: prende come parametri la posizione iniziale
    // della pallina, sotto forma di due `float`.
    Ball(float mX, float mY)
    {
        // SFML usa un sistema di coordinate avente l'origine
        // posizionata nell'angolo in alto a sinistra della 
        // finestra.
        // {Info: coordinate system}

        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    // Nel nostro design ogni "game object" avrà un metodo `update`
    // ed un metodo `draw`.

    // Il metodo `update` gestirà la logica dell'entità di gioco.

    // Il metodo `draw` renderizzerà l'entità di gioco su schermo.
    // Prenderà in input una reference ad una `sf::RenderWindow`,
    // la quale sarà il nostro target di rendering.

    void update()
    {
        // Le classi shape di SFML hanno un metodo `move` che prende
        // come parametro un vettore `float` di offset.
        // {Info: ball movement}
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget)
    {
        // Nel metodo `draw` sarà sufficiente "chiedere" alla
        // finestra di renderizzare la forma.
        mTarget.draw(shape);
    }
};

// I membri statici non-`constexpr` devono essere inizializzati
// fuori dalla definizione della classe.
const sf::Color Ball::defColor{sf::Color::Red};

int main() 
{
    // Creiamo un'istanza di `Ball`, posizionata al centro della
    // finestra.
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    
    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 2"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
            break;

        // Nel game loop, dobbiamo ricordarci di chiamare `update`  
        // e `draw` per tutte le entità di gioco.
        ball.update();
        ball.draw(window);

        window.display();
    }   

    return 0;
}