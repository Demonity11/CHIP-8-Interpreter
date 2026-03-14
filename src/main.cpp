#include "../include/chip8.h"
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>

using namespace std::literals::chrono_literals;

int main()
{   
    // Chip-8 setup
    Chip8 cpu{ init("IBM Logo.ch8") };
    std::vector<std::uint8_t> display(64 * 32 * 4);
    
    using Milliseconds = std::chrono::duration<double, std::milli>;

    // timers setup
    constexpr Milliseconds timePerCycle{ 1000.0 / 500.0 }; // 500 Hz
    constexpr Milliseconds timePerTimer{ 1000.0 / 60.0 }; // 60 Hz
    auto lastTime = std::chrono::steady_clock::now();
    double timerAccumulator{ 0.0 };
    double cycleAccumulator{ 0.0 };

    // SFML setup
    sf::RenderWindow window( sf::VideoMode( { 64, 32 } ), "CHIP-8" );
    sf::Texture texture(sf::Vector2u(64, 32));

    while (window.isOpen())
    {
        auto currentTime{ std::chrono::steady_clock::now() };
        Milliseconds dt{ currentTime - lastTime }; // gets the time elapsed
        lastTime = currentTime; // currentTime is now in the past, so lastTime = currentTime

        timerAccumulator += dt.count(); // increments the elapsed time per loop
        cycleAccumulator += dt.count(); 

        while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

        while (cycleAccumulator >= timePerCycle.count()) // fetch-decode-execute cycle at 500 Hz
        {
            std::uint16_t opcode = fetch(cpu);
            decode(cpu, opcode);
            cycleAccumulator -= timePerCycle.count();
        }

        display = getDisplay(cpu);
        texture.update(display.data());
        sf::Sprite sprite(texture);

        while (timerAccumulator >= timePerTimer.count()) // decrement timers and update screen at 60 Hz
        {
            if (cpu.delayTimer > 0) cpu.delayTimer--;
            if (cpu.soundTimer > 0) {
                cpu.soundTimer--;
                // bips in the future
            }

            window.clear(sf::Color::Black);
            window.draw( sprite );
            window.display();
        
            timerAccumulator -= timePerTimer.count();
        }

        // std::this_thread::sleep_for(1ms);
    }

    return 0;
}
