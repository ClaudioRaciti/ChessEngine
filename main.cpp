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


void initKnightAttacks(uint64_t *m_knightAttacks){
    for (int sq = 0; sq < 64; sq ++)
        m_knightAttacks[sq] = (uint64_t) 0;

    // // East-North-East
    uint64_t eaNoEaDir = (uint64_t) 1 << 10;
    for (int file = 0; file < 8; file ++, wrapEast(eaNoEaDir)){
        uint64_t wrappedDir = eaNoEaDir;
        for (int rank = 0; rank < 7; rank ++, wrappedDir <<= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // North-East-North
    uint64_t noEaNoDir = (uint64_t) 1 << 17;
    for (int file = 0; file < 8; file ++, wrapEast(noEaNoDir)){
        uint64_t wrappedDir = noEaNoDir;
        for (int rank = 0; rank < 6; rank ++, wrappedDir <<= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // West-North-West
    uint64_t weNoWeDir = (uint64_t) 1 << 13;
    for (int file = 7; file >= 0; file --, wrapWest(weNoWeDir)){
        uint64_t wrappedDir = weNoWeDir;
        for (int rank = 0; rank < 7; rank ++, wrappedDir <<= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // North-West-North
    uint64_t noWeNoDir = (uint64_t) 1 << 22;
    for (int file = 7; file >= 0; file --, wrapWest(noWeNoDir)){
        uint64_t wrappedDir = noWeNoDir;
        for (int rank = 0; rank < 6; rank ++, wrappedDir <<= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // East-South-East
    uint64_t eaSoEaDir = (uint64_t) 1 << 50;
    for (int file = 0; file < 8; file ++, wrapEast(eaSoEaDir)){
        uint64_t wrappedDir = eaSoEaDir;
        for (int rank = 7; rank >= 1; rank --, wrappedDir >>= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // South-East-Sout
    uint64_t soEaSoDir = (uint64_t) 1 << 41;
    for (int file = 0; file < 8; file ++, wrapEast(soEaSoDir)){
        uint64_t wrappedDir = soEaSoDir;
        for (int rank = 7; rank >= 2; rank --, wrappedDir >>= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // West-South-West
    uint64_t weSoWeDir = (uint64_t) 1 << 55;
    for (int file = 7; file >= 0; file --, wrapWest(weSoWeDir)){
        uint64_t wrappedDir = weSoWeDir;
        for (int rank = 7; rank >= 1; rank --, wrappedDir >>= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }

    // South-West-South
    uint64_t soWeSoDir = (uint64_t) 1 << 46;
    for (int file = 7; file >= 0; file --, wrapWest(soWeSoDir)){
        uint64_t wrappedDir = soWeSoDir;
        for (int rank = 7; rank >= 2; rank --, wrappedDir >>= 8)
            m_knightAttacks[rank * 8 + file] |= wrappedDir;
    }
}


int main(){
    Board cBoard;
    uint64_t knightAttacks[64];

    initKnightAttacks(knightAttacks);

    for (int i = 0; i<63;i++){
        std::bitset<64> rep(knightAttacks[i]);
        std::cout << i << ") " << rep << std::endl;
        }

    std::cout << "loop-end" << std::endl;

    return 0;
}
