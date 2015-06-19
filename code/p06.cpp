// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Adesso implementiamo l'ultimo elemento fondamentale del 
// nostro gioco: i mattoncini (bricks).
// In questo segmento di codice definiremo una classe `Brick`
// e faremo apparire una griglia di mattoncini sulla finestra.

#include <SFML/Graphics.hpp>

// Spostiamo le funzioni di utility all'inizio del codice.

template<typename T>
auto getLength(const T& mVec) noexcept
{
    return std::sqrt(std::pow(mVec.x, 2) + std::pow(mVec.y, 2));
}

template<typename T>
auto getNormalized(const sf::Vector2<T>& mVec) noexcept
{
    return mVec / static_cast<T>(getLength(mVec));
}

template<typename T1, typename T2>
auto getDotProduct(const T1& mVec1, const T2& mVec2)
{
    return mVec1.x * mVec2.x + mVec1.y * mVec2.y;    
}

template<typename T1, typename T2>
auto getReflected(const T1& mVec, const T2& mNormal)
{
    return mVec - (mNormal * (2.f * getDotProduct(mVec, mNormal)));
}

template<typename T1, typename T2> 
bool isIntersecting(const T1& mA, const T2& mB) noexcept
{
    return  mA.right() >= mB.left() && mA.left() <= mB.right() 
            && mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

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

struct Paddle
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

// La classe `Brick` sarà molto simile a quella `Paddle`, dato
// che entrambe contengono una forma rettangolare.
class Brick
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};

    sf::RectangleShape shape;   

    // Dobbiamo tenere traccia dello stato di ogni mattoncino.
    // Quando sarà colpito dalla pallina, setteremo un `bool` a
    // `true`. Nel game loop poi distruggeremo tutti i mattoncini
    // marcati.
    bool destroyed{false};

    Brick(float mX, float mY) 
    { 
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() { }
    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float width() const noexcept    { return shape.getSize().x; }
    float height() const noexcept   { return shape.getSize().y; }
    float left() const noexcept     { return x() - width() / 2.f; }
    float right() const noexcept    { return x() + width() / 2.f; }
    float top() const noexcept      { return y() - height() / 2.f; }
    float bottom() const noexcept   { return y() + height() / 2.f; }
};

const sf::Color Brick::defColor{sf::Color::Yellow};

void solvePaddleBallCollision(const Paddle& mPaddle, Ball& mBall) noexcept
{
    if(!isIntersecting(mPaddle, mBall)) return;

    auto newY(mPaddle.top() - mBall.shape.getRadius() * 2.f);
    mBall.shape.setPosition(mBall.x(), newY);

    auto paddleBallDiff(mBall.x() - mPaddle.x());
    auto posFactor(paddleBallDiff / mPaddle.width());
    auto velFactor(mPaddle.velocity.x * 0.05f);
    
    sf::Vector2f collisionVec{posFactor + velFactor, -2.f};    
    mBall.velocity = getReflected(mBall.velocity, getNormalized(collisionVec));   
}

int main() 
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};

    // Abbiamo bisogno di istanze multiple di `Brick`.
    // Usiamo `std::vector`.
    std::vector<Brick> bricks;

    // Definiamo anche alcune costanti che ci aiuteranno a 
    // costruire il "grid-pattern" dei mattoncini.

    constexpr int brkCountX{11};      // Numero di colonne.
    constexpr int brkCountY{4};       // Numero di righe.
    constexpr int brkStartCol{1};     // Colonna di inizio.
    constexpr int brkStartRow{2};     // Riga di inizio.
    constexpr float brkSpacing{3};    // Spazio tra due mattoncini.
    constexpr float brkOffsetX{22.f}; // Offset X del pattern.

    // Sappiamo quanti mattoncini andremo a creare - questa è
    // un'opportunità per preallocare la memoria necessaria.
    bricks.reserve(brkCountX * brkCountY);

    // Riempiamo il nostro vector con un for-loop bidimensionale.
    for(int iX{0}; iX < brkCountX; ++iX)    
        for(int iY{0}; iY < brkCountY; ++iY)        
        {
            auto x((iX + brkStartCol) * (Brick::defWidth + brkSpacing));
            auto y((iY + brkStartRow) * (Brick::defHeight + brkSpacing));

            bricks.emplace_back(brkOffsetX + x, y); 
        }

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 6"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
            break;

        ball.update();
        paddle.update();
        
        // L'unico "pezzo mancante" è chiamare `update()` e `draw()`
        // su ogni mattoncino del nostro vector.
        for(auto& brick : bricks) brick.update();

        solvePaddleBallCollision(paddle, ball);

        ball.draw(window);
        paddle.draw(window);
        for(auto& brick : bricks) brick.draw(window);

        window.display();
    }   

    return 0;
}