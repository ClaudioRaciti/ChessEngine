#include <bitset>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <cassert>

#include "chessBoard.h"

void wrapEast(uint64_t &b){
    uint64_t mask = (uint64_t) 0x7f7f7f7f7f7f7f7f;
    b &= mask;
    b <<= 1;
}

void wrapWest(uint64_t &b){
    uint64_t mask = (uint64_t) 0xfefefefefefefefe;
    b &= mask;
    b >>= 1;
}

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

// Finds position of Least Significant 1 Bit
int bitScanForward(uint64_t bitBoard){
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (bitBoard != 0);
    return index64[((bitBoard ^ (bitBoard-1)) * deBrujin64) >> 58];
}

int bitScanReverse(uint64_t t_bitBoard){
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (t_bitBoard != 0);
    t_bitBoard |= t_bitBoard >> 1;
    t_bitBoard |= t_bitBoard >> 2;
    t_bitBoard |= t_bitBoard >> 4;
    t_bitBoard |= t_bitBoard >> 8;
    t_bitBoard |= t_bitBoard >> 16;
    t_bitBoard |= t_bitBoard >> 32;
    return index64[(t_bitBoard * deBrujin64) >> 58];
}



int main(){
    Board cBoard;
    uint64_t knightAttacks[64];

    std::cout << "loop-end" << std::endl;

    return 0;
    //for (int row = 0; row < 8; row ++){
    //    for (int col = 0; col < 8; col ++){
    //        intersection = whiteRooks & (1 << (row * 8 + col));
    //        if (intersection)
    //            std::cout << 1;
    //        else
    //            std::cout << 0;
    //    }
    //    std::cout << std::endl;
    //}
}
