#include "../include/chip8.h"
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>

using namespace std::literals::chrono_literals;

struct FPS
{
    double accumulator{ 0 };
    int frames{ 0 };
    double average{};
};

int main()
{   
    std::cout << "===== Select your ROM =====\n"
              << "1 - IBM Logo.ch8\n"
              << "2 - Airplane.ch8\n"
              << "3 - Cave.ch8\n"
              << "4 - Cavern.ch8\n"
              << "5 - Pong (1 player).ch8\n"
              << "===========================\n";

    int choice{};
    std::cin >> choice;
    std::string filename{};

    switch (choice)
    {
    case 1: filename = "IBM Logo.ch8";        break;
    case 2: filename = "Airplane.ch8";        break;
    case 3: filename = "Cave.ch8";            break;
    case 4: filename = "cavern.ch8";          break;
    case 5: filename = "Pong (1 player).ch8"; break;

    default: filename = "IBM Logo.ch8";       break;
    }

    // Chip-8 setup
    Chip8 cpu{ init(filename) };
    std::vector<std::uint8_t> display(64 * 32 * 4);
    std::uint16_t opcode{};
    
    using Milliseconds = std::chrono::duration<double, std::milli>;

    // timers setup
    constexpr Milliseconds timePerCycle{ 1000.0 / 500.0 }; // 500 Hz
    constexpr Milliseconds timePerTimer{ 1000.0 / 60.0 }; // 60 Hz
    auto lastTime = std::chrono::steady_clock::now();
    double timerAccumulator{ 0.0 };
    double cycleAccumulator{ 0.0 };

    FPS fps{};

    // window size and scale multiplier
    constexpr int wWidth{ 64 };
    constexpr int wHeight{ 32 };
    constexpr int wScale{ 15 };

    // SFML setup
    sf::RenderWindow window( sf::VideoMode( { wWidth * wScale, wHeight * wScale} ), "CHIP-8" );
    // window.setFramerateLimit(60);

    sf::Texture texture(sf::Vector2u(wWidth, wHeight));

    sf::Font font{};

    if (!font.openFromFile("../PressStart2P-Regular.ttf")) 
    {
        std::cerr << "Error loading font.\n";
        return -1; // Error loading
    }

    sf::Text fpsCounter(font);

    fpsCounter.setCharacterSize(10);
    fpsCounter.setFillColor(sf::Color::Green);
    fpsCounter.setPosition(sf::Vector2f(10, (wHeight * wScale) - 20));

    while (window.isOpen())
    {
        auto currentTime{ std::chrono::steady_clock::now() };
        Milliseconds dt{ currentTime - lastTime }; // gets the time elapsed
        lastTime = currentTime; // currentTime is now in the past, so lastTime = currentTime

        timerAccumulator += dt.count(); // increments the elapsed time per loop
        cycleAccumulator += dt.count(); 

        // FPS counter logic
        fps.accumulator += dt.count();

        if (fps.accumulator >= 1000.0)
        {
            fps.average = (fps.frames * 1000) / fps.accumulator;
            
            fpsCounter.setString(getFPS(fps.average));

            fps.frames = 0;
            fps.accumulator -= 1000.0;
        }

        while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scan::R) // reload the game: this is for games that freeze when you lose.
                {
                    cpu = init(filename);

                    timerAccumulator = 0;
                    cycleAccumulator = 0;
                    fps.accumulator = 0;
                    fps.frames = 0;
                }

                if (keyPressed->scancode == sf::Keyboard::Scan::Num1)
                {
                    cpu.keyBeingPressed = 0x1;
                    cpu.keypad[0x1]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num2)
                {
                    cpu.keyBeingPressed = 0x2;
                    cpu.keypad[0x2]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num3)
                {
                    cpu.keyBeingPressed = 0x3;
                    cpu.keypad[0x3]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num4)
                {
                    cpu.keyBeingPressed = 0xC;
                    cpu.keypad[0xC]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Q)
                {
                    cpu.keyBeingPressed = 0x4;
                    cpu.keypad[0x4]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::W)
                {
                    cpu.keyBeingPressed = 0x5;
                    cpu.keypad[0x5]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::E)
                {
                    cpu.keyBeingPressed = 0x6;
                    cpu.keypad[0x6]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::R)
                {
                    cpu.keyBeingPressed = 0xD;
                    cpu.keypad[0xD]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::A)
                {
                    cpu.keyBeingPressed = 0x7;
                    cpu.keypad[0x7]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::S)
                {
                    cpu.keyBeingPressed = 0x8;
                    cpu.keypad[0x8]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::D)
                {
                    cpu.keyBeingPressed = 0x9;
                    cpu.keypad[0x9]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::F)
                {
                    cpu.keyBeingPressed = 0xE;
                    cpu.keypad[0xE]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Z)
                {
                    cpu.keyBeingPressed = 0xA;
                    cpu.keypad[0xA]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::X)
                {
                    cpu.keyBeingPressed = 0x0;
                    cpu.keypad[0x0]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::C)
                {
                    cpu.keyBeingPressed = 0xB;
                    cpu.keypad[0xB]= 0x1;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::V)
                {
                    cpu.keyBeingPressed = 0xF;
                    cpu.keypad[0xF]= 0x1;
                }
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scan::Num1)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x1]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num2)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x2]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num3)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x3]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Num4)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xC]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Q)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x4]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::W)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x5]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::E)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x6]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::R)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xD]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::A)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x7]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::S)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x8]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::D)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x9]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::F)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xE]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::Z)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xA]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::X)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0x0]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::C)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xB]= 0x0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::V)
                {
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[0xF]= 0x0;
                }
            }
		}

        while (cycleAccumulator >= timePerCycle.count()) // fetch-decode-execute cycle at 500 Hz
        {
            if (!cpu.waitForAKeyPress)
                opcode = fetch(cpu);
            
            // std::cout << disassembler(opcode) << "\n";

            decode(cpu, opcode);
            cycleAccumulator -= timePerCycle.count();
        }

        display = getDisplay(cpu);
        texture.update(display.data());
        sf::Sprite sprite(texture);
        sprite.setScale(sf::Vector2f(wScale, wScale));

        while (timerAccumulator >= timePerTimer.count()) // decrement timers and update screen at 60 Hz
        {
            if (cpu.delayTimer > 0) 
                cpu.delayTimer--;

            if (cpu.soundTimer > 0) 
            {
                cpu.soundTimer--;
                // bips in the future
            }

            ++fps.frames;

            window.clear(sf::Color::Black);
            window.draw( sprite );
            window.draw( fpsCounter );
            window.display();
        
            timerAccumulator -= timePerTimer.count();
        }
        
        std::this_thread::sleep_for(1ms);
    }

    return 0;
}
