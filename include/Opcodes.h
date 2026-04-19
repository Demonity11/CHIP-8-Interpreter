#ifndef OPCODES_H
#define OPCODES_H

#include <cstdint>
#include <iomanip>

class Chip8;

class Opcodes
{
public:
	void setOpcode(std::uint16_t newOpcode)
	{
		opcode = newOpcode;
	}

	void fetch(Chip8& cpu);
	void decode(Chip8& cpu);

	void op_00E0(Chip8& cpu);
    void op_00EE(Chip8& cpu);
    void op_Annn(Chip8& cpu);
    void op_1nnn(Chip8& cpu);
    void op_2nnn(Chip8& cpu);
    void op_3xkk(Chip8& cpu);
    void op_4xkk(Chip8& cpu);
    void op_5xy0(Chip8& cpu);
    void op_6xkk(Chip8& cpu);
    void op_7xkk(Chip8& cpu);
    void op_8xy0(Chip8& cpu);
    void op_8xy1(Chip8& cpu);
    void op_8xy2(Chip8& cpu);
    void op_8xy3(Chip8& cpu);
    void op_8xy4(Chip8& cpu);
    void op_8xy5(Chip8& cpu);
    void op_8xy6(Chip8& cpu);
    void op_8xy7(Chip8& cpu);
    void op_8xyE(Chip8& cpu);
    void op_9xy0(Chip8& cpu);
    void op_Bnnn(Chip8& cpu);
    void op_Cxkk(Chip8& cpu);
    void op_Dxyn(Chip8& cpu);
    void op_Ex9E(Chip8& cpu); 
    void op_ExA1(Chip8& cpu); 
    void op_Fx07(Chip8& cpu);
    void op_Fx0A(Chip8& cpu); 
    void op_Fx15(Chip8& cpu);
    void op_Fx18(Chip8& cpu);
    void op_Fx1E(Chip8& cpu);
    void op_Fx29(Chip8& cpu);
    void op_Fx33(Chip8& cpu);
    void op_Fx55(Chip8& cpu);
    void op_Fx65(Chip8& cpu);

private:
	std::uint16_t opcode{};
};

#endif
