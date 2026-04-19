#include "Chip8.h"
#include "Disassembler.h"

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

Chip8::Chip8()
{
    reset();
    loadFontSprites();
}

// before loading another rom, we want to clear the state of the machine
void Chip8::reset() 
{
    m_pc = 0x200; 

    std::fill(std::begin(m_V), std::end(m_V), 0);

    m_I = 0;
    m_sp = 0;

    std::fill(std::begin(m_stack), std::end(m_stack), 0);

    m_delayTimer = 0;
    m_soundTimer = 0;

    std::fill(std::begin(m_memory) + 80, std::end(m_memory), 0);

    std::fill(std::begin(m_display), std::end(m_display), 0);
}

void Chip8::loadFontSprites()
{
    const int fontInitialAddress{ 0x50 };

    if (m_memory[fontInitialAddress] != 0x00)
    {
        std::cerr << "Error. Font sprites can only be loaded one time.\n";
        return;
    }

    for (int i{ 0 }; i < 80; ++i) // 80 is the size of the fontSet array
    {
        m_memory[fontInitialAddress + i] = fontSet[i];
    }

    std::cout << "Font sprites loaded into m_memory.\n";
}

void Chip8::loadROM() // loads the rom into memory
{
    const int romAddress{ 0x200 };

    if (m_memory[romAddress + 1] != 0x00) // clears the memory if a file is already loaded into memory
        reset();

    std::ifstream file("roms/" + m_filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) 
    {
        std::streampos romSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (romSize > (4096 - 512)) // if the file being read is bigger than the available space in memory, then we don't want to load in the memory
        {
            std::cerr << "Error. ROM's size is bigger than the available space.\n";
            m_fileSize = -1;
            return;
        }

        file.read(reinterpret_cast<char*>(&m_memory[romAddress]), romSize);

        file.close();

        std::cout << m_filename << " was successfully loaded into memory.\n";

        m_fileSize = romSize; // returns the file's size
        return;
    }

    std::cerr << "Error. File not loaded into memory.\n";

    m_fileSize = -1; 
}

std::vector<std::uint8_t> Chip8::getDisplay()
{   
    std::vector<std::uint8_t> vec_display(64 * 32 * 4);

    for (int i{ 0 }; i < 64 * 32; ++i)
    {
        if (m_display[i] == 1)
        {
            vec_display[i * 4]     = 255;
            vec_display[i * 4 + 1] = 255;
            vec_display[i * 4 + 2] = 255;
            vec_display[i * 4 + 3] = 255;
        }

        if (m_display[i] == 0)
        {
            vec_display[i * 4]     = 0;
            vec_display[i * 4 + 1] = 0;
            vec_display[i * 4 + 2] = 0;
            vec_display[i * 4 + 3] = 255;
        }
    }

    return vec_display;
}

std::vector<std::string> Chip8::getMemoryContent() const // print all opcodes loaded into memory
{
    if (m_fileSize == -1) // if fileSize = -1, then there was an error when loading the file into memory
    {
        std::cerr << "Error. A file is not loaded into memory.\n";
        return {};
    }

    const int romAddress{ 0x200 }; // roms are loaded from 0x200 onward in chip8
    std::vector<std::string> memoryContent{};

    for (int address{ romAddress }; address < romAddress + m_fileSize; address += 2)
    {
        std::uint16_t opcode = (m_memory[address] << 8) | m_memory[address + 1];

        memoryContent.push_back(Disassembler::hexToString(address, 4) + ": " + Disassembler::disassembler(opcode));
    }

    return memoryContent;
}

std::string Chip8::getCallStack(int index) const
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << "Stack#" << index << ": 0x" << std::setw(4) << std::setfill('0') << m_stack[index];

    std::string hexString{ ss.str() };

    return hexString;
}

std::string Chip8::getRegister(int index) const
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << "V" << index << ": 0x" << std::setw(2) << std::setfill('0') << static_cast<int>(m_V[index]);

    std::string hexString{ ss.str() };

    return hexString;
}

// getters and setters

void          Chip8::setState(State emuState) { m_state = emuState; }
Chip8::State  Chip8::getState() const { return m_state; }

void          Chip8::setKeyBeingPressed(std::uint8_t key) { m_keyBeingPressed = key; }
std::uint8_t  Chip8::getKeyBeingPressed() const { return m_keyBeingPressed; }

void          Chip8::setKeypad(int index, std::uint8_t key) { m_keypad[index] = key; }
std::uint8_t  Chip8::getKeypad(int index) const { return m_keypad[index]; }

void          Chip8::waitForKey(bool b) { m_waitForAKeyPress = b; }
bool          Chip8::isWaitingForKey() const { return m_waitForAKeyPress; }

void          Chip8::decrementDelayTimer() { --m_delayTimer; }
std::uint8_t  Chip8::getDelayTimer() const { return m_delayTimer; }

void          Chip8::decrementSoundTimer() { --m_soundTimer; }
std::uint8_t  Chip8::getSoundTimer() const { return m_soundTimer; }

std::uint16_t Chip8::getPC() const { return m_pc; }
std::uint8_t  Chip8::getSP() const { return m_sp; }
std::uint16_t Chip8::getI() const { return m_I; }

void          Chip8::setFilename(const std::string& file) { m_filename = file; }
std::string   Chip8::getFilename() { return m_filename; }
