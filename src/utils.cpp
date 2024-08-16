#include "utils.h"

#include <cassert>

void btw::wrapNort(uint64_t &bitBoard)
{
    bitBoard <<= 8;
}

void btw::wrapSout(uint64_t &bitBoard)
{
    bitBoard >>= 8;
}

void btw::wrapEast(uint64_t & bitBoard)
{
    const uint64_t mask = (uint64_t) 0x7f7f7f7f7f7f7f7f;
    bitBoard &= mask;
    bitBoard <<= 1;
}

void btw::wrapWest(uint64_t &bitBoard)
{
    const uint64_t mask = (uint64_t) 0xfefefefefefefefe;
    bitBoard &= mask;
    bitBoard >>= 1;
}

uint64_t btw::cpyWrapNort(uint64_t bitBoard){
    wrapNort(bitBoard);
    return bitBoard;
}

uint64_t btw::cpyWrapSout(uint64_t bitBoard){
    wrapSout(bitBoard);
    return bitBoard;
}

uint64_t btw::cpyWrapEast(uint64_t bitBoard){
    wrapEast(bitBoard);
    return bitBoard;
}


uint64_t btw::cpyWrapWest(uint64_t bitBoard){
    wrapWest(bitBoard);
    return bitBoard;
}

// Finds position of Least Significant 1 Bit
int btw::bitScanForward(uint64_t bitBoard){
    //Reference table for bitscans
    const int index64[64] = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (bitBoard != 0);
    return index64[((bitBoard ^ (bitBoard-1)) * deBrujin64) >> 58];
}

int btw::popCount(uint64_t bitBoard){
   int count = 0;
   while (bitBoard) {
       count++;
       bitBoard &= bitBoard - 1; // reset LS1B
   }
   return count;
}

// Finds position of Most Significant 1 Bit
int btw::bitScanReverse(uint64_t bitBoard){
    // Reference table for bitscans
    const int index64[64] = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (bitBoard != 0);
    bitBoard |= bitBoard >> 1;
    bitBoard |= bitBoard >> 2;
    bitBoard |= bitBoard >> 4;
    bitBoard |= bitBoard >> 8;
    bitBoard |= bitBoard >> 16;
    bitBoard |= bitBoard >> 32;
    return index64[(bitBoard * deBrujin64) >> 58];
}