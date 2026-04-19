#include "../include/Debugger.h"

bool& Debugger::isDebugging() { return showDebugger; }
void Debugger::setDebugging(bool b) { showDebugger = b; } // if true the debugger will be shown

bool Debugger::isPaused() const { return paused; }
void Debugger::pause(bool b) { paused = b; } // if true, pauses the emulation

bool Debugger::isSteping() { return stepMode; }
void Debugger::setStepMode (bool b) { stepMode = b; } // if true, the emulator steps

std::vector<std::string> Debugger::getAllInstructions() const { return disassembledInstructions; }
void Debugger::setAllInstructions(std::vector<std::string> instructions) { disassembledInstructions = instructions; }

std::string Debugger::getInstruction(int index) const { return disassembledInstructions.at(index); }
void Debugger::setInstruction(int index, std::string instruction) { disassembledInstructions[index] = instruction; }

int Debugger::getVisibleLinesCount() { return visibleLinesCount; }
void Debugger::setVisibleLinesCount(int linesCount) { visibleLinesCount = linesCount; }
