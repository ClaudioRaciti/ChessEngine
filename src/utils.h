#pragma once

#include <cstdint>
#include <vector>
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

    static int _popCount256[256];
    static bool _isPopCountInit = false;
    void initPopCount256();
    int popCount(uint64_t bitBoard);
}; // namespace btw

// Evaluation functions
namespace eval
{

    static int knightSquareTable[64] ={
        -105, -21, -58, -33, -17, -28, -19,  -23,
         -29, -53, -12,  -3,  -1,  18, -14,  -19,
         -23,  -9,  12,  10,  19,  17,  25,  -16,
         -13,   4,  16,  13,  28,  19,  21,   -8,
          -9,  17,  19,  53,  37,  69,  18,   22,
         -47,  60,  37,  65,  84, 129,  73,   44,
         -73, -41,  72,  36,  23,  62,   7,  -17,
        -167, -89, -34, -49,  61, -97, -15, -107
    };

    static int bishopSquareTable[64] = {
        -33,  -3, -14, -21, -13, -12, -39, -21,
          4,  15,  16,   0,   7,  21,  33,   1,
          0,  15,  15,  15,  14,  27,  18,  10,
         -6,  13,  13,  26,  34,  12,  10,   4,
         -4,   5,  19,  50,  37,  37,   7,  -2,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -29,   4, -82, -37, -25, -42,   7,  -8
    };

    static int rookSquareTable[64] = {
        -19, -13,   1,  17, 16,  7, -37, -26,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -24, -11,   7,  26, 24, 35,  -8, -20,
         -5,  19,  26,  36, 17, 45,  61,  16,
         27,  32,  58,  62, 80, 67,  26,  44,
         32,  42,  32,  51, 63,  9,  31,  43
    };

    static int queenSquareTable[64] = {
             -1, -18,  -9,  10, -15, -25, -31, -50,
            -35,  -8,  11,   2,   8,  15,  -3,   1,
            -14,   2, -11,  -2,  -5,   2,  14,   5,
             -9, -26,  -9, -10,  -2,  -4,   3,  -3,
            -27, -27, -16, -16,  -1,  17,  -2,   1,
            -13, -17,   7,   8,  29,  56,  47,  57,
            -24, -39,  -5,   1, -16,  57,  28,  54,
            -28,   0,  29,  12,  59,  44,  43,  45
    };

    static int kingSquareTable[64] = {
        -15,  36,  12, -54,   8, -28,  24,  14,
          1,   7,  -8, -64, -43, -16,   9,   8,
        -14, -14, -22, -46, -44, -30, -15, -27,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -17, -20, -12, -27, -30, -25, -14, -36,
         -9,  24,   2, -16, -20,   6,  22, -22,
         29,  -1, -20,  -7,  -8,  -4, -38, -29,
        -65,  23,  16, -15, -56, -34,   2,  13
    };

    static int pawnSquareTable[64]={
          0,   0,   0,   0,   0,   0,  0,   0,
        -35,  -1, -20, -23, -15,  24, 38, -22,
        -26,  -4,  -4, -10,   3,   3, 33, -12,
        -27,  -2,  -5,  12,  17,   6, 10, -25,
        -14,  13,   6,  21,  23,  12, 17, -23,
         -6,   7,  26,  31,  65,  56, 25, -20,
         98, 134,  61,  95,  68, 126, 34, -11,
          0,   0,   0,   0,   0,   0,  0,   0
    };

    static int *pieceSquareTable[6] = {
        pawnSquareTable,
        knightSquareTable,
        bishopSquareTable,
        rookSquareTable,
        queenSquareTable,
        kingSquareTable
    };

    static int stdVal[6] = {
          82,
         337,
         365,
         477,
        1025,
           0
    };

    inline int mirror(const int &square){return 56 - (8*(square/8)) + square%8;};
    inline int pieceVal(const int &sideToMove, const int &piece, const int &square){
        return stdVal[piece - 2] + pieceSquareTable[piece - 2][sideToMove == 0 ? square : mirror(square)];
    };


    float evaluate(const std::vector<uint64_t> &bitBoards);
    //int pieceVal(int sideToMove, int piece, int square);
    int material(const std::vector<uint64_t> &bitBoards);
    int pawnStructure(const std::vector<uint64_t> &bitBoards);
}; // namespace eval
