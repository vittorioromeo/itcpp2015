// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Prima di completare il nostro gioco, è bene fare un pò
// di "refactoring" sul nostro codice. C'è molta duplicazione
// che si può evitare!

#include <SFML/Graphics.hpp>

template <typename T>
auto getLength(const T& mVec) noexcept
{
    return std::sqrt(std::pow(mVec.x, 2) + std::pow(mVec.y, 2));
}

template <typename T>
auto getNormalized(const sf::Vector2<T>& mVec) noexcept
{
    return mVec / static_cast<T>(getLength(mVec));
}

template <typename T1, typename T2>
auto getDotProduct(const T1& mVec1, const T2& mVec2)
{
    return mVec1.x * mVec2.x + mVec1.y * mVec2.y;
}

template <typename T1, typename T2>
auto getReflected(const T1& mVec, const T2& mNormal)
{
    return mVec - (mNormal * (2.f * getDotProduct(mVec, mNormal)));
}

template <typename T1, typename T2>
bool isIntersecting(const T1& mA, const T2& mB) noexcept
{
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
           mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

constexpr unsigned int wndWidth{800}, wndHeight{600};

// Per iniziare, evitiamo di ripetere i nostri semplici getter in
// classi con la stessa forma. Definiamo due classi da cui i nostri
// game object deriveranno: una per i rettangoli ed una per i cerchi.
struct Rectangle
{
    sf::RectangleShape shape;

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }
};

struct Circle
{
    sf::CircleShape shape;

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float radius() const noexcept { return shape.getRadius(); }
    float left() const noexcept { return x() - radius(); }
    float right() const noexcept { return x() + radius(); }
    float top() const noexcept { return y() - radius(); }
    float bottom() const noexcept { return y() + radius(); }
};

// Adesso adattiamo le nostre classi alla nuova gerarchia.

class Ball : public Circle
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{8.f};

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

private:
    void solveBoundCollisions() noexcept
    {
        if(left() < 0 || right() > wndWidth) velocity.x *= -1.f;
        if(top() < 0 || bottom() > wndHeight) velocity.y *= -1.f;
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

class Paddle : public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{75.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

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

private:
    void processPlayerInput()
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
            velocity.x = -defVelocity;
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) &&
                right() < wndWidth)
            velocity.x = defVelocity;
        else
            velocity.x = 0;
    }
};

const sf::Color Paddle::defColor{sf::Color::Red};

class Brick : public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    bool destroyed{false};

    Brick(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() {}
    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }
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

void solveBrickBallCollision(Brick& mBrick, Ball& mBall) noexcept
{
    if(!isIntersecting(mBrick, mBall)) return;
    mBrick.destroyed = true;

    auto overlapLeft(mBall.right() - mBrick.left());
    auto overlapRight(mBrick.right() - mBall.left());
    auto overlapTop(mBall.bottom() - mBrick.top());
    auto overlapBottom(mBrick.bottom() - mBall.top());

    auto bFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));
    auto bFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

    auto minOverlapX(bFromLeft ? overlapLeft : overlapRight);
    auto minOverlapY(bFromTop ? overlapTop : overlapBottom);

    if(std::abs(minOverlapX) < std::abs(minOverlapY))
        mBall.velocity.x =
            std::abs(mBall.velocity.x) * (bFromLeft ? -1.f : 1.f);
    else
        mBall.velocity.y = std::abs(mBall.velocity.y) * (bFromTop ? -1.f : 1.f);
}

int main()
{
    constexpr int brkCountX{11}, brkCountY{4};
    constexpr int brkStartCol{1}, brkStartRow{2};
    constexpr float brkSpacing{3.f}, brkOffsetX{22.f};

    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};
    std::vector<Brick> bricks;

    for(int iX{0}; iX < brkCountX; ++iX)
        for(int iY{0}; iY < brkCountY; ++iY)
        {
            auto x((iX + brkStartCol) * (Brick::defWidth + brkSpacing));
            auto y((iY + brkStartRow) * (Brick::defHeight + brkSpacing));

            bricks.emplace_back(brkOffsetX + x, y);
        }

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 8"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        ball.update();
        paddle.update();
        for(auto& brick : bricks)
        {
            brick.update();
            solveBrickBallCollision(brick, ball);
        }

        bricks.erase(std::remove_if(std::begin(bricks), std::end(bricks),
                         [](const auto& mBrick)
                         {
                             return mBrick.destroyed;
                         }),
            std::end(bricks));

        solvePaddleBallCollision(paddle, ball);

        ball.draw(window);
        paddle.draw(window);
        for(auto& brick : bricks) brick.draw(window);

        window.display();
    }

    return 0;
}

// Ignorando i commenti e leggermente modificando il formatting,
// abbiamo raggiunto un ottimo risultato: un clone giocabile di
// arkanoid fatto da zero, in poco più di 200 righe di codice.

// Possiamo però decisamente migliorare l'architettura del nostro
// gioco. Una buona idea per rendere il sistema più flessibile ed
// estensibile è creare un'entità base polimorfica dalla quale
// deriveranno i nostri game object. La definiremo, insieme ad una
// classe `Game` ed una classe `Manager`, nei prossimi segmenti di
// codice.

// {Info: class hierarchy}
// {Info: game architecture}