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

uint64_t cpyWrapEast(uint64_t t_bitBoard){
    wrapEast(t_bitBoard);
    return t_bitBoard;
}

uint64_t cpyWrapWest(uint64_t t_bitBoard){
    wrapWest(t_bitBoard);
    return t_bitBoard;
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


void initKnightAttacks(uint64_t *m_knightAttacks){
    uint64_t knightPos = (uint64_t) 1;
    for (int sq = 0; sq <64; sq ++, knightPos <<= 1){
        m_knightAttacks[sq] = (uint64_t) 0;

        uint64_t eastDir = cpyWrapEast(knightPos);
        uint64_t eaEaDir = cpyWrapEast(eastDir);
        m_knightAttacks[sq] |= eastDir << 16 | eaEaDir << 8 
            | eastDir >> 16 | eaEaDir >> 8;

        uint64_t westDir = cpyWrapWest(knightPos);
        uint64_t weWeDir = cpyWrapWest(westDir);
        m_knightAttacks[sq] |= westDir << 16 | weWeDir << 8
            | westDir >> 16 | weWeDir >> 8;

    }
}

void initKingAttacks(){
    uint64_t m_kingAttacks[64];
    uint64_t kingPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, kingPosition <<= 1){
        m_kingAttacks[sq] = kingPosition << 8 | kingPosition >> 8;
        uint64_t eastDir = cpyWrapEast(kingPosition);
        m_kingAttacks[sq] |= eastDir | eastDir << 8 | eastDir >> 8;
        uint64_t westDir = cpyWrapWest(kingPosition);
        m_kingAttacks[sq] |= westDir | westDir << 8 | westDir >> 8;
        std::bitset<64> rep(m_kingAttacks[sq]);
        std::cout  << sq << ") " << rep << std::endl;
    }
}

int main(){
    Board cBoard;
    uint64_t knightAttacks[64];

    initKnightAttacks(knightAttacks);

    for (int sq = 0; sq < 64; sq ++){
        std::bitset<64> rep(knightAttacks[sq]);
        std::cout << sq << ") " << rep << std::endl;
    }

    std::cout << "loop-end" << std::endl;

    return 0;
}
