#include "chip8.h"
#include <sstream>

std::string hexToString(std::uint16_t hex, int size)
{
    std::stringstream ss{};

    ss << std::hex << std::setw(size) << std::setfill('0') << std::uppercase << hex;

    std::string hexString{ "0x" + ss.str() };

    return hexString;
}

std::string getRegisterName(std::uint16_t regIndex)
{
    const char* hexDigits{ "0123456789ABCDEF" };
    return std::string("V") + hexDigits[regIndex & 0xF];
}

std::string getOpcodeConvertedToString(std::uint16_t opcode)
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << opcode;

    std::string hexString{ "0x" + ss.str() };

    return hexString;
}

std::string getCallStack(const Chip8& cpu, int index)
{
    std::stringstream ss{};

    ss << std::hex << std::uppercase << cpu.stack[index];

    std::string hexString{ "0x" + ss.str() };

    return hexString;
}

std::string disassembler(std::uint16_t opcode)
{
    std::uint16_t firstNibble = (opcode >> 12) & 0x000F;
    std::uint16_t lastNibble = opcode & 0x000F;

    std::string instruction{ getOpcodeConvertedToString(opcode) };

    switch (firstNibble)
    {
    case 0x0:
        if (opcode == 0x00E0) instruction += " - CLS";
        if (opcode == 0x00EE) instruction += " - RET";

        return instruction;

    case 0xA: 
    {
        const std::string nnn{ hexToString(opcode & Masks::nnn, 3) };
        
        instruction += " - LD I, " + nnn; 
        return instruction;
    }

    case 0x1: 
    {
        const std::string nnn{ hexToString(opcode & Masks::nnn, 3) };
        
        instruction += " - JP " + nnn; 
        return instruction;
    }

    case 0x2: 
    {
        const std::string nnn{ hexToString(opcode & Masks::nnn, 3) };
        
        instruction += " - CALL " + nnn; 
        return instruction;
    }

    case 0x3: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string kk{ hexToString(opcode & Masks::kk, 2) };

        instruction += " - SE " + regX + ", " + kk;
        return instruction;
    }

    case 0x4: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string kk{ hexToString(opcode & Masks::kk, 2) };
        
        instruction += " - SNE " + regX + ", " + kk;
        return instruction;
    }
    
    case 0x5: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

        instruction += " - SE " + regX + ", " + regY;
        return instruction;
    }

    case 0x6: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string kk{ hexToString(opcode & Masks::kk, 2) };

        instruction += " - LD " + regX + ", " + kk;  
        return instruction;
    }

    case 0x7: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string kk{ hexToString(opcode & Masks::kk, 2) };

        instruction += " - ADD " + regX + ", " + kk;  
        return instruction;
    }
    
    case 0x8:
        switch (lastNibble)
        {
        case 0x0: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - LD " + regX + ", " + regY;
            return instruction;
        }

        case 0x1: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - OR " + regX + ", " + regY;
            return instruction;
        }
        
        case 0x2: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - AND " + regX + ", " + regY;
            return instruction;
        }
            
        case 0x3: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - XOR " + regX + ", " + regY;
            return instruction;
        }

        case 0x4: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - ADD " + regX + ", " + regY;
            return instruction;
        }

        case 0x5: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - SUB " + regX + ", " + regY;
            return instruction;
        }

        case 0x6: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - SHR " + regX + "{, " + regY + "}";
            return instruction;
        }

        case 0x7: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - SUBN " + regX + ", " + regY;
            return instruction;
        }

        case 0xE: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

            instruction += " - SHL " + regX + "{, " + regY + "}";
            return instruction;
        }
        }
        break;

    case 0x9: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };

        instruction += " - SNE " + regX + ", " + regY;
        return instruction;
    }

    case 0xB: 
    {
        const std::string nnn{ hexToString(opcode & Masks::nnn, 3) };

        instruction += " - JP V0, " + nnn;
        return instruction;
    }

    case 0xC: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string kk{ hexToString(opcode & Masks::kk, 2) };

        instruction += " - RND " + regX + ", " + kk;
        return instruction;
    }

    case 0xD: 
    {
        const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
        const std::string regY{ getRegisterName((opcode & Masks::y) >> 4) };
        const std::string n{ hexToString(opcode & Masks::n, 1) };

        instruction += " - DRW " + regX + ", " + regY + ", " + n;
        return instruction;
    }
    
    case 0xE:
        if (lastNibble == 0xE) 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            
            instruction += " - SKP " + regX;
        }

        if (lastNibble == 0x1) 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };
            
            instruction += " - SKNP " + regX;
        }
        
        return instruction;

    case 0xF:
        switch (opcode & 0x00FF)
        {
        case 0x07: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD " + regX + ", DT";
            return instruction;
        }

        case 0x0A: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD " + regX + ", K";
            return instruction;
        }

        case 0x15: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD DT, " + regX;
            return instruction;
        }

        case 0x55: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD [I], " + regX;
            return instruction;
        }

        case 0x65: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD " + regX + ", [I]";
            return instruction;
        }

        case 0x18: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD ST, " + regX;
            return instruction;
        }

        case 0x1E: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - ADD I, " + regX;
            return instruction;
        }

        case 0x29: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD F, " + regX;
            return instruction;
        }

        case 0x33: 
        {
            const std::string regX{ getRegisterName((opcode & Masks::x) >> 8) };

            instruction += " - LD B, " + regX;
            return instruction;
        }
        }

    default:
    {
        return std::string("???");
    }
    }
    return std::string("???");
}