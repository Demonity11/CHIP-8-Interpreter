#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <vector>
#include <string>

class Debugger 
{
public:
	auto isDebugging() -> bool&;
	auto setDebugging(bool b) -> void; // if true the debugger will be shown

	auto isPaused() const -> bool;
	auto pause(bool b) -> void; // if true, pauses the emulation

	auto isSteping() -> bool;
	auto setStepMode(bool b) -> void; // if true, the emulator steps

	auto getAllInstructions() const -> std::vector<std::string>;
	auto setAllInstructions(std::vector<std::string> instructions) -> void;

	auto getInstruction(int index) const -> std::string;
	auto setInstruction(int index, std::string instruction) -> void;

	auto getVisibleLinesCount() -> int;
	auto setVisibleLinesCount(int linesCount) -> void; 

private:
    std::vector<std::string> disassembledInstructions;
    int currentLineIndex{ 0 };

    int visibleLinesCount{ 15 };
    int topVisibleLine{ 0 };

    bool paused{ false };
    bool stepMode{ false };
	bool showDebugger{ false };

    std::string pcString{};
    std::string iString{};
    std::vector<std::string> vRegisterStrings{};
};

#endif
