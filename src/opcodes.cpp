#include "../include/chip8.h"
#include "../include/Random.h"

namespace Masks
{
	constexpr std::uint16_t nnn{ 0x0FFF }; // extracts the lower 12 bits
	constexpr std::uint16_t n{ 0x000F }; // extracts the lower 4 bits
	constexpr std::uint16_t x{ 0x0F00 }; // extracts the lower byte of the highbyte
	constexpr std::uint16_t y{ 0x00F0 }; // extracts the higher byte of the lowbyte
	constexpr std::uint16_t kk{0x00FF}; // extracts the lower 8 bits
}

void op_00E0(Chip8& cpu, std::uint16_t opcode)
{
    std::fill(std::begin(cpu.display), std::end(cpu.display), 0);
}

void op_00EE(Chip8& cpu, std::uint16_t opcode) {
    if (cpu.sp > 0) 
	{
        --cpu.sp;
        cpu.pc = cpu.stack[cpu.sp];
    } 
}

void op_Annn(Chip8& cpu, std::uint16_t opcode)
{
    cpu.I = opcode & Masks::nnn;
    // std::cout << "I = " << cpu.I << "\n";
}

void op_1nnn(Chip8& cpu, std::uint16_t opcode)
{
	cpu.pc = opcode & Masks::nnn;
	// std::cout << "0x" << std::hex << std::uppercase << cpu.pc << "\n";
}

void op_2nnn(Chip8& cpu, std::uint16_t opcode) {
    if (cpu.sp < 15) 
	{ 
        cpu.stack[cpu.sp] = cpu.pc;
        ++cpu.sp;
        cpu.pc = opcode & Masks::nnn;
    } 
}

void op_3xkk(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x  = (opcode & Masks::x) >> 8;
	std::uint16_t kk = opcode & Masks::kk;

	if (cpu.V[x] == kk)
		cpu.pc += 2;
}

void op_4xkk(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x  = (opcode & Masks::x) >> 8;
	std::uint16_t kk = opcode & Masks::kk;

	if (cpu.V[x] != kk)
		cpu.pc += 2;
}

void op_5xy0(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	// std::cout << "x = 0x" << std::hex << std::uppercase << x << "\n";
	// std::cout << "y = 0x" << std::hex << std::uppercase << y << "\n";

	if (cpu.V[x] == cpu.V[y])
		cpu.pc += 2;
}

void op_6xkk(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x  = (opcode & Masks::x) >> 8;
	std::uint16_t kk = opcode & Masks::kk;

	cpu.V[x] = static_cast<std::uint8_t>(kk);

	// std::cout does not print a std::uint8_t without converting to an int
	// std::cout << "V[x] = 0x" << std::hex << std::uppercase << static_cast<int>(cpu.V[x]) << "\n";
}

void op_7xkk(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x  = (opcode & Masks::x) >> 8;
	std::uint16_t kk = opcode & Masks::kk;

	// std::cout << "V[x] = 0x" << std::hex << std::uppercase << static_cast<int>(cpu.V[x]) << " + 0x" << kk << "\n";

	cpu.V[x] += static_cast<std::uint8_t>(kk);

	// std::cout << "V[x] = 0x" << std::hex << std::uppercase << static_cast<int>(cpu.V[x]) << "\n";
}

void op_8xy0(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	cpu.V[x] = cpu.V[y];
}

void op_8xy1(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	cpu.V[x] = cpu.V[x] | cpu.V[y];
}

void op_8xy2(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	cpu.V[x] = cpu.V[x] & cpu.V[y];
}

void op_8xy3(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	cpu.V[x] = cpu.V[x] ^ cpu.V[y];
}

void op_8xy4(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	std::uint16_t sum = cpu.V[x] + cpu.V[y];
	// std::cout << "sum = 0x" << std::hex << std::uppercase << sum << "\n";

	if (sum > 0xFF)
	{
		cpu.V[x] = static_cast<std::uint8_t>(sum & 0xFF);
		cpu.V[0xF] = 1; 
		// std::cout << "Vx = 0x" << std::hex << std::uppercase << static_cast<int>(cpu.V[x]) << "\n";

		return;
	}

	cpu.V[x] = cpu.V[x] + cpu.V[y];
	cpu.V[0xF] = 0;
}

void op_8xy5(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	if (cpu.V[x] >= cpu.V[y])
		cpu.V[0xF] = 1;
	else
		cpu.V[0xF] = 0;

	cpu.V[x] = cpu.V[x] - cpu.V[y];
}

void op_8xy6(Chip8& cpu, std::uint16_t opcode) // this instruction can cause undefined behavior since the community implement this in two different ways
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	std::uint8_t leastBit = cpu.V[y] & 0x01;

	cpu.V[x] = cpu.V[y] >> 1;
	cpu.V[0xF] = leastBit;
}

void op_8xy7(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	if (cpu.V[y] >= cpu.V[x])
		cpu.V[0xF] = 1;
	else
		cpu.V[0xF] = 0;

	cpu.V[x] = cpu.V[y] - cpu.V[x];
}

void op_8xyE(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	if (static_cast<bool>(cpu.V[x] & 0x80))
		cpu.V[0xF] = 1;
	else
		cpu.V[0xF] = 0;

	cpu.V[x] = cpu.V[x] << 1;
}

void op_9xy0(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;

	if (cpu.V[x] != cpu.V[y])
		cpu.pc += 2;
}

void op_Bnnn(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t nnn = opcode & Masks::nnn;

	cpu.pc = nnn + cpu.V[0x0];
}

void op_Cxkk(Chip8& cpu, std::uint16_t opcode)
{
	std::uint8_t randomNumber = Random::get(0, 255);
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t kk = opcode & Masks::kk;
	
	cpu.V[x] = randomNumber & kk;
}

void op_Dxyn(Chip8& cpu, std::uint16_t opcode) 
{	
	const int screenWidth{ 64 };
	const int screenHeight{ 32 };

	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint16_t y = (opcode & Masks::y) >> 4;
	std::uint16_t n = opcode & Masks::n;

	// debug lines
	// cpu.memory[cpu.I]     = 0xF0; // sprite 0 values
	// cpu.memory[cpu.I + 1] = 0x90;
	// cpu.memory[cpu.I + 2] = 0x90;
	// cpu.memory[cpu.I + 3] = 0x90;
	// cpu.memory[cpu.I + 4] = 0xF0;
	// cpu.V[x] = 30;
	// cpu.V[y] = 29;
	// end of debug

	cpu.V[0xF] = 0x00; // VF begins at value 0

	int xCoord{ cpu.V[x] };
	int yCoord{ cpu.V[y] };

	for (int j{ 0 }; j < n; ++j) // this loop executes n times, where n is equal to sprite's height
	{
		if (yCoord > screenHeight - 1) // checks if the next vertical pixel coordinates is out of the screen
			continue;

		int spriteInitialPos{ (screenWidth * yCoord) + xCoord };

		for (int i{ 0 }, shift{ 7 }; i < 8; ++i, --shift) // this loop executes 8 times, which is the numbers of bits of a std::uint8_t
		{
			std::uint8_t bit = (cpu.memory[cpu.I + j] >> shift) & 0x01; // isolate each individual bit

			if (spriteInitialPos + i > (screenWidth * screenHeight) - 1) // checks if spriteInitialPos + i is bigger than the display's last index (2047)
				continue;

			if (xCoord + i > screenWidth - 1) // checks if the next horizontal pixel coordinates is out of the screen
				continue;

			if ((cpu.display[spriteInitialPos + i] & 0x01) == 0x01 && (bit == 0x01)) // if any pixel is erased, then VF is set to 1, otherwise 0
				cpu.V[0xF] = 0x01;

			cpu.display[spriteInitialPos + i] ^= bit;

			// std::cout << static_cast<bool>(cpu.display[spriteInitialPos + i]) << " ";
		}

		++yCoord;
		// std::cout << "\n";
	}
}

void op_Ex9E(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint8_t key = cpu.V[x];

	if (key < 16 && cpu.keypad[key])
		cpu.pc += 2;
}

void op_ExA1(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	std::uint8_t key = cpu.V[x];

	if (key < 16 && !cpu.keypad[key])
		cpu.pc += 2;
}

void op_Fx07(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	cpu.V[x] = cpu.delayTimer;
}

void op_Fx0A(Chip8& cpu, std::uint16_t opcode)
{
	std::cout << "not finished\n";
}

void op_Fx15(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	cpu.delayTimer = cpu.V[x];
}

void op_Fx18(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	cpu.soundTimer = cpu.V[x];
}

void op_Fx1E(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	cpu.I += cpu.V[x];
}

void op_Fx29(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;
	cpu.I = 0x50 + (cpu.V[x] * 5);
}

void op_Fx33(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;

	std::uint8_t number = cpu.V[x];

	for (int digitCounter{ 2 }; digitCounter >= 0; --digitCounter)
	{
		std::uint8_t digit = number % 10; 

		cpu.memory[cpu.I + digitCounter] = digit; // get the digits in backwards order, so we store backwards

		number = (number - digit) / 10;
	}
}

void op_Fx55(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;

	for (int i{ 0 }; i <= x; ++i)
	{
		cpu.memory[cpu.I + i] = cpu.V[i];
	}
}

void op_Fx65(Chip8& cpu, std::uint16_t opcode)
{
	std::uint16_t x = (opcode & Masks::x) >> 8;

	for (int i{ 0 }; i <= x; ++i)
	{
		cpu.V[i] = cpu.memory[cpu.I + i];
	}
}
