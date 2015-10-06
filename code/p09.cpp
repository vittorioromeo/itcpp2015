// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Il primo step verso una migliore architettura è definire
// una classe `Game` che "encapsuli" lo stato del gioco.

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

// La classe `Game` conterrà costanti ed elementi di gioco e
// terrà traccia del suo stato. Fornirà anche funzioni per mettere
// in pausa e ricominciare il gioco.
class Game
{
private:
    // Creiamo una `enum class` con i possibili stati del gioco.
    enum class State
    {
        Paused,
        InProgress
    };

    static constexpr int brkCountX{11}, brkCountY{4};
    static constexpr int brkStartCol{1}, brkStartRow{2};
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 9"};

    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};
    std::vector<Brick> bricks;

    // La classe terrà traccia dello stato con il campo `state`.
    State state{State::InProgress};

    // Per evitare che il gioco venga messo e tolto dalla pausa
    // più volte durante la pressione del tasto pausa, usiamo
    // questo campo per "ricordarci" se il tasto era pressato nel
    // frame precedente.
    bool pausePressedLastFrame{false};

public:
    Game() { window.setFramerateLimit(60); }

    // Il metodo `restart` ricostruirà tutti i game object e
    // porterà il gioco allo stato iniziale.
    void restart()
    {
        state = State::Paused;

        for(int iX{0}; iX < brkCountX; ++iX)
            for(int iY{0}; iY < brkCountY; ++iY)
            {
                auto x((iX + brkStartCol) * (Brick::defWidth + brkSpacing));
                auto y((iY + brkStartRow) * (Brick::defHeight + brkSpacing));

                bricks.emplace_back(brkOffsetX + x, y);
            }

        ball = Ball{wndWidth / 2.f, wndHeight / 2.f};
        paddle = Paddle{wndWidth / 2, wndHeight - 50};
    }

    // Il metodo `run` farà partire il game loop.
    void run()
    {
        while(true)
        {
            window.clear(sf::Color::Black);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

            // Il tasto `P` gestirà la pausa.
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                // Prima di mettere/togliere la pausa, controlliamo
                // se il tasto era già stato pressato.
                if(!pausePressedLastFrame)
                {
                    if(state == State::Paused)
                        state = State::InProgress;
                    else if(state == State::InProgress)
                        state = State::Paused;
                }

                pausePressedLastFrame = true;
            }
            else
                pausePressedLastFrame = false;

            // Il tasto `R` farà ricominciare il gioco.
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) restart();

            // Se il gioco è in pausa, non aggiorneremo i game
            // object.
            if(state != State::Paused)
            {
                ball.update();
                paddle.update();
                for(auto& brick : bricks)
                {
                    brick.update();
                    solveBrickBallCollision(brick, ball);
                }

                bricks.erase(
                    std::remove_if(std::begin(bricks), std::end(bricks),
                        [](const auto& mBrick)
                        {
                            return mBrick.destroyed;
                        }),
                    std::end(bricks));

                solvePaddleBallCollision(paddle, ball);
            }

            ball.draw(window);
            paddle.draw(window);
            for(auto& brick : bricks) brick.draw(window);

            window.display();
        }
    }
};

int main()
{
    // Per far partire il gioco, basta instanziare `Game`
    // e chiamare `restart` seguito da `run`.
    Game game;
    game.restart();
    game.run();
    return 0;
}