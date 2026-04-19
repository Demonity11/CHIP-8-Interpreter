#include "Opcodes.h"
#include "Utilities.h"
#include "Chip8.h"
#include "Random.h"

void Opcodes::op_00E0(Chip8& cpu)
{
    std::fill(std::begin(cpu.display), std::end(cpu.display), 0);
}

void Opcodes::op_00EE(Chip8& cpu) 
{
    if (cpu.sp > 0) 
    {
        --cpu.sp;
        cpu.pc = cpu.stack[cpu.sp];
    } 
}

void Opcodes::op_Annn(Chip8& cpu)
{
    cpu.I = opcode & Masks::nnn;
}

void Opcodes::op_1nnn(Chip8& cpu)
{
    cpu.pc = opcode & Masks::nnn;
}

void Opcodes::op_2nnn(Chip8& cpu) 
{
    if (cpu.sp < 15) 
    { 
        cpu.stack[cpu.sp] = cpu.pc;
        ++cpu.sp;
        cpu.pc = opcode & Masks::nnn;
    } 
}

void Opcodes::op_3xkk(Chip8& cpu)
{
    std::uint16_t x  = (opcode & Masks::x) >> 8;
    std::uint16_t kk = opcode & Masks::kk;

    if (cpu.V[x] == kk)
        cpu.pc += 2;
}

void Opcodes::op_4xkk(Chip8& cpu)
{
    std::uint16_t x  = (opcode & Masks::x) >> 8;
    std::uint16_t kk = opcode & Masks::kk;

    if (cpu.V[x] != kk)
        cpu.pc += 2;
}

void Opcodes::op_5xy0(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    if (cpu.V[x] == cpu.V[y])
        cpu.pc += 2;
}

void Opcodes::op_6xkk(Chip8& cpu)
{
    std::uint16_t x  = (opcode & Masks::x) >> 8;
    std::uint16_t kk = opcode & Masks::kk;

    cpu.V[x] = static_cast<std::uint8_t>(kk);
}

void Opcodes::op_7xkk(Chip8& cpu)
{
    std::uint16_t x  = (opcode & Masks::x) >> 8;
    std::uint16_t kk = opcode & Masks::kk;

    cpu.V[x] += static_cast<std::uint8_t>(kk);
}

void Opcodes::op_8xy0(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    cpu.V[x] = cpu.V[y];
}

void Opcodes::op_8xy1(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    cpu.V[x] = cpu.V[x] | cpu.V[y];
}

void Opcodes::op_8xy2(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    cpu.V[x] = cpu.V[x] & cpu.V[y];
}

void Opcodes::op_8xy3(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    cpu.V[x] = cpu.V[x] ^ cpu.V[y];
}

void Opcodes::op_8xy4(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    std::uint16_t sum = cpu.V[x] + cpu.V[y];

    if (sum > 0xFF)
    {
        cpu.V[x] = static_cast<std::uint8_t>(sum & 0xFF);
        cpu.V[0xF] = 1; 
        return;
    }

    cpu.V[x] = cpu.V[x] + cpu.V[y];
    cpu.V[0xF] = 0;
}

void Opcodes::op_8xy5(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    if (cpu.V[x] >= cpu.V[y])
        cpu.V[0xF] = 1;
    else
        cpu.V[0xF] = 0;

    cpu.V[x] = cpu.V[x] - cpu.V[y];
}

void Opcodes::op_8xy6(Chip8& cpu) 
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    std::uint8_t leastBit = cpu.V[y] & 0x01;

    cpu.V[x] = cpu.V[y] >> 1;
    cpu.V[0xF] = leastBit;
}

void Opcodes::op_8xy7(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    if (cpu.V[y] >= cpu.V[x])
        cpu.V[0xF] = 1;
    else
        cpu.V[0xF] = 0;

    cpu.V[x] = cpu.V[y] - cpu.V[x];
}

void Opcodes::op_8xyE(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;

    if (static_cast<bool>(cpu.V[x] & 0x80))
        cpu.V[0xF] = 1;
    else
        cpu.V[0xF] = 0;

    cpu.V[x] = cpu.V[x] << 1;
}

void Opcodes::op_9xy0(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;

    if (cpu.V[x] != cpu.V[y])
        cpu.pc += 2;
}

void Opcodes::op_Bnnn(Chip8& cpu)
{
    std::uint16_t nnn = opcode & Masks::nnn;
    cpu.pc = nnn + cpu.V[0x0];
}

void Opcodes::op_Cxkk(Chip8& cpu)
{
    std::uint8_t randomNumber = Random::get(0, 255);
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t kk = opcode & Masks::kk;
    
    cpu.V[x] = randomNumber & kk;
}

void Opcodes::op_Dxyn(Chip8& cpu) 
{   
    const int screenWidth{ 64 };
    const int screenHeight{ 32 };

    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint16_t y = (opcode & Masks::y) >> 4;
    std::uint16_t n = opcode & Masks::n;

    cpu.V[0xF] = 0x00; 

    int xCoord{ cpu.V[x] };
    int yCoord{ cpu.V[y] };

    for (int j{ 0 }; j < n; ++j) 
    {
        if (yCoord > screenHeight - 1) 
            continue;

        int spriteInitialPos{ (screenWidth * yCoord) + xCoord };

        for (int i{ 0 }, shift{ 7 }; i < 8; ++i, --shift) 
        {
            std::uint8_t bit = (cpu.memory[cpu.I + j] >> shift) & 0x01; 

            if (spriteInitialPos + i > (screenWidth * screenHeight) - 1) 
                continue;

            if (xCoord + i > screenWidth - 1) 
                continue;

            if ((cpu.display[spriteInitialPos + i] & 0x01) == 0x01 && (bit == 0x01)) 
                cpu.V[0xF] = 0x01;

            cpu.display[spriteInitialPos + i] ^= bit;
        }
        ++yCoord;
    }
}

void Opcodes::op_Ex9E(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint8_t key = cpu.V[x];

    if (key < 16 && cpu.keypad[key])
    {
        cpu.keypad[key] = 0x0;
        cpu.keyBeingPressed = 0xFF;
        cpu.pc += 2;
    }
}

void Opcodes::op_ExA1(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint8_t key = cpu.V[x];

    if (key < 16 && cpu.keypad[key] == 0x0)
    {
        cpu.pc += 2;
    }
}

void Opcodes::op_Fx07(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.V[x] = cpu.delayTimer;
}

void Opcodes::op_Fx0A(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.waitForAKeyPress = true;

    if (cpu.keyBeingPressed >= 0x0 && cpu.keyBeingPressed <= 0xF)
    {
        cpu.V[x] = cpu.keyBeingPressed;
        cpu.keypad[cpu.keyBeingPressed] = 0x0;
        cpu.keyBeingPressed = 0xFF;
        cpu.waitForAKeyPress = false;
    }
}

void Opcodes::op_Fx15(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.delayTimer = cpu.V[x];
}

void Opcodes::op_Fx18(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.soundTimer = cpu.V[x];
}

void Opcodes::op_Fx1E(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.I += cpu.V[x];
}

void Opcodes::op_Fx29(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    cpu.I = 0x50 + (cpu.V[x] * 5);
}

void Opcodes::op_Fx33(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;
    std::uint8_t number = cpu.V[x];

    for (int digitCounter{ 2 }; digitCounter >= 0; --digitCounter)
    {
        std::uint8_t digit = number % 10; 
        cpu.memory[cpu.I + digitCounter] = digit; 
        number = (number - digit) / 10;
    }
}

void Opcodes::op_Fx55(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;

    for (int i{ 0 }; i <= x; ++i)
    {
        cpu.memory[cpu.I + i] = cpu.V[i];
    }
}

void Opcodes::op_Fx65(Chip8& cpu)
{
    std::uint16_t x = (opcode & Masks::x) >> 8;

    for (int i{ 0 }; i <= x; ++i)
    {
        cpu.V[i] = cpu.memory[cpu.I + i];
    }
}

void Opcodes::fetch(Chip8& cpu)
{
    std::uint16_t highByte = (cpu.memory[cpu.pc] << 8);
    std::uint16_t lowByte = cpu.memory[cpu.pc + 1];
    
    opcode = highByte | lowByte;
    
    cpu.pc += 2;
}

void Opcodes::decode(Chip8& cpu)
{
    std::uint16_t firstNibble = (opcode >> 12) & 0x000F;
    std::uint16_t lastNibble = opcode & 0x000F;

    switch (firstNibble)
    {
    case 0x0:
        if (opcode == 0x00E0) op_00E0(cpu);

        if (opcode == 0x00EE) op_00EE(cpu);

        break;

    case 0xA: op_Annn(cpu); break;

    case 0x1: op_1nnn(cpu); break;

    case 0x2: op_2nnn(cpu); break;

    case 0x3: op_3xkk(cpu); break;

    case 0x4: op_4xkk(cpu); break;
    
    case 0x5: op_5xy0(cpu); break;

    case 0x6: op_6xkk(cpu); break;

    case 0x7: op_7xkk(cpu); break;
    
    case 0x8:
        switch (lastNibble)
        {
        case 0x0: op_8xy0(cpu); break;

        case 0x1: op_8xy1(cpu); break;
        
        case 0x2: op_8xy2(cpu); break;
            
        case 0x3: op_8xy3(cpu); break;

        case 0x4: op_8xy4(cpu); break;

        case 0x5: op_8xy5(cpu); break;

        case 0x6: op_8xy6(cpu); break;

        case 0x7: op_8xy7(cpu); break;

        case 0xE: op_8xyE(cpu); break;
        }
        break;

    case 0x9: op_9xy0(cpu); break;

    case 0xB: op_Bnnn(cpu); break;

    case 0xC: op_Cxkk(cpu); break;

    case 0xD: op_Dxyn(cpu); break;
    
    case 0xE:
        if (lastNibble == 0xE) op_Ex9E(cpu);
        if (lastNibble == 0x1) op_ExA1(cpu);
        
        break;

    case 0xF:
        switch (opcode & 0x00FF)
        {
        case 0x07: op_Fx07(cpu); break;

        case 0x0A: op_Fx0A(cpu); break;

        case 0x15: op_Fx15(cpu); break;

        case 0x55: op_Fx55(cpu); break;

        case 0x65: op_Fx65(cpu); break;

        case 0x18: op_Fx18(cpu); break;

        case 0x1E: op_Fx1E(cpu); break;

        case 0x29: op_Fx29(cpu); break;

        case 0x33: op_Fx33(cpu); break;
        }
    }
}