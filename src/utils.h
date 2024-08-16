#pragma once

#include <cstdint>

// Bit TWiddling functions
namespace btw
{
    void wrapNort(uint64_t &bitBoard);
    void wrapSout(uint64_t &bitBoard);
    void wrapEast(uint64_t &bitBoard);
    void wrapWest(uint64_t &bitBoard);
    uint64_t cpyWrapNort(uint64_t bitBoard);
    uint64_t cpyWrapSout(uint64_t bitBoard);
    uint64_t cpyWrapEast(uint64_t bitBoard);
    uint64_t cpyWrapWest(uint64_t bitBoard);
    int bitScanReverse(uint64_t bitBoard);
    int bitScanForward(uint64_t bitBoard);
    int popCount(uint64_t bitBoard);
}; // namespace btw
