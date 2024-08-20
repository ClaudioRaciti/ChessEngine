#pragma once

#include <cstdint>
#include "utils.h"

class LookupTables
{
public:
    static LookupTables& getInstance();

    LookupTables(const LookupTables&)               = delete;
    LookupTables& operator=(const LookupTables&)    = delete;

public:
    uint64_t getRayAttacks(uint64_t t_occupied, int t_direction, int t_square);
    uint64_t rookAttacks(uint64_t t_occupied, int t_square);
    uint64_t rookXRays(int t_square);
    uint64_t bishopAttacks(uint64_t t_occupied, int t_square);
    uint64_t bishopXRays(int t_square);
    uint64_t queenAttacks(uint64_t t_occupied, int t_square);
    uint64_t knightAttacks(int t_square);
    uint64_t kingAttacks(int t_square);
    uint64_t pawnPushes(int t_square, int t_sideToMove);
    uint64_t pawnAttacks(int t_square, int t_sideToMove);

    int populationCount(uint64_t x);
private:
    LookupTables();
    ~LookupTables() = default;

    void initRayAttacks();
    void initKnightAttacks();
    void initKingAttacks();
    void initPawnAttacks();
    void initPopCount256();

private:
    static LookupTables* m_instance;

    uint64_t m_rayAttacks[64][8]; 
    uint64_t m_knightAttacks[64];
    uint64_t m_kingAttacks[64];
    uint64_t m_pawnAttacks[64][2];
    uint64_t m_pawnPushes[64][2];

    int m_popCount256[256];
};
