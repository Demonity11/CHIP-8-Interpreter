#include "../include/chip8.h"

constexpr std::uint8_t fontSet[]{0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
								 0x20, 0x60, 0x20, 0x20, 0x70,  // 1
								 0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
								 0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
								 0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
								 0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
								 0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
								 0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
								 0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
								 0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
								 0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
								 0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
								 0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
								 0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
								 0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
								 0xF0, 0x80, 0xF0, 0x80, 0x80}; // F

std::uint16_t fetch(Chip8& cpu)
{
    std::uint16_t highByte = (cpu.memory[cpu.pc] << 8);
    std::uint16_t lowByte = cpu.memory[cpu.pc + 1];
    
    std::uint16_t opcode = highByte | lowByte;

    // std::cout << "opcode = 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << opcode << "\n";
    
    cpu.pc += 2;
    
    // std::cout << "pc = 0x" << std::hex << std::uppercase << cpu.pc << "\n\n"; 

    return opcode;
}

void decode(Chip8& cpu, std::uint16_t opcode)
{
    std::uint16_t firstNibble = (opcode >> 12) & 0x000F;
    std::uint16_t lastNibble = opcode & 0x000F;

    switch (firstNibble)
    {
    case 0x0:
        if (opcode == 0x00E0) op_00E0(cpu, opcode);

        if (opcode == 0x00EE) op_00EE(cpu, opcode);

        break;

    case 0xA: op_Annn(cpu, opcode); break;

    case 0x1: op_1nnn(cpu, opcode); break;

    case 0x2: op_2nnn(cpu, opcode); break;

    case 0x3: op_3xkk(cpu, opcode); break;

    case 0x4: op_4xkk(cpu, opcode); break;
    
    case 0x5: op_5xy0(cpu, opcode); break;

    case 0x6: op_6xkk(cpu, opcode); break;

    case 0x7: op_7xkk(cpu, opcode); break;
    
    case 0x8:
        switch (lastNibble)
        {
        case 0x0: op_8xy0(cpu, opcode); break;

        case 0x1: op_8xy1(cpu, opcode); break;
        
        case 0x2: op_8xy2(cpu, opcode); break;
            
        case 0x3: op_8xy3(cpu, opcode); break;

        case 0x4: op_8xy4(cpu, opcode); break;

        case 0x5: op_8xy5(cpu, opcode); break;

        case 0x6: op_8xy6(cpu, opcode); break;

        case 0x7: op_8xy7(cpu, opcode); break;

        case 0xE: op_8xyE(cpu, opcode); break;
        }
        break;

    case 0x9: op_9xy0(cpu, opcode); break;

    case 0xB: op_Bnnn(cpu, opcode); break;

    case 0xC: op_Cxkk(cpu, opcode); break;

    case 0xD: op_Dxyn(cpu, opcode); break;
    
    case 0xE:
        if (lastNibble == 0xE) op_Ex9E(cpu, opcode);
        if (lastNibble == 0x1) op_ExA1(cpu, opcode);
        
        break;

    case 0xF:
        switch (opcode & 0x00FF)
        {
        case 0x07: op_Fx07(cpu, opcode); break;

        case 0x0A: op_Fx0A(cpu, opcode); break;

        case 0x15: op_Fx15(cpu, opcode); break;

        case 0x55: op_Fx55(cpu, opcode); break;

        case 0x65: op_Fx65(cpu, opcode); break;

        case 0x18: op_Fx18(cpu, opcode); break;

        case 0x1E: op_Fx1E(cpu, opcode); break;

        case 0x29: op_Fx29(cpu, opcode); break;

        case 0x33: op_Fx33(cpu, opcode); break;
        }
    }
}

void clearMemory(Chip8& cpu) // before loading another rom, we want to clear the memory
{
    std::fill(std::begin(cpu.memory), std::end(cpu.memory), 0);
}

void loadFontSprites(Chip8& cpu)
{
    const int fontInitialAddress{ 0x50 };

    if (cpu.memory[fontInitialAddress] != 0x00)
    {
        std::cerr << "Error. Font sprites can only be loaded one time.\n";
        return;
    }

    for (int i{ 0 }; i < 80; ++i) // 80 is the size of the fontSet array
    {
        cpu.memory[fontInitialAddress + i] = fontSet[i];
    }

    std::cout << "Font sprites loaded into memory.\n";
}

int loadROM(Chip8& cpu, const std::string& filename) // loads the rom into memory
{
    const int romAddress{ 0x200 };

    if (cpu.memory[romAddress + 1] != 0x00) // clears the memory if a file is already loaded into memory
        clearMemory(cpu);

    std::ifstream file("../roms/" + filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) 
    {
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (fileSize > (4096 - 512)) // if the file being read is bigger than the available space in memory, then we don't want to load in the memory
        {
            std::cerr << "Error. ROM's size is bigger than the available space.\n";
            return -1;
        }

        file.read(reinterpret_cast<char*>(&cpu.memory[romAddress]), fileSize);

        file.close();

        std::cout << filename << " was successfully loaded into memory.\n";

        return fileSize; // returns the file's size
    }

    std::cerr << "Error. File not loaded into memory.\n";

    return -1; 
}

void printROM(const Chip8& cpu, int fileSize) // print all opcodes loaded into memory
{
    if (fileSize == -1) // if fileSize = -1, then there was an error when loading the file into memory
    {
        std::cerr << "Error. A file is not loaded into memory.\n";
        return;
    }

    const int romAddress{ 0x200 }; // roms are loaded from 0x200 onward in chip8

    for (int i{ romAddress }; i < romAddress + fileSize; i += 2)
    {
        std::uint16_t opcode = (cpu.memory[i] << 8) | cpu.memory[i + 1];

        std::cout << "opcode: 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << opcode;
        std::cout << " address: 0x" << std::hex << std::uppercase << std::setw(3) << std::setfill('0') << i << "\n";
    }
}

std::vector<std::uint8_t> getDisplay(const Chip8& cpu)
{   
    std::vector<std::uint8_t> display(64 * 32 * 4);

    for (int i{ 0 }; i < 64 * 32; ++i)
    {
        if (cpu.display[i] == 1)
        {
            display[i * 4] = 255;
            display[i * 4 + 1] = 255;
            display[i * 4 + 2] = 255;
            display[i * 4 + 3] = 255;
        }

        if (cpu.display[i] == 0)
        {
            display[i * 4] = 0;
            display[i * 4 + 1] = 0;
            display[i * 4 + 2] = 0;
            display[i * 4 + 3] = 255;
        }
    }

    // for (int i{ 0 }; i < 64 * 32; ++i)
    // {
    //     std::cout << (display.at(i*4) == 255 ? static_cast<char>(219) : ' '); // 219 = █ in ASCII Table

    //     if ((i + 1) % 64 == 0)
    //         std::cout << "\n"; // ends line when each row is printed
    // }

    return display;
}

void printDisplay(const Chip8& cpu) // print the contents of the display array
{
    const int displaySize{ 64 * 32 };

    for (int i{ 0 }; i < displaySize; ++i)
    {
        std::cout << (cpu.display[i] == 1 ? static_cast<char>(219) : ' '); // 219 = █ in ASCII Table

        if ((i + 1) % 64 == 0)
            std::cout << "\n"; // ends line when each row is printed
    }
}

Chip8 init(std::string romName)
{
    Chip8 cpu{};
    
	loadROM(cpu, romName);

    loadFontSprites(cpu);

    return cpu;
}

// int main()
// {
//     Chip8 cpu{ init("IBM Logo.ch8") };

//     for (int i{0}; i < 200; ++i)
//     {
//         auto opcode = fetch(cpu);
//         decode(cpu, opcode);
//     }

//     std::vector<std::uint8_t> display{ getDisplay(cpu) };

//     return 0;
// }
