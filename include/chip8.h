#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>

struct Chip8 // the chip 8 cpu works at a clock of 500 instructions per second (500 Hz)
{
	std::uint8_t memory[4096]{}; // most programs starts at 0x200 (512)
	std::uint8_t V[16]{}; // general purpose 8-bit registers
	std::uint16_t I{}; // used to store addresses
	std::uint16_t pc{0x200}; // program counter (pc): holds the memory address of the instruction being executed
	std::uint16_t stack[16]{}; // used to hold the value of the address to get back to when a routine is finished
	std::uint8_t sp{}; // used to hold the address of the top level of the stack

	std::uint8_t delayTimer{}; // decrements 60 times per second (60 Hz)
	std::uint8_t soundTimer{}; // decrements at a frequency established by the programmer (me). not defined yet 

	std::uint8_t display[64 * 32]{}; // a 64x32 display represented by an array of size 2048
	std::uint8_t keypad[16]{0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF}; 
	// a 16-key keypad, represented by hexadecimal numbers, from 0 to F
};

// forward declarations for "chip8.cpp"
auto decode(Chip8& cpu, std::uint16_t opcode)         				 -> void;
auto fetch(Chip8& cpu) 								  				 -> std::uint16_t;
auto loadROM(Chip8& cpu, const std::string& filename) 				 -> int;
auto loadFontSprites(Chip8& cpu)                      				 -> void;
auto clearMemory(Chip8& cpu) 						  				 -> void;
auto printROM(const Chip8& cpu, int fileSize) 		  				 -> void;
auto printDisplay(const Chip8& cpu) 				  				 -> void;
auto init(std::string romName) 						  				 -> Chip8;
auto getDisplay(const Chip8& cpu) 									 -> std::vector<std::uint8_t>;

// forward declarations for "opcodes.cpp"
void op_00E0(Chip8& cpu, std::uint16_t opcode);
void op_00EE(Chip8& cpu, std::uint16_t opcode);
void op_Annn(Chip8& cpu, std::uint16_t opcode);
void op_1nnn(Chip8& cpu, std::uint16_t opcode);
void op_2nnn(Chip8& cpu, std::uint16_t opcode);
void op_3xkk(Chip8& cpu, std::uint16_t opcode);
void op_4xkk(Chip8& cpu, std::uint16_t opcode);
void op_5xy0(Chip8& cpu, std::uint16_t opcode);
void op_6xkk(Chip8& cpu, std::uint16_t opcode);
void op_7xkk(Chip8& cpu, std::uint16_t opcode);
void op_8xy0(Chip8& cpu, std::uint16_t opcode);
void op_8xy1(Chip8& cpu, std::uint16_t opcode);
void op_8xy2(Chip8& cpu, std::uint16_t opcode);
void op_8xy3(Chip8& cpu, std::uint16_t opcode);
void op_8xy4(Chip8& cpu, std::uint16_t opcode);
void op_8xy5(Chip8& cpu, std::uint16_t opcode);
void op_8xy6(Chip8& cpu, std::uint16_t opcode);
void op_8xy7(Chip8& cpu, std::uint16_t opcode);
void op_8xyE(Chip8& cpu, std::uint16_t opcode);
void op_9xy0(Chip8& cpu, std::uint16_t opcode);
void op_Bnnn(Chip8& cpu, std::uint16_t opcode);
void op_Cxkk(Chip8& cpu, std::uint16_t opcode);
void op_Dxyn(Chip8& cpu, std::uint16_t opcode);
void op_Ex9E(Chip8& cpu, std::uint16_t opcode);
void op_ExA1(Chip8& cpu, std::uint16_t opcode);
void op_Fx07(Chip8& cpu, std::uint16_t opcode);
void op_Fx0A(Chip8& cpu, std::uint16_t opcode); // not finished
void op_Fx15(Chip8& cpu, std::uint16_t opcode);
void op_Fx18(Chip8& cpu, std::uint16_t opcode);
void op_Fx1E(Chip8& cpu, std::uint16_t opcode);
void op_Fx29(Chip8& cpu, std::uint16_t opcode);
void op_Fx33(Chip8& cpu, std::uint16_t opcode);
void op_Fx55(Chip8& cpu, std::uint16_t opcode);
void op_Fx65(Chip8& cpu, std::uint16_t opcode);

#endif
