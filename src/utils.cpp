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

void btw::initPopCount256()
{
    _popCount256[0] = 0;
    for(int i = 1; i < 256; i ++)
        _popCount256[i] = _popCount256[i / 2] + (i & 1);
    _isPopCountInit = true;
}

int btw::popCount(uint64_t bitBoard)
{
    if (!_isPopCountInit) initPopCount256();
    uint8_t *p = (uint8_t *) &bitBoard;
    return  _popCount256[p[0]] + 
            _popCount256[p[1]] +
            _popCount256[p[2]] +
            _popCount256[p[3]] +
            _popCount256[p[4]] +
            _popCount256[p[5]] +
            _popCount256[p[6]] +
            _popCount256[p[7]];
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

enum pieceType {
    white, black, pawns, knights, bishops, rooks, queens, kings
};

float eval::evaluate(const std::vector<uint64_t> &bitBoards)
{
    int res = 0;
    res += material(bitBoards);
    return float(res) / 100;
}

// int eval::pieceVal(int sideToMove, int piece, int square)
// {
//     int res = 0;

//     const int knightSquareTable[2][64] ={
//         {
//             -105, -21, -58, -33, -17, -28, -19,  -23,
//              -29, -53, -12,  -3,  -1,  18, -14,  -19,
//              -23,  -9,  12,  10,  19,  17,  25,  -16,
//              -13,   4,  16,  13,  28,  19,  21,   -8,
//               -9,  17,  19,  53,  37,  69,  18,   22,
//              -47,  60,  37,  65,  84, 129,  73,   44,
//              -73, -41,  72,  36,  23,  62,   7,  -17,
//             -167, -89, -34, -49,  61, -97, -15, -107
//         },
//         {
//             -167, -89, -34, -49,  61, -97, -15, -107,
//              -73, -41,  72,  36,  23,  62,   7,  -17,
//              -47,  60,  37,  65,  84, 129,  73,   44,
//               -9,  17,  19,  53,  37,  69,  18,   22,
//              -13,   4,  16,  13,  28,  19,  21,   -8,
//              -23,  -9,  12,  10,  19,  17,  25,  -16,
//              -29, -53, -12,  -3,  -1,  18, -14,  -19,
//             -105, -21, -58, -33, -17, -28, -19,  -23
//         }
//     };

//     const int bishopSquareTable[2][64] = {
//         {
//             -33,  -3, -14, -21, -13, -12, -39, -21,
//               4,  15,  16,   0,   7,  21,  33,   1,
//               0,  15,  15,  15,  14,  27,  18,  10,
//              -6,  13,  13,  26,  34,  12,  10,   4,
//              -4,   5,  19,  50,  37,  37,   7,  -2,
//             -16,  37,  43,  40,  35,  50,  37,  -2,
//             -26,  16, -18, -13,  30,  59,  18, -47,
//             -29,   4, -82, -37, -25, -42,   7,  -8
//         },
//         {
//             -29,   4, -82, -37, -25, -42,   7,  -8,
//             -26,  16, -18, -13,  30,  59,  18, -47,
//             -16,  37,  43,  40,  35,  50,  37,  -2,
//              -4,   5,  19,  50,  37,  37,   7,  -2,
//              -6,  13,  13,  26,  34,  12,  10,   4,
//               0,  15,  15,  15,  14,  27,  18,  10,
//               4,  15,  16,   0,   7,  21,  33,   1,
//             -33,  -3, -14, -21, -13, -12, -39, -21
//         }
//     };

//     const int rookSquareTable[2][64] = {
//         {
//             -19, -13,   1,  17, 16,  7, -37, -26,
//             -44, -16, -20,  -9, -1, 11,  -6, -71,
//             -45, -25, -16, -17,  3,  0,  -5, -33,
//             -36, -26, -12,  -1,  9, -7,   6, -23,
//             -24, -11,   7,  26, 24, 35,  -8, -20,
//              -5,  19,  26,  36, 17, 45,  61,  16,
//              27,  32,  58,  62, 80, 67,  26,  44,
//              32,  42,  32,  51, 63,  9,  31,  43
//         },
//         {
//              32,  42,  32,  51, 63,  9,  31,  43,
//              27,  32,  58,  62, 80, 67,  26,  44,
//              -5,  19,  26,  36, 17, 45,  61,  16,
//             -24, -11,   7,  26, 24, 35,  -8, -20,
//             -36, -26, -12,  -1,  9, -7,   6, -23,
//             -45, -25, -16, -17,  3,  0,  -5, -33,
//             -44, -16, -20,  -9, -1, 11,  -6, -71,
//             -19, -13,   1,  17, 16,  7, -37, -26
//         }
//     };

//     const int queenSquareTable[2][64] = {
//         {
//              -1, -18,  -9,  10, -15, -25, -31, -50,
//             -35,  -8,  11,   2,   8,  15,  -3,   1,
//             -14,   2, -11,  -2,  -5,   2,  14,   5,
//              -9, -26,  -9, -10,  -2,  -4,   3,  -3,
//             -27, -27, -16, -16,  -1,  17,  -2,   1,
//             -13, -17,   7,   8,  29,  56,  47,  57,
//             -24, -39,  -5,   1, -16,  57,  28,  54,
//             -28,   0,  29,  12,  59,  44,  43,  45
//         },
//         {
//             -28,   0,  29,  12,  59,  44,  43,  45,
//             -24, -39,  -5,   1, -16,  57,  28,  54,
//             -13, -17,   7,   8,  29,  56,  47,  57,
//             -27, -27, -16, -16,  -1,  17,  -2,   1,
//              -9, -26,  -9, -10,  -2,  -4,   3,  -3,
//             -14,   2, -11,  -2,  -5,   2,  14,   5,
//             -35,  -8,  11,   2,   8,  15,  -3,   1,
//              -1, -18,  -9,  10, -15, -25, -31, -50
//         }
//     };

//     const int kingSquareTable[2][64] = {
//         {
//             -15,  36,  12, -54,   8, -28,  24,  14,
//               1,   7,  -8, -64, -43, -16,   9,   8,
//             -14, -14, -22, -46, -44, -30, -15, -27,
//             -49,  -1, -27, -39, -46, -44, -33, -51,
//             -17, -20, -12, -27, -30, -25, -14, -36,
//              -9,  24,   2, -16, -20,   6,  22, -22,
//              29,  -1, -20,  -7,  -8,  -4, -38, -29,
//             -65,  23,  16, -15, -56, -34,   2,  13
//         },
//         {
//             -65,  23,  16, -15, -56, -34,   2,  13,
//              29,  -1, -20,  -7,  -8,  -4, -38, -29,
//              -9,  24,   2, -16, -20,   6,  22, -22,
//             -17, -20, -12, -27, -30, -25, -14, -36,
//             -49,  -1, -27, -39, -46, -44, -33, -51,
//             -14, -14, -22, -46, -44, -30, -15, -27,
//               1,   7,  -8, -64, -43, -16,   9,   8,
//             -15,  36,  12, -54,   8, -28,  24,  14
//         }
//     };

//     const int pawnSquareTable[2][64]={
//         {
//               0,   0,   0,   0,   0,   0,  0,   0,
//             -35,  -1, -20, -23, -15,  24, 38, -22,
//             -26,  -4,  -4, -10,   3,   3, 33, -12,
//             -27,  -2,  -5,  12,  17,   6, 10, -25,
//             -14,  13,   6,  21,  23,  12, 17, -23,
//              -6,   7,  26,  31,  65,  56, 25, -20,
//              98, 134,  61,  95,  68, 126, 34, -11,
//               0,   0,   0,   0,   0,   0,  0,   0
//         },
//         {
//               0,   0,   0,   0,   0,   0,  0,   0,
//              98, 134,  61,  95,  68, 126, 34, -11,
//              -6,   7,  26,  31,  65,  56, 25, -20,
//             -14,  13,   6,  21,  23,  12, 17, -23,
//             -27,  -2,  -5,  12,  17,   6, 10, -25,
//             -26,  -4,  -4, -10,   3,   3, 33, -12,
//             -35,  -1, -20, -23, -15,  24, 38, -22,
//               0,   0,   0,   0,   0,   0,  0,   0
//         }
//     };
    
//     switch (piece)
//     {
//     case pawns:     res +=   82 + pawnSquareTable[sideToMove][square];  break;
//     case knights:   res +=  337 + knightSquareTable[sideToMove][square];break;
//     case bishops:   res +=  365 + bishopSquareTable[sideToMove][square];break;
//     case rooks:     res +=  477 + rookSquareTable[sideToMove][square];  break;
//     case queens:    res += 1025 + queenSquareTable[sideToMove][square]; break;
//     case kings:     res += kingSquareTable[sideToMove][square];         break;
//     }

//     return res;
// }

int eval::material(const std::vector<uint64_t> &position)
{
    int res = 0;

    uint64_t wPawns     = position[white] & position[pawns];
    uint64_t wKnights   = position[white] & position[knights];
    uint64_t wBishops   = position[white] & position[bishops];
    uint64_t wRooks     = position[white] & position[rooks];
    uint64_t wQueens    = position[white] & position[queens];
    uint64_t wKing      = position[white] & position[kings];

    uint64_t bPawns     = position[black] & position[pawns];
    uint64_t bKnights   = position[black] & position[knights];
    uint64_t bBishops   = position[black] & position[bishops];
    uint64_t bRooks     = position[black] & position[rooks];
    uint64_t bQueens    = position[black] & position[queens];
    uint64_t bKing      = position[black] & position[kings];

    if(wPawns) do {
        res += pieceVal(white, pawns, btw::bitScanForward(wPawns));
    } while (wPawns &= (wPawns - 1)); 
    if(wKnights) do {
        res += pieceVal(white, knights, btw::bitScanForward(wKnights));
    } while (wKnights &= (wKnights - 1));
    if(wBishops) do {
        res += pieceVal(white, bishops, btw::bitScanForward(wBishops));
    } while (wBishops &= (wBishops - 1));
    if(wRooks) do {
        res += pieceVal(white, rooks, btw::bitScanForward(wRooks));
    } while (wRooks &= (wRooks - 1));
    if(wQueens) do {
        res += pieceVal(white, queens, btw::bitScanForward(wQueens));
    } while (wQueens &= (wQueens - 1));

    if(bPawns) do {
        res -= pieceVal(black, pawns, btw::bitScanForward(bPawns));
    } while (bPawns &= (bPawns - 1)); 
    if(bKnights) do {
        res -= pieceVal(black, knights, btw::bitScanForward(bKnights));
    } while (bKnights &= (bKnights - 1));
    if(bBishops) do {
        res -= pieceVal(black, bishops, btw::bitScanForward(bBishops));
    } while (bBishops &= (bBishops - 1));
    if(bRooks) do {
        res -= pieceVal(black, rooks, btw::bitScanForward(bRooks));
    } while (bRooks &= (bRooks - 1));
    if(bQueens) do {
        res -= pieceVal(black, queens, btw::bitScanForward(bQueens));
    } while (wQueens &= (bQueens - 1));

    res += pieceVal(white, kings, btw::bitScanForward(wKing));
    res -= pieceVal(black, kings, btw::bitScanForward(bKing));

    return res;
}

int eval::pawnStructure(const std::vector<uint64_t> &bitBoards)
{
    int res = 0;
    uint64_t col = 0x0101010101010101;
    uint64_t wPawns = bitBoards[white] & bitBoards[pawns];
    uint64_t bPawns = bitBoards[black] & bitBoards[pawns];

    for(int i = 0; i < 8; i ++, col <<= 1){    
        if(wPawns & col) res -= 40 * (btw::popCount(wPawns & col) - 1);
        if(bPawns & col) res += 40 * (btw::popCount(bPawns & col) - 1);
    }

    return res;
}
