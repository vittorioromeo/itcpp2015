// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In questo segmento di codice ci occuperemo delle interazioni
// tra la pallina ed il paddle. Controlleremo eventuali collisioni
// e risponderemo ad esse in maniera adeguata.

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

    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float left() const noexcept     { return x() - shape.getRadius(); }
    float right() const noexcept    { return x() + shape.getRadius(); }
    float top() const noexcept      { return y() - shape.getRadius(); }
    float bottom() const noexcept   { return y() + shape.getRadius(); }

private:
    void solveBoundCollisions() noexcept
    {
        if(left() < 0 || right() > wndWidth) velocity.x *= -1.f;        
        if(top() < 0 || bottom() > wndHeight) velocity.y *= -1.f;     
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

class Paddle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{75.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::RectangleShape shape;
    sf::Vector2f velocity;

    Paddle(float mX, float mY) 
    { 
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update()
    {
        processPlayerInput();
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float width() const noexcept    { return shape.getSize().x; }
    float height() const noexcept   { return shape.getSize().y; }
    float left() const noexcept     { return x() - width() / 2.f; }
    float right() const noexcept    { return x() + width() / 2.f; }
    float top() const noexcept      { return y() - height() / 2.f; }
    float bottom() const noexcept   { return y() + height() / 2.f; }

private:
    void processPlayerInput()
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) 
            && left() > 0) velocity.x = -defVelocity;
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) 
            && right() < wndWidth) velocity.x = defVelocity;
        else velocity.x = 0;    
    }
};

const sf::Color Paddle::defColor{sf::Color::Red};

// Iniziamo definendo una function template che restituisce `true`
// quando due game object stanno collidendo.
// I tipi `T1` e `T2` devono avere i metodi `left()`, `right()`, 
// `top()`, `bottom()`.
// Possiamo quindi usare questa function template sia su `Ball` che
// su `Paddle`.
template<typename T1, typename T2> 
bool isIntersecting(const T1& mA, const T2& mB) noexcept
{
    // {Info: collisione AABB vs AABB}
    return mA.right() >= mB.left() 
        && mA.left() <= mB.right() 
        && mA.bottom() >= mB.top() 
        && mA.top() <= mB.bottom();
}

// Per calcolare un rimbalzo "realistico" della pallina dovremo
// utilizzare un pò di matematica vettoriale. 
// Definiamo alcune funzioni di utility che ci aiuteranno.

// `getLength` restituisce il modulo (lunghezza) di un vettore.
template<typename T>
auto getLength(const T& mVec) noexcept
{
    return std::sqrt(std::pow(mVec.x, 2) + std::pow(mVec.y, 2));
}

// `getNormalized` restituisce una copia del vettore passato,
// normalizzato con lunghezza unitaria.
template<typename T>
auto getNormalized(const sf::Vector2<T>& mVec) noexcept
{
    return mVec / static_cast<T>(getLength(mVec));
}

// `getDotProduct` restituisce il prodotto scalare tra due vettori.
template<typename T1, typename T2>
auto getDotProduct(const T1& mVec1, const T2& mVec2)
{
    return mVec1.x * mVec2.x + mVec1.y * mVec2.y;    
}

// `getReflected` restituisce il "riflesso" di un vettore su un 
// altro.
template<typename T1, typename T2>
auto getReflected(const T1& mVec, const T2& mNormal)
{
    auto val(2.f * getDotProduct(mVec, mNormal));
    return mVec - (mNormal * val);
}

// Definiamo anche una funzione che sarà eseguita ogni frame e
// gestirà le collisioni tra il paddle ed una pallina.
// La funzione controllerà la presenza di una collisione, e in
// tal caso la risolverà spingendo la pallina verso l'alto e
// nella direzione orizzontale di "riflesso sul paddle".
void solvePaddleBallCollision(const Paddle& mPaddle, Ball& mBall) noexcept 
{
    // Se non c'è alcuna intersezione, usciamo dalla funzione.
    if(!isIntersecting(mPaddle, mBall)) return;
    
    // Se è avvenuta una collisione, posizionamo la pallina appena
    // sopra il paddle, in modo da evitare un'altra intersezione
    // "fasulla" nel frame seguente.     
    auto newY(mPaddle.top() - mBall.shape.getRadius() * 2.f);
    mBall.shape.setPosition(mBall.x(), newY);

    // Dobbiamo adesso dirigere la pallina in base al punto in
    // cui il paddle è stato colpito.

    // Calcoleremo il nuovo vettore velocità della pallina in base
    // a due fattori:
    // * Il punto in cui il paddle è stato colpito (se il paddle 
    //   viene colpito vicino alle estremità, l'angolo di rimbalzo
    //   sarà più "acuto").
    // * La velocità del paddle: se il paddle è in movimento, la
    //   pallina sarà spinta verso tale direzione.

    // {Info: collisione pallina vs paddle}

    // Per gestire il primo fattore, necessitiamo di un valore che
    // vada da `-0.5` a `+0.5` in base al punto del paddle colpito.

    auto paddleBallDiff(mBall.x() - mPaddle.x());
    auto posFactor(paddleBallDiff / mPaddle.width());

    // Per gestire il secondo fattore, basterà moltiplicare la 
    // velocità X del paddle per un numero abbastanza piccolo.

    auto velFactor(mPaddle.velocity.x * 0.05f);

    // Calcoliamo il vettore su cui rimbalzerà la pallina:
    sf::Vector2f collisionVec{posFactor + velFactor, -2.f};    
    
    // Modifichiamo la velocità della pallina usando `getReflected`.
    mBall.velocity = 
        getReflected(mBall.velocity, getNormalized(collisionVec));   
}

int main() 
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 5"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
            break;

        ball.update();
        paddle.update();

        // Dopo aver chiamato `update()` sulla pallina e sul paddle,
        // controlliamo e risolviamo eventuali collisioni.
        solvePaddleBallCollision(paddle, ball);

        ball.draw(window);
        paddle.draw(window);

        window.display();
    }   

    return 0;
}