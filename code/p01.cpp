// Copyright (c) 2015 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Iniziamo lo sviluppo del nostro clone di arkanoid.

// La prima cosa che faremo sarà creare una finestra vuota 
// utilizzando la libreria SFML. La finestra gestirà l'input
// e sarà il target di rendering per la grafica del gioco.

// Il modulo <SFML/Graphics.hpp> è richiesto per gestire la grafica.
// Include moduli della STL comunemente usati ed anche l'header
// <SFML/Window.hpp>, che gestisce la creazione delle finestre.
#include <SFML/Graphics.hpp>

// Definiamo alcune costanti per la finestra.
// Usiamo la keyword `constexpr` per rendere chiara la nostra
// intenzione che questi valori non cambieranno durante l'esecuzione
// del programma.
constexpr unsigned int wndWidth{800}, wndHeight{600};

int main()  
{
    // Nel `main` creeremo la finestra.

    // La classe SFML che rappresenta una finestra è chiamata
    // `sf::RenderWindow`, ed il suo costruttore richiede come 
    // parametri un `sf::Vector2u` (grandezza della finestra), ed
    // una `std::string` (titolo della finestra).
    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 1"};

    // `sf::RenderWindow` deriva da `sf::RenderTarget` - può quindi
    // essere utilizzata come bersaglio di rendering.

    // Invece di specificare in maniera esplicita `sf::Vector2u`,
    // abbiamo utilizzato la {...} "uniform initialization syntax".

    // Diamo un limite di 60 al "framerate" del gioco, garantendo
    // che la logica dell'applicazione sia uguale su ogni macchina.
    window.setFramerateLimit(60);
    
    // Lo step seguente è tenere "in vita" la finestra.
    // Questo è uno dei compiti del cosidetto "game loop".
    // {Info: game loop}

    while(true)
    {
        // Ogni iterazione di questo loop è un "frame" del nostro 
        // gioco.
        // All'inizio di ogni iterazione bisogna "ripulire" il frame
        // dalla grafica renderizzata precedentemente, utilizzando
        // `sf::RenderWindow::clear`.
        window.clear(sf::Color::Black);

        // Successivamente controlleremo lo stato dell'input. Se il
        // giocatore ha pressato "Escape", utilizzeremo `break` per 
        // uscire fuori dal game loop e terminare il programma.
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) 
            break;

        // Al termine del frame, usiamo `sf::RenderWindow::display` 
        // per mostrare i contenuti renderizzati sulla finestra.
        // La tecnica utilizzata è quella del "double buffering".
        window.display();
    }   

    return 0;
}