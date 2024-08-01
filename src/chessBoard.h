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

public:
    bool isIllegal();
    std::vector<CMove> getMoveList();
    std::vector<uint64_t> getBitBoards();

    void makeMove(CMove move);

private:
    void initBoard();
    void initRayAttacks();
    void initKnightAttacks();
    void initKingAttacks();
    void initPawnAttacks();
    void toggleSideToMove();

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
    uint64_t getAttackSet(int t_pieceType, uint64_t t_occupied, int t_square);
    uint32_t capturedPiece(int t_square);

    void Board::generatePieceMoves(int pieceType, std::vector<CMove>& moveList);
    void Board::generatePawnsMoves(std::vector<CMove>& moveList);
    
    bool isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide);
    bool isCheck(int t_attackingSide);

private:
    uint64_t m_bitBoard[8];
    uint64_t m_rayAttacks[64][8]; 
    uint64_t m_knightAttacks[64];
    uint64_t m_kingAttacks[64];
    uint64_t m_pawnAttacks[64][2];
    uint64_t m_pawnPushes[64][2];

    uint64_t m_enPassantSquare = 0;
    uint8_t m_sideToMove = 0;
    uint8_t m_castlingRights = 0x0f;
};
