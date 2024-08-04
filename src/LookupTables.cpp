#include "LookupTables.h"

enum rayDirections {
    soWe, sout, soEa, west, east, noWe, nort, noEa
};

enum pieceType {
    white, black, pawns, knights, bishops, rooks, queens, kings
};

LookupTables::LookupTables()
{
    initRayAttacks();
    initKnightAttacks();
    initKingAttacks();
    initPawnAttacks();
}

LookupTables* LookupTables::m_instance = nullptr;

LookupTables &LookupTables::getInstance()
{
    if (m_instance == nullptr){
        m_instance = new LookupTables();
    }
    return *m_instance;
}

void LookupTables::initRayAttacks()
{
    uint64_t nortRay =  (uint64_t) 0x0101010101010100;
    for (int sq = 0; sq < 64; sq ++, nortRay <<= 1)
        m_rayAttacks[sq][nort] = nortRay;


    uint64_t noEaRay = (uint64_t) 0x8040201008040200;
    for (int file = 0; file < 8; file ++, btw::wrapEast(noEaRay)){
        uint64_t wrappedRay = noEaRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noEa] = wrappedRay;
    }

    uint64_t noWeRay = (uint64_t) 0x102040810204000;
    for (int file = 7; file >= 0; file --, btw::wrapWest(noWeRay)){
        uint64_t wrappedRay = noWeRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noWe] = wrappedRay;
    }

    uint64_t eastRay = (uint64_t) 0x0000000000000fe;
    for (int file = 0; file < 8; file ++, btw::wrapEast(eastRay)){
        uint64_t wrappedRay = eastRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][east] = wrappedRay;
    }

    uint64_t soutRay = (uint64_t) 0x0080808080808080;
    for (int sq = 63; sq >= 0; sq --, soutRay >>= 1)
        m_rayAttacks[sq][sout] = soutRay;

    uint64_t soEaRay = (uint64_t) 0x0002040810204080;
    for (int file = 0; file < 8; file ++, btw::wrapEast(soEaRay)){
        uint64_t wrappedRay = soEaRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soEa] = wrappedRay;
    }

    uint64_t soWeRay = (uint64_t) 0x0040201008040201;
    for (int file = 7; file >= 0; file --, btw::wrapWest(soWeRay)){
        uint64_t wrappedRay = soWeRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soWe] = wrappedRay;
    }

    uint64_t westRay = (uint64_t) 0x7f00000000000000;
    for (int file = 7; file >= 0; file --, btw::wrapWest(westRay)){
        uint64_t wrappedRay = westRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][west] = wrappedRay;
    }
}

void LookupTables::initKnightAttacks()
{
    uint64_t knightPos = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, knightPos <<= 1){
        m_knightAttacks[sq] = (uint64_t) 0;

        uint64_t eastDir = btw::cpyWrapEast(knightPos);
        uint64_t eaEaDir = btw::cpyWrapEast(eastDir);
        m_knightAttacks[sq] |= eastDir << 16 | eaEaDir << 8 
            | eastDir >> 16 | eaEaDir >> 8;

        uint64_t westDir = btw::cpyWrapWest(knightPos);
        uint64_t weWeDir = btw::cpyWrapWest(westDir);
        m_knightAttacks[sq] |= westDir << 16 | weWeDir << 8
            | westDir >> 16 | weWeDir >> 8;
    }
}

void LookupTables::initKingAttacks()
{
    uint64_t kingPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, kingPosition <<= 1){
        m_kingAttacks[sq] = kingPosition << 8 | kingPosition >> 8;
        uint64_t eastDir = btw::cpyWrapEast(kingPosition);
        m_kingAttacks[sq] |= eastDir | eastDir << 8 | eastDir >> 8;
        uint64_t westDir = btw::cpyWrapWest(kingPosition);
        m_kingAttacks[sq] |= westDir | westDir << 8 | westDir >> 8;
    }
}

void LookupTables::initPawnAttacks()
{
    uint64_t pawnPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++ , pawnPosition <<= 1){
        uint64_t nortDir = pawnPosition << 8;
        uint64_t soutDir = pawnPosition >> 8;

        m_pawnAttacks[sq][white] = btw::cpyWrapEast(nortDir) | btw::cpyWrapWest(nortDir);
        m_pawnAttacks[sq][black] = btw::cpyWrapEast(soutDir) | btw::cpyWrapWest(soutDir);
        m_pawnPushes [sq][white] = nortDir;
        m_pawnPushes [sq][black] = soutDir;
    }    
}

uint64_t LookupTables::getRayAttacks(uint64_t t_occupied, int t_direction, int t_square)
{
    const uint64_t mask[8] = {
        0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,
        0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000
    };
    const uint64_t bitDir[8] = {
        0x0000000000000001,0x0000000000000001,0x0000000000000001,0x0000000000000001,
        0x8000000000000000,0x8000000000000000,0x8000000000000000,0x8000000000000000
    };
    uint64_t attacks    = m_rayAttacks[t_square][t_direction];
    uint64_t blocker    = (t_occupied & attacks) | bitDir[t_direction];
    blocker &= (-blocker) | mask[t_direction];
    int blockerSq = btw::bitScanReverse(blocker);
    return (attacks ^ m_rayAttacks[blockerSq][t_direction]);
}

uint64_t LookupTables::rookAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, nort, t_square) | getRayAttacks(t_occupied, sout, t_square)
        | getRayAttacks(t_occupied, east, t_square) | getRayAttacks(t_occupied, west, t_square);
}

uint64_t LookupTables::bishopAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, noWe, t_square) | getRayAttacks(t_occupied, noEa, t_square)
        | getRayAttacks(t_occupied, soWe, t_square) | getRayAttacks(t_occupied, soEa, t_square);
}

uint64_t LookupTables::queenAttacks(uint64_t t_occupied, int t_square){
    return bishopAttacks(t_occupied, t_square) | rookAttacks(t_occupied, t_square);
}

uint64_t LookupTables::knightAttacks(int t_square)
{
    return m_knightAttacks[t_square];
}

uint64_t LookupTables::kingAttacks(int t_square)
{
    return m_kingAttacks[t_square];
}

uint64_t LookupTables::pawnPushes(int t_square, int t_sideToMove)
{
    return m_pawnPushes[t_square][t_sideToMove];
}

uint64_t LookupTables::pawnAttacks(int t_square, int t_sideToMove)
{
    return m_pawnAttacks[t_square][t_sideToMove];
}

