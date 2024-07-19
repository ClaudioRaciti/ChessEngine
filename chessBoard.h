#pragma once

#include <cstdint>
#include <vector>

class Board{
public:
    Board();
    Board(const Board &) = default;
    Board(Board &&) = default;
    Board &operator=(const Board &) = default;
    Board &operator=(Board &&) = default;

private:
    void initBoard();
    void initRayAttacks();
    void initKnightAttacks();
    void initKingAttacks();
    void generateMoves();

    void wrapEast(uint64_t &t_bitBoard);
    void wrapWest(uint64_t &t_bitBoard);
    uint64_t cpyWrapEast(uint64_t t_bitBoard);
    uint64_t cpyWrapWest(uint64_t t_bitBoard);
    int bitScanReverse(uint64_t t_bitBoard);
    uint64_t getRayAttacks(uint64_t t_occupied, int t_direction, int t_square);
    uint64_t rookAttacks(uint64_t t_occupied, int t_square);
    uint64_t bishopAttacks(uint64_t t_occupied, int t_square);
    uint64_t queenAttacks(uint64_t t_occupied, int t_square);
    uint64_t knightAttacks(int t_square);
    uint64_t kingAttacks(int t_square);

private:
    uint64_t m_bitBoard[15], m_rayAttacks[64][8], m_knightAttacks[64], m_kingAttacks[64];
    std::vector<uint16_t> m_moveList;
};
