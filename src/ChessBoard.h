#pragma once

#include <cstdint>
#include <vector>

#include "cMove.h"
#include "LookupTables.h"
#include "utils.h"

class ChessBoard{
public:
    ChessBoard();
    ChessBoard(const ChessBoard &) = default;
    ChessBoard(ChessBoard &&) = default;
    ChessBoard &operator=(const ChessBoard &) = default;
    ChessBoard &operator=(ChessBoard &&) = default;

public:
    bool isIllegal();
    std::vector<CMove> getMoveList();
    std::vector<uint64_t> getBitBoards();

    void makeMove(CMove move);

private:
    void initBoard();

    void toggleSideToMove();

    uint32_t capturedPiece(int t_square);

    void generatePieceMoves(int pieceType, std::vector<CMove>& t_moveList);
    void generatePawnsMoves(std::vector<CMove>& t_moveList);
    void generatePawnsCaptures(std::vector<CMove>& t_moveList, int t_startingSquare, uint64_t t_enemyPieces);
    void generatePawnsPushes(std::vector<CMove>& t_moveList, int t_startingSquare, uint64_t t_emptySet);
    void generateDoublePushes(std::vector<CMove>& t_moveList, uint64_t t_pawnSet, uint64_t t_emptySet);
    
    bool isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide);
    bool isCheck(int t_attackingSide);

private:
    uint64_t m_bitBoard[8];

    LookupTables& m_Lookup = LookupTables::getInstance();

    uint64_t m_enPassantSquare = 0;
    uint8_t m_sideToMove = 0;
    uint8_t m_castlingRights = 0x0f;
};
