#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <cstdint>
#include <string>

auto hexToString(std::uint16_t hex, int size) 		  -> std::string;
auto getRegisterName(std::uint16_t regIndex)  		  -> std::string;
auto getOpcodeConvertedToString(std::uint16_t opcode) -> std::string;
auto disassembler(std::uint16_t opcode) 			  -> std::string;
auto getFPS(double averageFPS)						  -> std::string;

#endif