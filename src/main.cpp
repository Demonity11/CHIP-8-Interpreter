#include <chrono>
#include <cmath>
#include <map>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include "Chip8.h"
#include "Debugger.h"
#include "Disassembler.h"
#include "Opcodes.h"
#include "Utilities.h"

// foward declarations
void showDebugger(sf::RenderWindow& window, Debugger& debugger, Chip8& cpu);
std::optional<std::string> romSelection(Debugger& debugger, bool& showFPS, sf::RenderWindow& window);

int main()
{   
    // Debugger initialization
    Debugger debugger{};

    // Opcodes initialization
    Opcodes opcode{};

    // Main Menu Bar State
    bool showMainMenu{ true };

    // keymapping setup
    std::map<sf::Keyboard::Scan, std::uint8_t> keyMapping
    {
        {sf::Keyboard::Scan::Num1, 0x1}, 
        {sf::Keyboard::Scan::Num2, 0x2},
        {sf::Keyboard::Scan::Num3, 0x3},
        {sf::Keyboard::Scan::Num4, 0xC},
        {sf::Keyboard::Scan::Q, 0x4},
        {sf::Keyboard::Scan::W, 0x5},
        {sf::Keyboard::Scan::E, 0x6},
        {sf::Keyboard::Scan::R, 0xD},
        {sf::Keyboard::Scan::A, 0x7},
        {sf::Keyboard::Scan::S, 0x8},
        {sf::Keyboard::Scan::D, 0x9},
        {sf::Keyboard::Scan::F, 0xE},
        {sf::Keyboard::Scan::Z, 0xA},
        {sf::Keyboard::Scan::X, 0x0},
        {sf::Keyboard::Scan::C, 0xB},
        {sf::Keyboard::Scan::V, 0xF}
    };

    // Chip-8 initialization
    Chip8 cpu{};

    std::vector<std::uint8_t> display(64 * 32 * 4);
    
    // timers setup
    using Milliseconds = std::chrono::duration<double, std::milli>;

    constexpr Milliseconds timePerCycle{ 1000.0 / 500.0 }; // 500 Hz
    constexpr Milliseconds timePerTimer{ 1000.0 / 60.0 }; // 60 Hz
    double timerAccumulator{ 0.0 };
    double cycleAccumulator{ 0.0 };
    
    auto lastTime = std::chrono::steady_clock::now();

    // window size and scale multiplier
    constexpr int windowWidth{ 64 };
    constexpr int windowHeight{ 32 };
    constexpr int windowScale{ 20 };

    // SFML setup
    sf::RenderWindow window( sf::VideoMode( { windowWidth * windowScale, windowHeight * windowScale} ), "2[E]mu" );
    window.setFramerateLimit(60);
    sf::Clock deltaClock{};

    // ImGui setup
    if (!ImGui::SFML::Init(window))
        return -1;

    // textures setup
    sf::Texture gameWindow(sf::Vector2u(windowWidth, windowHeight));
    sf::Texture romSelectionWindow{};

    if (!romSelectionWindow.loadFromFile("assets/2emu_logo.png", false, sf::IntRect({0, 0}, {64,32})))
    {
        std::cerr << "Error. '2emu_logo.png' not found.\n";
        return -1;
    }

	// sound setup
    constexpr unsigned int SAMPLE_RATE{ 44100 };
    constexpr int AMPLITUDE{ 30000 };
    constexpr double FREQUENCY{ 440.0 };

    std::vector<std::int16_t> samples(SAMPLE_RATE / 10);

    constexpr double cyclesPerSample{ FREQUENCY / SAMPLE_RATE };

    for (unsigned int i{ 0 }; i < samples.size(); ++i)
    {
        if (std::sin(2 * 3.1415926535 * cyclesPerSample * i) > 0)
            samples[i] = AMPLITUDE;
        else
            samples[i] = -AMPLITUDE;
    }

    sf::SoundBuffer buffer;
    if (!buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE, {sf::SoundChannel::Mono}))
        return -1;

    sf::Sound sound(buffer);
    sound.setLooping(true);

    // font setup
    sf::Font font{};

    if (!font.openFromFile("assets/PressStart2P-Regular.ttf")) 
    {
        std::cerr << "Error loading font.\n";
        return -1; // Error loading
    }

    // fps counter setup
    FPS fps{};
    bool showFPS{ false };

    sf::Text fpsCounter(font);
    fpsCounter.setCharacterSize(10);
    fpsCounter.setFillColor(sf::Color::Green);
    fpsCounter.setPosition(sf::Vector2f(10, (windowHeight * windowScale) - 20));

    // game loop
    while (window.isOpen())
    {
        auto currentTime{ std::chrono::steady_clock::now() };
        Milliseconds dt{ currentTime - lastTime }; // gets the time elapsed
        lastTime = currentTime; // currentTime is now in the past, so lastTime = currentTime

        double frameTime = std::min(dt.count(), 250.0); 

        timerAccumulator += frameTime;
        cycleAccumulator += frameTime;

        // FPS counter logic
        fps.accumulator += dt.count();

        if (fps.accumulator >= 1000.0) // if 1s has passed, then we update the fps.
        {
            fps.average = (fps.frames * 1000) / fps.accumulator;
            
            fpsCounter.setString(Disassembler::getFPS(fps.average));

            fps.frames = 0;
            fps.accumulator -= 1000.0;
        }

        // event loop
        while ( const std::optional event = window.pollEvent() )
		{
            ImGui::SFML::ProcessEvent(window, *event);

			if ( event->is<sf::Event::Closed>() )
				window.close();

            // input handler
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                auto key { keyPressed->scancode };

                if (key == sf::Keyboard::Scan::F1) // reload the game. This is for games that freezes when you lose.
                {
                    if (cpu.getFilename() != "")
                    {
                        cpu.reset();
                        cpu.loadROM();

                        timerAccumulator = 0;
                        cycleAccumulator = 0;
                        fps.accumulator = 0;
                        fps.frames = 0;
                    }
                }

                else
                {
                    if (keyMapping.count(key) != 0)
                    {
                        cpu.setKeyBeingPressed(keyMapping.at(key));
                        cpu.setKeypad(keyMapping.at(key), 0x1);
                    }
                }
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
            {
                auto key { keyPressed->scancode };

                if (key == sf::Keyboard::Scan::Escape)
                {
                    showMainMenu ^= 1;
                }

                if (keyMapping.count(key) != 0)
                {
                    cpu.setKeyBeingPressed(0xFF);
                    cpu.setKeypad(keyMapping.at(key), 0x0);
                }
            }
		}

        if (debugger.isSteping()) // if the Step button is clicked, then we enter here and execute exactly one instruction. 
        {
            if (!cpu.isWaitingForKey())
            {
                opcode.fetch(cpu);
            }
            
            opcode.decode(cpu);

            debugger.stepMode(false);
        }

        while (cycleAccumulator >= timePerCycle.count()) // fetch-decode-execute cycle at 500 Hz
        {
            cycleAccumulator -= timePerCycle.count();

            if (cpu.getState() == Chip8::Running) // if the emulator is not paused, then we fetch-decode-execute
            {
                if (cpu.isWaitingForKey())
                {
                    opcode.decode(cpu);
                    continue;
                }
    
                opcode.fetch(cpu);
                opcode.decode(cpu);
            }
        }

        while (timerAccumulator >= timePerTimer.count()) // decrement timers and update screen at 60 Hz
        {
            ++fps.frames;
            timerAccumulator -= timePerTimer.count();

            if (cpu.getState() == Chip8::Running)
            {
                if (cpu.getDelayTimer() > 0) 
                    cpu.decrementDelayTimer();
    
                if (cpu.getSoundTimer() > 0) 
                {
                    cpu.decrementSoundTimer();

                    if (sound.getStatus() != sf::Sound::Status::Playing)
                    {
                        sound.play();
                    }
                }
                else
                {
                    if (sound.getStatus() == sf::Sound::Status::Playing)
                    {
                        sound.stop();
                    }
                }
            }
        }
        
        // paused because when we step, we want to see things updating on screen
        if (cpu.getState() == Chip8::Running || cpu.getState() == Chip8::Paused)
        {
            display = cpu.getDisplay();
            gameWindow.update(display.data());
        }

        sf::Sprite gameWindowSprite((cpu.getState() == Chip8::Running || cpu.getState() == Chip8::Paused) ? gameWindow : romSelectionWindow);
        gameWindowSprite.setScale(sf::Vector2f(windowScale, windowScale));
        
        // ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // ImGui debugger interface
        if (debugger.isDebugging())
        {
            showDebugger(window, debugger, cpu);
        }

        // ImGui rom selection interface
        
        if (showMainMenu)
        {
            auto newFile { romSelection(debugger, showFPS, window) };
    
            if (newFile)
            {
                cpu.setFilename(*newFile);

                cpu.loadROM();
                cpu.setState(Chip8::Running);
                showMainMenu = false;
            }
        }

        // SFML drawing functions
        window.clear( (cpu.getState() == Chip8::Running) ? sf::Color::Black : sf::Color::White );
        window.draw( gameWindowSprite );

        if (showFPS)
            window.draw( fpsCounter );

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}

void showDebugger(sf::RenderWindow& window, Debugger& debugger, Chip8& cpu)
{
    if (debugger.isDebugging())
    {
        ImGui::Begin("Debugger", &debugger.isDebugging(), ImGuiWindowFlags_None);
        
        if (ImGui::BeginTabBar("DebuggerTabs"))
        {
            if (ImGui::BeginTabItem("Instructions"))
            {
                debugger.setAllInstructions(cpu.getMemoryContent());

                int offset{ (cpu.getPC() > 0x200) ? 2 : 0 }; // if the address greater than 0x200, then we subtract 2 from baseIndex
                int baseIndex{ (cpu.getPC() - 0x200 - offset) / 2 };
        
                for (int i{ 0 }; i < debugger.getVisibleLinesCount(); ++i)
                {
                    int index{ baseIndex + i };
        
                    if (index >= 0 && index < static_cast<int>(debugger.getAllInstructions().size()))
                    {
                        if (i == 0)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
                            ImGui::Text(debugger.getInstruction(index).c_str());
                            ImGui::PopStyleColor();
                        }

                        else 
                        {
                            ImGui::TextUnformatted(debugger.getInstruction(index).c_str());
                        }
                    }
                }
                ImGui::EndTabItem();
            }
    
            if (ImGui::BeginTabItem("Register (V)"))
            {
                if (ImGui::BeginTable("RegisterTable", 2))
                {
                    for (int i = 0; i < 16; i++)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(cpu.getRegister(i).c_str());
                    }
    
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
    
            if (ImGui::BeginTabItem("Stack"))
            {
                if (ImGui::BeginTable("StackTable", 2))
                {
                    for (int i = 0; i < 16; i++)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(cpu.getCallStack(i).c_str());
                    }
    
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
    
            if (ImGui::BeginTabItem("Other"))
            {
                ImGui::Text("PC: 0x%04X", cpu.getPC());
                ImGui::Text("SP: 0x%02X", cpu.getSP());
                ImGui::Text("DT: 0x%02X", cpu.getDelayTimer());
                ImGui::Text("ST: 0x%02X", cpu.getSoundTimer());
                ImGui::Text("I:  0x%04X", cpu.getI());

                if (cpu.isWaitingForKey()) ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "STATUS: WAITING FOR KEY");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        if (ImGui::Button((cpu.getState() == Chip8::Running) ? "Pause Emulation" : "Resume Emulation"))
        {
            cpu.setState((cpu.getState() == Chip8::Running) ? Chip8::Paused : Chip8::Running);
            // if the emulator is running, then we switch to paused and vice-versa.
        }

        if (ImGui::Button("Step"))
        {
            debugger.stepMode(true);
        }

        ImGui::End();
    }
}

std::optional<std::string> romSelection(Debugger& debugger, bool& showFPS, sf::RenderWindow& window)
{
    namespace fs = std::filesystem;

    std::optional<std::string> filename{};

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("(Rom Manager)", NULL, false, false);
            if (ImGui::BeginMenu("Select ROM"))
            {
                std::string romsPath{ "roms/" };

                // Iterate over all files and subdirectories in the given path
                for (const auto& entry : fs::directory_iterator(romsPath)) 
                {
                    if ( ImGui::MenuItem(entry.path().filename().string().c_str()) )
                    {
                        filename = entry.path().filename().string();
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Show FPS", NULL, showFPS))
            {
                showFPS ^= 1;
            }

            if (ImGui::MenuItem("Show Debugger", NULL, debugger.isDebugging()))
            {
                debugger.setDebugging((debugger.isDebugging() == true) ? false : true);
            }

            if (ImGui::MenuItem("Quit"))
            {
                window.close();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    return filename;
}
