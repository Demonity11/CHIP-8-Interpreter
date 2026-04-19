#include "Chip8.h"
#include "disassembler.h"

constexpr std::uint8_t fontSet[]
{
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
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
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
}; 

// before loading another rom, we want to clear the state of the machine
void Chip8::reset() 
{
    pc = 0x200; 

    std::fill(std::begin(V), std::end(V), 0);

    I = 0;
    sp = 0;

    std::fill(std::begin(stack), std::end(stack), 0);

    delayTimer = 0;
    soundTimer = 0;

    std::fill(std::begin(memory) + 80, std::end(memory), 0);

    std::fill(std::begin(display), std::end(display), 0);
}

void Chip8::loadFontSprites()
{
    const int fontInitialAddress{ 0x50 };

    if (memory[fontInitialAddress] != 0x00)
    {
        std::cerr << "Error. Font sprites can only be loaded one time.\n";
        return;
    }

    for (int i{ 0 }; i < 80; ++i) // 80 is the size of the fontSet array
    {
        memory[fontInitialAddress + i] = fontSet[i];
    }

    std::cout << "Font sprites loaded into memory.\n";
}

void Chip8::loadROM() // loads the rom into memory
{
    const int romAddress{ 0x200 };

    if (memory[romAddress + 1] != 0x00) // clears the memory if a file is already loaded into memory
        reset();

    std::ifstream file("roms/" + filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) 
    {
        std::streampos romSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (romSize > (4096 - 512)) // if the file being read is bigger than the available space in memory, then we don't want to load in the memory
        {
            std::cerr << "Error. ROM's size is bigger than the available space.\n";
            fileSize = -1;
            return;
        }

        file.read(reinterpret_cast<char*>(&memory[romAddress]), romSize);

        file.close();

        std::cout << filename << " was successfully loaded into memory.\n";

        fileSize = romSize; // returns the file's size
        return;
    }

    std::cerr << "Error. File not loaded into memory.\n";

    fileSize = -1; 
}

std::vector<std::uint8_t> Chip8::getDisplay()
{   
    std::vector<std::uint8_t> vec_display(64 * 32 * 4);

    for (int i{ 0 }; i < 64 * 32; ++i)
    {
        if (display[i] == 1)
        {
            vec_display[i * 4]     = 255;
            vec_display[i * 4 + 1] = 255;
            vec_display[i * 4 + 2] = 255;
            vec_display[i * 4 + 3] = 255;
        }

        if (display[i] == 0)
        {
            vec_display[i * 4]     = 0;
            vec_display[i * 4 + 1] = 0;
            vec_display[i * 4 + 2] = 0;
            vec_display[i * 4 + 3] = 255;
        }
    }

    return vec_display;
}

void Chip8::printDisplay() // print the contents of the display array
{
    const int displaySize{ 64 * 32 };

    for (int i{ 0 }; i < displaySize; ++i)
    {
        std::cout << (display[i] == 1 ? static_cast<char>(219) : ' '); // 219 = █ in ASCII Table

        if ((i + 1) % 64 == 0)
            std::cout << "\n"; // ends line when each row is printed
    }
}

std::vector<std::string> Chip8::getMemoryContent() const // print all opcodes loaded into memory
{
    if (fileSize == -1) // if fileSize = -1, then there was an error when loading the file into memory
    {
        std::cerr << "Error. A file is not loaded into memory.\n";
        return {};
    }

    const int romAddress{ 0x200 }; // roms are loaded from 0x200 onward in chip8
    std::vector<std::string> memoryContent{};

    for (int address{ romAddress }; address < romAddress + fileSize; address += 2)
    {
        std::uint16_t opcode = (memory[address] << 8) | memory[address + 1];

        memoryContent.push_back(hexToString(address, 4) + ": " + disassembler(opcode));
    }

    return memoryContent;
}

std::string Chip8::getCallStack(int index) const
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << "Stack #" << index << stack[index];

    std::string hexString{ ss.str() };

    return hexString;
}

std::string Chip8::getRegister(int index) const
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << "V" << index << ": 0x" << std::setw(2) << std::setfill('0') << V[index];

    std::string hexString{ ss.str() };

    return hexString;
}

void Chip8::init()
{
    loadFontSprites();
    loadROM();
}