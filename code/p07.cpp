// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Per completare la logica del nostro gioco dobbiamo controllare
// e rispondere alle collisioni "pallina vs mattoncino".

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

class Brick
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::RectangleShape shape;
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

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }
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

// Ecco la funzione che gestisce le collisioni pallina-mattoncino.
// Dobbiamo capire da che direzione è stato colpito il mattoncino
// per rispondere in maniera corretta.
void solveBrickBallCollision(Brick& mBrick, Ball& mBall) noexcept
{
    // Se non c'è alcuna intersezione, usciamo dalla funzione.
    if(!isIntersecting(mBrick, mBall)) return;

    // Altrimenti, il mattoncino è stato colpito!
    mBrick.destroyed = true;

    // Calcoliamo "quanto la pallina interseca" il mattoncino
    // in tutte le direzioni.
    // {Info: ball vs brick collision}
    auto overlapLeft(mBall.right() - mBrick.left());
    auto overlapRight(mBrick.right() - mBall.left());
    auto overlapTop(mBall.bottom() - mBrick.top());
    auto overlapBottom(mBrick.bottom() - mBall.top());

    // Se la magnitudine dell'overlap a sinistra è minore di quella
    // dell'overlap a destra, siamo sicuri che la pallina è arrivata da
    // sinistra.
    auto bFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));

    // La stessa idea viene applicata per gli overlap verticali.
    auto bFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

    // Adesso conserviamo gli overlap minimi per entrambi gli assi.
    auto minOverlapX(bFromLeft ? overlapLeft : overlapRight);
    auto minOverlapY(bFromTop ? overlapTop : overlapBottom);

    // Se la magnitudine dell'overlap orizzontale è minore di quella
    // verticale, allora la pallina ha colpito il mattoncino
    // orizzontalmente.

    // In base all'overlap minore ed alla direzione in cui la
    // pallina ha colpito il mattoncino, cambiamo il segno della
    // velocità X o Y della pallina.
    if(std::abs(minOverlapX) < std::abs(minOverlapY))
    {
        mBall.velocity.x =
            std::abs(mBall.velocity.x) * (bFromLeft ? -1.f : 1.f);
    }
    else
    {
        mBall.velocity.y = std::abs(mBall.velocity.y) * (bFromTop ? -1.f : 1.f);
    }
}

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};
    std::vector<Brick> bricks;

    constexpr int brkCountX{11};
    constexpr int brkCountY{4};
    constexpr int brkStartColumn{1};
    constexpr int brkStartRow{2};
    constexpr float brkSpacing{3.f};
    constexpr float brkOffsetX{22.f};

    for(int iX{0}; iX < brkCountX; ++iX)
        for(int iY{0}; iY < brkCountY; ++iY)
        {
            auto x((iX + brkStartColumn) * (Brick::defWidth + brkSpacing));
            auto y((iY + brkStartRow) * (Brick::defHeight + brkSpacing));

            bricks.emplace_back(brkOffsetX + x, y);
        }

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 7"};
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

            // Testiamo eventuali collisioni con ogni mattoncino.
            solveBrickBallCollision(brick, ball);
        }

        // Dopo aver testato le collisioni, è possibile che alcuni
        // mattoncini siano marcati come "destroyed".

        // Per rimuovere dal gioco tali mattoncini, utilizzeremo
        // l'"erase-remove idiom", che permette la rimozione
        // efficiente di oggetti da un "container" che soddisfano
        // un predicato. Tale predicato sarà specificato con una
        // lambda generica (C++14).

        // `std::remove_if` ordina gli elementi di un contenitore in
        // modo che quelli da rimuovere siano tutti alla sua fine.
        // Restituisce un iteratore che punta al primo elemento
        // da rimuovere.

        // Chiamando `std::vector::erase` con l'iteratore restituito
        // e l'iteratore finale del contenitore, rimuoviamo tutti i
        // mattoncini marcati come "destroyed".
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