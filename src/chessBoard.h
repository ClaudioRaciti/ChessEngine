#pragma once

#include <cstdint>
#include <vector>

#include "cMove.h"

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
    void initPawnAttacks();
    void toggleSideToMove();
    void generateMoveList();
    
    void generatePawnMoves();
    void generatePiecesMoves(uint64_t t_enemyPcs, int t_pieceType);
    void serializePawnMoves(uint64_t t_pawns, int t_offset, int t_moveType);
    void serializePawnPromo(uint64_t t_pawns, int t_offset, bool t_isCapture);
    void serializeMoves(uint64_t t_moves, int t_startingSquare, int t_moveType);

    void wrapEast(uint64_t &t_bitBoard);
    void wrapWest(uint64_t &t_bitBoard);
    uint64_t cpyWrapEast(uint64_t t_bitBoard);
    uint64_t cpyWrapWest(uint64_t t_bitBoard);
    int bitScanReverse(uint64_t t_bitBoard);
    int bitScanForward(uint64_t t_bitBoard);

    uint64_t getRayAttacks(uint64_t t_occupied, int t_direction, int t_square);
    uint64_t rookAttacks(uint64_t t_occupied, int t_square);
    uint64_t bishopAttacks(uint64_t t_occupied, int t_square);
    uint64_t queenAttacks(uint64_t t_occupied, int t_square);
    uint64_t knightAttacks(int t_square);
    uint64_t kingAttacks(int t_square);
    uint64_t wPawnAttacks(int t_square);
    uint64_t bPawnAttacks(int t_square);

    uint64_t wPawnsCapturingEast(uint64_t t_wPawns, uint64_t t_bPieces);
    uint64_t wPawnsCapturingWest(uint64_t t_wPawns, uint64_t t_bPieces);
    uint64_t bPawnsCapturingEast(uint64_t t_bPawns, uint64_t t_wPieces);
    uint64_t bPawnsCapturingWest(uint64_t t_bPawns, uint64_t t_wPieces);    
    uint64_t wPushablePawns(uint64_t t_wPawns, uint64_t t_empty);
    uint64_t wDoublePushablePawns(uint64_t t_wPawns, uint64_t t_empty);
    uint64_t bPushablePawns(uint64_t t_bPawns, uint64_t t_empty);
    uint64_t bDoublePushablePawns(uint64_t t_bPawns, uint64_t t_empty);


private:
    uint64_t m_bitBoard[8];
    uint64_t m_rayAttacks[64][8]; 
    uint64_t m_knightAttacks[64];
    uint64_t m_kingAttacks[64];
    uint64_t m_pawnAttacks[64][2];
    int m_sideToMove;
    bool m_whiteToMove;
    std::vector<CMove> m_moveList;
};
