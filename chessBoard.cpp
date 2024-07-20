#include "chessBoard.h"

#include <cassert>

enum rayDirections {
    soWe, sout, soEa, west, east, noWe, nort, noEa
};

enum enumSquare {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum pieceType {
    wPcs, wPawns, wKnights, wBishops, wRooks, wQueens, wKing,
    bPcs, bPawns, bKnights, bBishops, bRooks, bQueens, bKing,
    empty
};

enum pieceColor {
    white, black
};

Board::Board(){
    initBoard();
    initRayAttacks();
    initKnightAttacks();
    initKingAttacks();
    initPawnAttacks();
}

void Board::initBoard(){
    m_bitBoard[wPawns] = (((uint64_t) 1 << 8) - 1) << 8;
    m_bitBoard[wKnights] = ((uint64_t) 1 << 1) | ((uint64_t) 1 << 6);
    m_bitBoard[wBishops] = ((uint64_t) 1 << 2) | ((uint64_t) 1 << 5);
    m_bitBoard[wRooks] = ((uint64_t) 1 << 0) | ((uint64_t) 1 << 7);
    m_bitBoard[wQueens] = ((uint64_t) 1 << 3);
    m_bitBoard[wKing] = ((uint64_t) 1 << 4);

    m_bitBoard[wPcs] = m_bitBoard[wPawns] | m_bitBoard[wKnights] | m_bitBoard[wBishops] | m_bitBoard[wRooks]
        | m_bitBoard[wQueens] | m_bitBoard[wKing];

    m_bitBoard[bPawns] = (((uint64_t) 1 << 8) - 1) << 48;
    m_bitBoard[bKnights] = ((uint64_t) 1 << 57) | ((uint64_t) 1 << 62);
    m_bitBoard[bBishops] = ((uint64_t) 1 << 58) | ((uint64_t) 1 << 61);
    m_bitBoard[bRooks] = ((uint64_t) 1 << 56) | ((uint64_t) 1 << 63);
    m_bitBoard[bQueens] = ((uint64_t) 1 << 59);
    m_bitBoard[bKing] = ((uint64_t) 1 << 60);

    m_bitBoard[bPcs] = m_bitBoard[bPawns] | m_bitBoard[bKnights] | m_bitBoard[bBishops] | m_bitBoard[bRooks]
        | m_bitBoard[bQueens] | m_bitBoard[bKing];

    m_bitBoard[empty] = ~ (m_bitBoard[wPcs] | m_bitBoard[bPcs]);
}

void Board::initRayAttacks(){
    uint64_t nortRay =  (uint64_t) 0x0101010101010100;
    for (int sq = 0; sq < 64; sq ++, nortRay <<= 1)
        m_rayAttacks[sq][nort] = nortRay;


    uint64_t noEaRay = (uint64_t) 0x8040201008040200;
    for (int file = 0; file < 8; file ++, wrapEast(noEaRay)){
        uint64_t wrappedRay = noEaRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noEa] = wrappedRay;
    }

    uint64_t noWeRay = (uint64_t) 0x102040810204000;
    for (int file = 7; file >= 0; file --, wrapWest(noWeRay)){
        uint64_t wrappedRay = noWeRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noWe] = wrappedRay;
    }

    uint64_t eastRay = (uint64_t) 0x0000000000000fe;
    for (int file = 0; file < 8; file ++, wrapEast(eastRay)){
        uint64_t wrappedRay = eastRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][east] = wrappedRay;
    }

    uint64_t soutRay = (uint64_t) 0x0080808080808080;
    for (int sq = 63; sq >= 0; sq --, soutRay >>= 1)
        m_rayAttacks[sq][sout] = soutRay;

    uint64_t soEaRay = (uint64_t) 0x0002040810204080;
    for (int file = 0; file < 8; file ++, wrapEast(soEaRay)){
        uint64_t wrappedRay = soEaRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soEa] = wrappedRay;
    }

    uint64_t soWeRay = (uint64_t) 0x0040201008040201;
    for (int file = 7; file >= 0; file --, wrapWest(soWeRay)){
        uint64_t wrappedRay = soWeRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soWe] = wrappedRay;
    }

    uint64_t westRay = (uint64_t) 0x7f00000000000000;
    for (int file = 7; file >= 0; file --, wrapWest(westRay)){
        uint64_t wrappedRay = westRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][west] = wrappedRay;
    }
}

void Board::initKnightAttacks(){
    uint64_t knightPos = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, knightPos <<= 1){
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

void Board::initKingAttacks(){
    uint64_t kingPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, kingPosition <<= 1){
        m_kingAttacks[sq] = kingPosition << 8 | kingPosition >> 8;
        uint64_t eastDir = cpyWrapEast(kingPosition);
        m_kingAttacks[sq] |= eastDir | eastDir << 8 | eastDir >> 8;
        uint64_t westDir = cpyWrapWest(kingPosition);
        m_kingAttacks[sq] |= westDir | westDir << 8 | westDir >> 8;
    }
}

void Board::initPawnAttacks(){
    uint64_t pawnPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++ , pawnPosition <<= 1){
        if (sq < 8 || sq > 55){
            m_pawnAttacks[sq][white] = (uint64_t) 0;
            m_pawnAttacks[sq][black] = (uint64_t) 0;
        }
        else {
            uint64_t nortDir = pawnPosition << 8;
            uint64_t soutDir = pawnPosition >> 8;

            m_pawnAttacks[sq][white] = cpyWrapEast(nortDir) | cpyWrapWest(nortDir);
            m_pawnAttacks[sq][black] = cpyWrapEast(soutDir) | cpyWrapWest(soutDir);
        }
    }    
}

void Board::wrapEast(uint64_t &t_bitBoard){
    uint64_t mask = (uint64_t) 0x7f7f7f7f7f7f7f7f;
    t_bitBoard &= mask;
    t_bitBoard <<= 1;
}

uint64_t Board::cpyWrapEast(uint64_t t_bitBoard){
    wrapEast(t_bitBoard);
    return t_bitBoard;
}

void Board::wrapWest(uint64_t &t_bitBoard){
    uint64_t mask = (uint64_t) 0xfefefefefefefefe;
    t_bitBoard &= mask;
    t_bitBoard >>= 1;
}

uint64_t Board::cpyWrapWest(uint64_t t_bitBoard){
    wrapWest(t_bitBoard);
    return t_bitBoard;
}

// Finds position of Least Significant 1 Bit
// int Board::bitScanForward(uint64_t t_bitBoard){
//     const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
//     assert (t_bitBoard != 0);
//     return index64[((t_bitBoard ^ (t_bitBoard-1)) * deBrujin64) >> 58];
// }

// Finds position of Most Significant 1 Bit
int Board::bitScanReverse(uint64_t t_bitBoard){
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
    assert (t_bitBoard != 0);
    t_bitBoard |= t_bitBoard >> 1;
    t_bitBoard |= t_bitBoard >> 2;
    t_bitBoard |= t_bitBoard >> 4;
    t_bitBoard |= t_bitBoard >> 8;
    t_bitBoard |= t_bitBoard >> 16;
    t_bitBoard |= t_bitBoard >> 32;
    return index64[(t_bitBoard * deBrujin64) >> 58];
}

uint64_t Board::getRayAttacks(uint64_t t_occupied, int t_direction, int t_square){
    const uint64_t mask[8] = {
        0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,
        0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000
    };
    const uint64_t bitDir[8] = {
        0x0000000000000001,0x0000000000000001,0x0000000000000001,0x0000000000000001,
        0x8000000000000000,0x8000000000000000,0x8000000000000000,0x8000000000000000
    };
    uint64_t attacks    = m_rayAttacks[t_square][t_direction];
    uint64_t blocker    = t_occupied & attacks;
    blocker &= (-blocker) | mask[t_direction];
    return attacks ^ m_rayAttacks[bitScanReverse(blocker | bitDir[t_direction])][t_direction];
}

uint64_t Board::rookAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, nort, t_square) | getRayAttacks(t_occupied, sout, t_square)
        | getRayAttacks(t_occupied, east, t_square) | getRayAttacks(t_occupied, west, t_square);
}

uint64_t Board::bishopAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, noWe, t_square) | getRayAttacks(t_occupied, noEa, t_square)
        | getRayAttacks(t_occupied, soWe, t_square) | getRayAttacks(t_occupied, soEa, t_square);
}

uint64_t Board::queenAttacks(uint64_t t_occupied, int t_square){
    return bishopAttacks(t_occupied, t_square) | rookAttacks(t_occupied, t_square);
}

uint64_t Board::knightAttacks(int t_square)
{
    return m_knightAttacks[t_square];
}

uint64_t Board::kingAttacks(int t_square)
{
    return m_kingAttacks[t_square];
}

uint64_t Board::wPawnAttacks(int t_square)
{
    return m_pawnAttacks[t_square][white];
}

uint64_t Board::bPawnAttacks(int t_square)
{
    return m_pawnAttacks[t_square][black];
}

uint64_t Board::wPawnsCapturingEast(uint64_t t_wPawns, uint64_t t_bPieces)
{
    return (cpyWrapWest(t_bPieces) >> 8) & t_wPawns;
}

uint64_t Board::wPawnsCapturingWest(uint64_t t_wPawns, uint64_t t_bPieces)
{
    return (cpyWrapEast(t_bPieces) >> 8) & t_wPawns;
}

uint64_t Board::bPawnsCapturingEast(uint64_t t_bPawns, uint64_t t_wPieces)
{
    return (cpyWrapWest(t_wPieces) << 8) & t_bPawns;
}

uint64_t Board::bPawnsCapturingWest(uint64_t t_bPawns, uint64_t t_wPieces)
{
    return (cpyWrapEast(t_wPieces) << 8) & t_bPawns;
}

uint64_t Board::wPushablePawns(uint64_t t_wPawns, uint64_t t_empty)
{
    return (t_empty >> 8) & t_wPawns;
}

uint64_t Board::wDoublePushablePawns(uint64_t t_wPawns, uint64_t t_empty)
{
    const uint64_t rank4 = (uint64_t) 0x00000000FF000000;
    uint64_t emptyRow3 = ((t_empty & rank4) >> 8) & t_empty;
    return wPushablePawns(t_wPawns, emptyRow3);
}

uint64_t Board::bPushablePawns(uint64_t t_bPawns, uint64_t t_empty)
{
    return (t_empty << 8) & t_bPawns;
}

uint64_t Board::bDoublePushablePawns(uint64_t t_bPawns, uint64_t t_empty)
{
    const uint64_t rank5 = (uint64_t) 0x000000FF00000000;
    uint64_t emptyRow6 = ((t_empty & rank5) << 8) & t_empty;
    return bPushablePawns(t_bPawns, emptyRow6);
}

void Board::generateMoves(){
    return;
}
