// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In questo segmento di codice finale aggiungeremo qualche feature
// al nostro gioco:
// * Testo informativo
// * Stati di vittoria/sconfitta (con vite limitate)
// * Mattoncini che richiedono più colpi per essere distrutti

#include <memory>
#include <typeinfo>
#include <map>
#include <SFML/Graphics.hpp>

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

class Entity
{
public:
    bool destroyed{false};

    virtual ~Entity() { }
    virtual void update() { } 
    virtual void draw(sf::RenderWindow& mTarget) { }
};

class Manager
{
private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::map<std::size_t, std::vector<Entity*>> groupedEntities;

public:
    template<typename T, typename... TArgs> 
    T& create(TArgs&&... mArgs)
    {
        static_assert(std::is_base_of<Entity, T>(), 
            "`T` must derive from `Entity`");

        auto uPtr(std::make_unique<T>(std::forward<TArgs>(mArgs)...));
        auto ptr(uPtr.get());
        groupedEntities[typeid(T).hash_code()].emplace_back(ptr);
        entities.emplace_back(std::move(uPtr));

        return *ptr;
    }   

    void refresh()
    {
        for(auto& pair : groupedEntities)
        {
            auto& vector(pair.second);

            vector.erase(
                std::remove_if(std::begin(vector), std::end(vector), 
                [](auto mPtr){ return mPtr->destroyed; }), 
                std::end(vector));
        }

        entities.erase(
            std::remove_if(std::begin(entities), std::end(entities), 
            [](const auto& mUPtr){ return mUPtr->destroyed; }), 
            std::end(entities));
    }

    void clear() 
    { 
        groupedEntities.clear(); 
        entities.clear(); 
    }
    
    template<typename T> auto& getAll() 
    { 
        return groupedEntities[typeid(T).hash_code()]; 
    }

    template<typename T, typename TFunc> 
    void forEach(TFunc mFunc)
    {
        for(auto ptr : getAll<T>()) 
            mFunc(*reinterpret_cast<T*>(ptr));
    }

    void update()                           
    { 
        for(auto& e : entities) e->update(); 
    }
    void draw(sf::RenderWindow& mTarget)    
    { 
        for(auto& e : entities) e->draw(mTarget); 
    }
};

struct Rectangle
{
    sf::RectangleShape shape;

    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float width() const noexcept    { return shape.getSize().x; }
    float height() const noexcept   { return shape.getSize().y; }
    float left() const noexcept     { return x() - width() / 2.f; }
    float right() const noexcept    { return x() + width() / 2.f; }
    float top() const noexcept      { return y() - height() / 2.f; }
    float bottom() const noexcept   { return y() + height() / 2.f; }
};

struct Circle
{
    sf::CircleShape shape;

    float x() const noexcept        { return shape.getPosition().x; }
    float y() const noexcept        { return shape.getPosition().y; }
    float radius() const noexcept   { return shape.getRadius(); }
    float left() const noexcept     { return x() - radius(); }
    float right() const noexcept    { return x() + radius(); }
    float top() const noexcept      { return y() - radius(); }
    float bottom() const noexcept   { return y() + radius(); }
};

class Ball : public Entity, public Circle
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f}, defVelocity{8.f};

    sf::Vector2f velocity{-defVelocity, -defVelocity};

    Ball(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    void update() override
    {
        shape.move(velocity);
        solveBoundCollisions();
    }

    void draw(sf::RenderWindow& mTarget) override 
    { 
        mTarget.draw(shape);
    }

private:
    void solveBoundCollisions() noexcept
    {
        if(left() < 0 || right() > wndWidth) velocity.x *= -1.f;        
        
        if(top() < 0) velocity.y *= -1.f;
        
        // Se la pallina ha lasciato la finestra in basso, deve
        // essere distrutta.
        else if(bottom() > wndHeight) destroyed = true;           
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

class Paddle : public Entity, public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{75.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::Vector2f velocity;

    Paddle(float mX, float mY) 
    { 
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() override
    {
        processPlayerInput();
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget) override 
    { 
        mTarget.draw(shape); 
    }

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

class Brick : public Entity, public Rectangle
{
public:
    static const sf::Color defColorHits1;
    static const sf::Color defColorHits2;
    static const sf::Color defColorHits3;
    static constexpr float defWidth{60.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    // Aggiungiamo un campo per il numero di colpi richiesti.
    int requiredHits{1};

    Brick(float mX, float mY) 
    { 
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});       
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() override
    {
        // Alteriamo il colore del mattoncino in base al numero di
        // colpi richiesti.
        if(requiredHits == 1) shape.setFillColor(defColorHits1);
        else if(requiredHits == 2) shape.setFillColor(defColorHits2);
        else shape.setFillColor(defColorHits3);
    }
    void draw(sf::RenderWindow& mTarget) override 
    { 
        mTarget.draw(shape); 
    }
};

const sf::Color Brick::defColorHits1{255, 255, 0, 80};
const sf::Color Brick::defColorHits2{255, 255, 0, 170};
const sf::Color Brick::defColorHits3{255, 255, 0, 255};

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
    
    // Invece di distruggere immediatemente il mattoncino,
    // decrementiamo prima la sua "vita".
    --mBrick.requiredHits;
    if(mBrick.requiredHits <= 0) mBrick.destroyed = true;

    auto overlapLeft(mBall.right() - mBrick.left());
    auto overlapRight(mBrick.right() - mBall.left());
    auto overlapTop(mBall.bottom() - mBrick.top());
    auto overlapBottom(mBrick.bottom() - mBall.top());

    auto bFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));
    auto bFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

    auto minOverlapX(bFromLeft ? overlapLeft : overlapRight);
    auto minOverlapY(bFromTop ? overlapTop : overlapBottom);

    if(std::abs(minOverlapX) < std::abs(minOverlapY))   
        mBall.velocity.x = std::abs(mBall.velocity.x) * (bFromLeft ? -1.f : 1.f);    
    else                                         
        mBall.velocity.y = std::abs(mBall.velocity.y) * (bFromTop ? -1.f : 1.f);    
}

class Game
{
private:
    // Aggiungiamo due stati aggiuntivi: `GameOver` e `Victory`.
    enum class State{Paused, GameOver, InProgress, Victory};
    
    static constexpr int brkCountX{11}, brkCountY{4};       
    static constexpr int brkStartCol{1}, brkStartRow{2};     
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};   

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 11"};
    Manager manager;

    // SFML offre delle classi `sf::Font` ed `sf::Text` molto facili
    // da usare. Le impiegheremo per mostrare il numero di vite 
    // rimanenti e lo stato del gioco.
    sf::Font liberationSans;
    sf::Text textState, textLives;

    State state{State::GameOver};
    bool pausePressedLastFrame{false};
    
    // Teniamo traccia delle vite del player nella classe `Game`.
    int remainingLives{0};

public:
    Game() 
    { 
        window.setFramerateLimit(60); 

        // E' necessario caricare un font da file prima di poter
        // usare i nostri oggetti di tipo `sf::Text`.
        liberationSans.loadFromFile(
            R"(/usr/share/fonts/TTF/LiberationSans-Regular.ttf)");

        textState.setFont(liberationSans);
        textState.setPosition(10, 10);
        textState.setCharacterSize(35.f);
        textState.setColor(sf::Color::White);
        textState.setString("Paused");

        textLives.setFont(liberationSans);
        textLives.setPosition(10, 10);
        textLives.setCharacterSize(15.f);
        textLives.setColor(sf::Color::White);
    }

    void restart()
    {
        // Ricordiamoci di settare le vite all'inizio di `restart`.
        remainingLives = 3;

        state = State::Paused;
        manager.clear();

        for(int iX{0}; iX < brkCountX; ++iX)    
            for(int iY{0}; iY < brkCountY; ++iY)        
            {
                auto x((iX + brkStartCol) 
                    * (Brick::defWidth + brkSpacing));
                auto y((iY + brkStartRow) 
                    * (Brick::defHeight + brkSpacing));

                auto& brick(manager.create<Brick>(brkOffsetX + x, y));

                // Settiamo il numero di colpi richiesti per la
                // distruzione dei mattoncini usando un pattern
                // periodico.
                brick.requiredHits = 1 + ((iX * iY) % 3);
            }

        manager.create<Ball>(wndWidth / 2.f, wndHeight / 2.f);
        manager.create<Paddle>(wndWidth / 2, wndHeight - 50);
    }

    void run()
    {
        while(true)
        {
            window.clear(sf::Color::Black);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
                break;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                if(!pausePressedLastFrame) 
                {
                    if(state == State::Paused) 
                        state = State::InProgress;
                    else if(state == State::InProgress) 
                        state = State::Paused;
                }
                pausePressedLastFrame = true;
            }               
            else pausePressedLastFrame = false;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) 
                restart();

            // Se il gioco non è "in progress", non renderizziamo o 
            // aggiorniamo gli elementi, e mostriamo al player lo
            // stato corrente con una stringa.
            if(state != State::InProgress)
            {
                if(state == State::Paused) 
                    textState.setString("Paused");
                else if(state == State::GameOver) 
                    textState.setString("Game over!");
                else if(state == State::Victory) 
                    textState.setString("You won!");

                window.draw(textState);         
            }
            else
            {
                // Se non ci sono più palline sullo schermo,
                // decrementiamo il numero di vite e creiamo una
                // nuova pallina al centro della finestra.
                if(manager.getAll<Ball>().empty())
                {
                    manager.create<Ball>(wndWidth / 2.f, 
                        wndHeight / 2.f);
                    
                    --remainingLives;
                }

                // Se non ci sono più mattoncini sullo schermo, il
                // player ha vinto!
                if(manager.getAll<Brick>().empty()) 
                    state = State::Victory;

                // Se il giocatore non ha più vite rimanenti, 
                // è "game over"!
                if(remainingLives <= 0) state = State::GameOver;

                manager.update();

                manager.forEach<Ball>([this](auto& mBall)
                {
                    manager.forEach<Brick>([&mBall](auto& mBrick)
                    {
                        solveBrickBallCollision(mBrick, mBall);
                    });
                    manager.forEach<Paddle>([&mBall](auto& mPaddle)
                    {
                        solvePaddleBallCollision(mPaddle, mBall);
                    });
                });

                manager.refresh();
            
                manager.draw(window);

                // Aggiorniamo il testo delle vite rimanenti e
                // renderizziamolo.
                textLives.setString("Lives: " 
                    + std::to_string(remainingLives));
                
                window.draw(textLives);
            }

            window.display();
        }   
    }
};

int main() 
{
    Game game; game.restart(); game.run();
    return 0;
}