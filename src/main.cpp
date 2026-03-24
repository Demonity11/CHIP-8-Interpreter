#include <chrono>
#include <cmath>
#include <map>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "../include/chip8.h"
#include "imgui.h"
#include "imgui-SFML.h"

// foward declarations
void showDebugger(bool isDebugging, sf::RenderWindow& window, DebuggerViewState& debugger, const Chip8& cpu, EmulatorState& emulatorState);
std::optional<std::string> romSelection(bool& isDebugging, sf::RenderWindow& window);

int main()
{   
    EmulatorState emulatorState{ EmulatorState::RomSelection };

    std::string filename{};

    // Debugger setup
    bool isDebugging{ false };
    DebuggerViewState debugger{};

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

    // Chip-8 setup
    Chip8 cpu{};

    std::vector<std::uint8_t> display(64 * 32 * 4);
    std::uint16_t opcode{};
    
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
    sf::RenderWindow window( sf::VideoMode( { windowWidth * windowScale, windowHeight * windowScale} ), "CHIP-8" );
    window.setFramerateLimit(60);
    sf::Clock deltaClock{};

    // ImGui setup
    if (!ImGui::SFML::Init(window))
        return -1;

    // textures setup
    sf::Texture gameWindow(sf::Vector2u(windowWidth, windowHeight));

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

    if (!font.openFromFile("../PressStart2P-Regular.ttf")) 
    {
        std::cerr << "Error loading font.\n";
        return -1; // Error loading
    }

    // fps counter setup
    FPS fps{};

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
            
            fpsCounter.setString(getFPS(fps.average));

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
                    if (filename != "")
                    {
                        cpu = init(filename, debugger, isDebugging);

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
                        cpu.keyBeingPressed = keyMapping.at(key);
                        cpu.keypad[keyMapping.at(key)] = 0x1;
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
                    cpu.keyBeingPressed = 0xFF;
                    cpu.keypad[keyMapping.at(key)] = 0x0;
                }
            }
		}

        if (debugger.stepMode) // if the Step button is clicked, then we enter here and execute exactly one instruction. 
        {
            if (!cpu.waitForAKeyPress)
            {
                opcode = fetch(cpu);
            }
            
            decode(cpu, opcode);

            debugger.stepMode = false;
        }

        while (cycleAccumulator >= timePerCycle.count()) // fetch-decode-execute cycle at 500 Hz
        {
            cycleAccumulator -= timePerCycle.count();

            if (emulatorState == EmulatorState::Running) // if the emulator is not paused, then we fetch-decode-execute
            {
                if (cpu.waitForAKeyPress)
                {
                    decode(cpu, opcode);
                    continue;
                }
    
                opcode = fetch(cpu);
                decode(cpu, opcode);
            }
        }

        while (timerAccumulator >= timePerTimer.count()) // decrement timers and update screen at 60 Hz
        {
            ++fps.frames;
            timerAccumulator -= timePerTimer.count();

            if (emulatorState == EmulatorState::Running)
            {
                if (cpu.delayTimer > 0) 
                    cpu.delayTimer--;
    
                if (cpu.soundTimer > 0) 
                {
                    cpu.soundTimer--;
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
        if (emulatorState == EmulatorState::Running || emulatorState == EmulatorState::Paused)
        {
            display = getDisplay(cpu);
            gameWindow.update(display.data());
        }

        sf::Sprite gameWindowSprite(gameWindow);
        gameWindowSprite.setScale(sf::Vector2f(windowScale, windowScale));
        
        // ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // ImGui debugger interface
        if (isDebugging)
        {
            showDebugger(isDebugging, window, debugger, cpu, emulatorState);
        }

        // ImGui rom selection interface
        
        if (showMainMenu)
        {
            auto newFile { romSelection(isDebugging, window) }; // under construction.
    
            if (newFile)
            {
                filename = *newFile;

                cpu = init(filename, debugger, isDebugging);
                emulatorState = EmulatorState::Running;
                showMainMenu = false;
            }
        }

        // SFML drawing functions
        window.clear(sf::Color::Black);
        window.draw( gameWindowSprite );
        window.draw( fpsCounter );

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}

void showDebugger(bool isDebugging, sf::RenderWindow& window, DebuggerViewState& debugger, const Chip8& cpu, EmulatorState& emulatorState)
{
    if (isDebugging)
    {
        ImGui::Begin("Debugger", &debugger.showDebugger, ImGuiWindowFlags_None);
        
        if (ImGui::BeginTabBar("DebuggerTabs"))
        {
            if (ImGui::BeginTabItem("Instructions"))
            {
                int offset{ (cpu.pc > 0x200) ? 2 : 0 }; // if the address greater than 0x200, then we subtract 2 from baseIndex
                int baseIndex{ (cpu.pc - 0x200 - offset) / 2 };
        
                for (int i{ 0 }; i < debugger.visibleLinesCount; ++i)
                {
                    int index{ baseIndex + i };
        
                    if (index >= 0 && index < static_cast<int>(debugger.disassembledInstructions.size()))
                    {
                        if (i == 0)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
                            ImGui::Text(debugger.disassembledInstructions[index].c_str());
                            ImGui::PopStyleColor();
                        }

                        else 
                        {
                            ImGui::TextUnformatted(debugger.disassembledInstructions[index].c_str());
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
                        ImGui::Text("V%X: 0x%02X", i, cpu.V[i]);
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
                        ImGui::Text("S%X: 0x%02X", i, cpu.stack[i]);
                    }
    
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
    
            if (ImGui::BeginTabItem("Other"))
            {
                ImGui::Text("PC: 0x%04X", cpu.pc);
                ImGui::Text("SP: 0x%02X", cpu.sp);
                ImGui::Text("DT: 0x%02X", cpu.delayTimer);
                ImGui::Text("ST: 0x%02X", cpu.soundTimer);
                ImGui::Text("I: 0x%04X", cpu.I);

                if (cpu.waitForAKeyPress) ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "STATUS: WAITING FOR KEY");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        if (ImGui::Button((emulatorState == EmulatorState::Running) ? "Pause Emulation" : "Resume Emulation"))
        {
            emulatorState = (emulatorState == EmulatorState::Running) ? EmulatorState::Paused : EmulatorState::Running;
            // if the emulator is running, then we switch to paused and vice-versa.
        }

        if (ImGui::Button("Step"))
        {
            debugger.stepMode = true;
        }

        ImGui::End();
    }
}

std::optional<std::string> romSelection(bool& isDebugging, sf::RenderWindow& window)
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
                std::string romsPath{ "../roms" };

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

            if (ImGui::MenuItem("Show Debugger", NULL, isDebugging))
            {
                isDebugging ^= 1;
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
