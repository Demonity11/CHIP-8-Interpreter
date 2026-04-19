#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

class Chip8 // the chip 8 cpu works at a clock of 500 instructions per second (500 Hz)
{
public:
	enum State
	{
		RomSelection,
		Running,
		Paused,
	};

	auto loadROM() 											-> void; 										 
	auto loadFontSprites()      							-> void;
	auto reset() 						  					-> void;
	auto printDisplay()										-> void;
	auto init() 											-> void;
	auto getDisplay()										-> std::vector<std::uint8_t>;
	auto getOpcodeConvertedToString(std::uint16_t opcode)   -> std::string;

	std::vector<std::string> getMemoryContent() const;
	std::string getCallStack(int index) const;
	std::string getRegister(int index) const;

	void setState(State emuState) { state = emuState; }
	State getState() const { return state; }

	void setKeyBeingPressed(std::uint8_t key) { keyBeingPressed = key; }
	std::uint8_t getKeyBeingPressed() const { return keyBeingPressed; }

	void setKeypad(int index, std::uint8_t key) { keypad[index] = key; }
	std::uint8_t getKeypad(int index) const { return keypad[index]; }

	void waitForKey(bool b) { waitForAKeyPress = b; }
	bool isWaitingForKey() const { return waitForAKeyPress; }

	void decrementDelayTimer() { --delayTimer; }
	std::uint8_t getDelayTimer() const { return delayTimer; }

	void decrementSoundTimer() { --soundTimer; }
	std::uint8_t getSoundTimer() const { return soundTimer; }

	std::uint16_t getPC() const { return pc; }
	std::uint8_t getSP() const { return sp; }
	std::uint16_t getI() const { return I; }

	void setFilename(const std::string& file) { filename = file; }
	std::string getFilename() { return filename; }

	friend class Opcodes;

private:
	std::uint8_t memory[4096]{}; // most programs starts at 0x200 (512)
	std::uint8_t V[16]{}; // general purpose 8-bit registers
	std::uint16_t I{}; // used to store addresses
	std::uint16_t pc{0x200}; // program counter (pc): holds the memory address of the instruction being executed
	std::uint16_t stack[16]{}; // used to hold the value of the address to get back to when a routine is finished
	std::uint8_t sp{}; // used to hold the address of the top level of the stack

	std::uint8_t delayTimer{}; // decrements 60 times per second (60 Hz)
	std::uint8_t soundTimer{}; // decrements at a frequency established by the programmer (me). not defined yet 

	std::uint8_t display[64 * 32]{}; // a 64x32 display represented by an array of size 2048
	std::uint8_t keypad[16]{}; // a 16-key keypad, represented by hexadecimal numbers, from 0 to F

	// Chip-8 layout     // mapped to
	// 	  1 2 3 C      		1 2 3 4
	//    4 5 6 D 			Q W E R
	//    7 8 9 E           A S D F
	//    A 0 B F           Z X C V

	bool waitForAKeyPress{ false }; // when this is true, the program should stop the pc until a key is pressed
	std::uint8_t keyBeingPressed{0xFF}; // 0xFF it will represent the that nothing is being pressed, otherwise it will store a number from 0x to 0xF
	State state{State::RomSelection};

	std::string filename{};
	int fileSize{};
};

#endif
