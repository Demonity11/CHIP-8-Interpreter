#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdint>

namespace Masks
{
	constexpr std::uint16_t nnn{ 0x0FFF }; // extracts the lower 12 bits
	constexpr std::uint16_t n{ 0x000F }; // extracts the lower 4 bits
	constexpr std::uint16_t x{ 0x0F00 }; // extracts the lower byte of the highbyte
	constexpr std::uint16_t y{ 0x00F0 }; // extracts the higher byte of the lowbyte
	constexpr std::uint16_t kk{0x00FF}; // extracts the lower 8 bits
}

struct FPS
{
    double accumulator{ 0 };
    int frames{ 0 };
    double average{};
};

#endif
