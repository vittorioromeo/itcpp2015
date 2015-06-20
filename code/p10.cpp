// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Il prossimo step che migliorerà notevolmente l'architettura
// del codice è creare un sistema che ci permetta di creare
// velocemente nuovi tipi di game object durante lo sviluppo,
// ma anche istanziare e distruggere più facilmente le instanze
// di tali oggetti a run-time.

// Necessiteremo di una gerarchia polimorfica per i nostri game
// object, e di una classe `Manager` per semplificare la loro 
// gestione.

// Utilizzeremo gli "smart pointers", contenuti nell'header 
// `<memory>`, introdotto nello standard C++11.
#include <memory>

// Useremo anche gli header `<typeinfo>` e `<map>` per organizzare
// i game object in base al loro tipo.
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

// Tutti i game object condivideranno la stessa interfaccia.
// Forniranno i metodi `update` e `draw`.
// Creiamo una classe `Entity`, che sarà alla base della
// nostra gerarchia. Conterrà anche un `bool` chiamato
// "destroyed" per marcare le entità da distruggere.
class Entity
{
public:
    bool destroyed{false};

    // Ricordiamoci la keyword `virtual` per abilitare il
    // polimorfismo.
    virtual ~Entity() { }
    virtual void update() { } 
    virtual void draw(sf::RenderWindow& mTarget) { }
};

// Creiamo una classe `Manager` per creare/distruggere le
// entità di gioco e fare "query" in base al loro tipo.
class Manager
{
private:
    // Dato che le nostre entità adesso sono polimorfiche, 
    // dobbiamo conservarle nel "free store". Utilizzeremo un
    // `std::vector` di `std::unique_ptr` per contenerle.
    std::vector<std::unique_ptr<Entity>> entities;

    // Avremo anche bisogno di trovare tutte le entità di un
    // tipo specifico a run-time. Invece di controllare manualmente
    // il tipo delle entità nel game loop, creiamo una specie
    // di "database" che conserva le entità in base al loro tipo.
    // Utilizzermo una `std::map`: le chiavi saranno hash
    // `typeid` dei game object, i valori saranno `std::vector`
    // di "raw pointer".
    std::map<std::size_t, std::vector<Entity*>> groupedEntities;

public:
    // Per popolare/interrogare correttamente queste strutture dati,
    // definiremo alcuni metodi helper. Usando "variadic templates",
    // introdotte nel C++11, permetteremo all'utente di creare entità
    // direttamente tramite il manager.

    // Il primo metodo che definiremo è `create`: dato un tipo `T`,
    // e un "variadic pack" di argomenti, esso costruirà un entità
    // di tipo `T`, la conserverà e restituirà una reference ad essa.
    template<typename T, typename... TArgs> 
    T& create(TArgs&&... mArgs)
    {
        // Controlliamo che `T` sia effettivamente parte della
        // gerarchia di game object con uno `static_assert`.
        static_assert(std::is_base_of<Entity, T>(), 
            "`T` must derive from `Entity`");

        // Adesso creiamo l'oggetto, usando `std::make_unique`. 
        // Utilizzeremo il "perfect forwarding" per passare 
        // correttamente i tipi dei parametri variadici al 
        // costruttore di `T`.
        auto uPtr(std::make_unique<T>(std::forward<TArgs>(mArgs)...));

        // Conserviamo il "raw pointer" all'oggetto appena creato.
        auto ptr(uPtr.get());

        // Calcoliamo l'hash per il tipo `T` usando la keyword 
        // `typeid`. Lo utilizzeremo come chiave per il nostro
        // "database" di oggetti.
        groupedEntities[typeid(T).hash_code()].emplace_back(ptr);

        // Attenzione:
        // `hash_code()` non garantisce che due tipi diversi abbiano
        // hash diversi. Ci sono varie soluzioni a questo problema.

        // Adesso "muoviamo" l'`unique_ptr` dentro il vettore.
        // Il manager sarà adesso il "proprietario" della memoria
        // di questo game object.
        entities.emplace_back(std::move(uPtr));

        return *ptr;
    }   

    // La rimozione delle entità funzionerà in questo modo: invece 
    // di rimuovere direttamente un'entità dallo storage, la 
    // marcheremo solamente come "destroyed". Il metodo `refresh` si
    // occuperà di "ripulire" tutte le entità marcate in un colpo
    // solo, alla fine di un `update`. Ciò migliora la performance e
    // ci permette di accedere ad entità prossime alla distruzione 
    // durante il resto del game loop.
    void refresh()
    {
        // Iniziamo a ripulire `groupedEntities`. 
        // Useremo l'"erase-remove idiom" su ogni suo vettore.
        for(auto& pair : groupedEntities)
        {
            auto& vector(pair.second);

            vector.erase(
                std::remove_if(std::begin(vector), std::end(vector), 
                [](auto mPtr){ return mPtr->destroyed; }), 
                std::end(vector)
            );
        }

        // Dopodichè faremo lo stesso per `entities`. Dato che esso 
        // contiene smart pointers, la memoria allocata per le 
        // entità distrutte verrà liberata automaticamente.
        entities.erase(
            std::remove_if(std::begin(entities), std::end(entities), 
            [](const auto& mUPtr){ return mUPtr->destroyed; }),
            std::end(entities)
        );
    }

    // Ci servirà anche un metodo `clear` per ripulire il manager.
    void clear() 
    { 
        groupedEntities.clear(); 
        entities.clear(); 
    }

    // Specificando il tipo `T` e chiamando `getAll`, l'utente
    // otterrà tutte le entità di quel tipo.
    template<typename T> auto& getAll() 
    { 
        return groupedEntities[typeid(T).hash_code()]; 
    }

    // Un metodo utilissimo sarà `forEach`, che ripeterà una funzione
    // passata dall'utente su ogni entità di tipo `T`, "castandola"
    // automaticamente.
    template<typename T, typename TFunc> 
    void forEach(TFunc mFunc)
    {
        auto& vector(getAll<T>());

        // Ogni puntatore nel vettore di "raw pointer" sarà castato
        // al suo "vero" tipo, e passato ad `mFunc` come parametro,
        // dereferenziato.
        for(auto ptr : vector) mFunc(*static_cast<T*>(ptr));
    }
    
    // Implementeremo infine metodi per aggiornare e renderizzare
    // tutte le entità.

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

// Adattiamo le nostre classi alla nuova architettura.

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

    // La keyword C++11 `override` controlla se stiamo veramente
    // re-implementando un metodo virtuale.
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
        if(top() < 0 || bottom() > wndHeight) velocity.y *= -1.f;
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
    static const sf::Color defColor;
    static constexpr float defWidth{60.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    Brick(float mX, float mY) 
    { 
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void draw(sf::RenderWindow& mTarget) override 
    { 
        mTarget.draw(shape); 
    }
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
        mBall.velocity.x = std::abs(mBall.velocity.x) * (bFromLeft ? -1.f : 1.f);    
    else                                         
        mBall.velocity.y = std::abs(mBall.velocity.y) * (bFromTop ? -1.f : 1.f);    
}

class Game
{
private:
    enum class State{Paused, InProgress};
    
    static constexpr int brkCountX{11}, brkCountY{4};       
    static constexpr int brkStartCol{1}, brkStartRow{2};     
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};   

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 10"};

    // Un'istanza di `Manager` sarà contenuta dentro `Game`.
    Manager manager;

    State state{State::InProgress};
    bool pausePressedLastFrame{false};  

public:
    Game() { window.setFramerateLimit(60); }

    void restart()
    {
        state = State::Paused;
        
        // Durante il restart, ripuliremo il manager.
        manager.clear();

        for(int iX{0}; iX < brkCountX; ++iX)    
            for(int iY{0}; iY < brkCountY; ++iY)        
            {
                auto x((iX + brkStartCol) 
                    * (Brick::defWidth + brkSpacing));
                auto y((iY + brkStartRow) 
                    * (Brick::defHeight + brkSpacing));
                
                // Creare game object attraverso il manager è 
                // veramente semplice e conveniente:
                manager.create<Brick>(brkOffsetX + x, y);
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

            if(state != State::Paused)
            {
                // Invece di manualmente aggiornare ogni entità, 
                // lasciamo fare questo lavoro al manager.
                manager.update();
                
                // La logica di gioco è molto più generica adesso: 
                // basta chiedere al manager di restituirci tutte le
                // entità di un tipo specifico per gestire le 
                // interazioni tra di esse.
                // Aggiungere nuovi tipi di game object non sarà
                // un problema in futuro.
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

                // Dopo gli `update`, non dimentichiamoci di 
                // chiamare `refresh` per distruggere le entità
                // marcate "destroyed".
                manager.refresh();
            }
            
            manager.draw(window);
            window.display();
        }   
    }
};


int main() 
{
    Game game; game.restart(); game.run();
    return 0;
}

// Il codice è adesso molto più complesso, ma i vantaggi offerti 
// allo sviluppatore da questo tipo di architettura sono davvero 
// notevoli.

// Utilizzare un manager per gestire le entità semplifica 
// l'eventuale aggiunta o rimozione di tipi di game object,
// e permette al developer di scrivere codice più pulito ed
// intuitivo.

// Nel prossimo (ed ultimo) segmento di codice, aggiungeremo dei
// "tocchi finali" (opzionali) al nostro gioco.