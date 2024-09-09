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
