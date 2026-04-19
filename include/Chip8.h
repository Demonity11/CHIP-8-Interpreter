#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

class Chip8 // the chip 8 cpu works at a clock of 500 instructions per second (500 Hz)
{
public:
	enum State
	{
		RomSelection,
		Running,
		Paused,
	};

	Chip8();

	auto loadROM() 											-> void; 										 
	auto loadFontSprites()      							-> void;
	auto reset() 						  					-> void;
	auto getDisplay()										-> std::vector<std::uint8_t>;
	auto getOpcodeConvertedToString(std::uint16_t opcode)   -> std::string;

	std::vector<std::string> getMemoryContent() const;
	std::string getCallStack(int index) const;
	std::string getRegister(int index) const;

	void setState(State emuState) { m_state = emuState; }
	State getState() const { return m_state; }

	void setKeyBeingPressed(std::uint8_t key) { m_keyBeingPressed = key; }
	std::uint8_t getKeyBeingPressed() const { return m_keyBeingPressed; }

	void setKeypad(int index, std::uint8_t key) { m_keypad[index] = key; }
	std::uint8_t getKeypad(int index) const { return m_keypad[index]; }

	void waitForKey(bool b) { m_waitForAKeyPress = b; }
	bool isWaitingForKey() const { return m_waitForAKeyPress; }

	void decrementDelayTimer() { --m_delayTimer; }
	std::uint8_t getDelayTimer() const { return m_delayTimer; }

	void decrementSoundTimer() { --m_soundTimer; }
	std::uint8_t getSoundTimer() const { return m_soundTimer; }

	std::uint16_t getPC() const { return m_pc; }
	std::uint8_t getSP() const { return m_sp; }
	std::uint16_t getI() const { return m_I; }

	void setFilename(const std::string& file) { m_filename = file; }
	std::string getFilename() { return m_filename; }

	friend class Opcodes;

private:
	std::uint8_t m_memory[4096]{}; // most programs starts at 0x200 (512)
	std::uint8_t m_V[16]{}; // general purpose 8-bit registers
	std::uint16_t m_I{}; // used to store addresses
	std::uint16_t m_pc{0x200}; // program counter (pc): holds the memory address of the instruction being executed
	std::uint16_t m_stack[16]{}; // used to hold the value of the address to get back to when a routine is finished
	std::uint8_t m_sp{}; // used to hold the address of the top level of the stack

	std::uint8_t m_delayTimer{}; // decrements 60 times per second (60 Hz)
	std::uint8_t m_soundTimer{}; // decrements at a frequency established by the programmer (me). not defined yet 

	std::uint8_t m_display[64 * 32]{}; // a 64x32 display represented by an array of size 2048
	std::uint8_t m_keypad[16]{}; // a 16-key keypad, represented by hexadecimal numbers, from 0 to F

	// Chip-8 layout     // mapped to
	// 	  1 2 3 C      		1 2 3 4
	//    4 5 6 D 			Q W E R
	//    7 8 9 E           A S D F
	//    A 0 B F           Z X C V

	bool m_waitForAKeyPress{ false }; // when this is true, the program should stop the pc until a key is pressed
	std::uint8_t m_keyBeingPressed{0xFF}; // 0xFF it will represent the that nothing is being pressed, otherwise it will store a number from 0x to 0xF
	State m_state{State::RomSelection};

	std::string m_filename{};
	int m_fileSize{};
};

#endif
