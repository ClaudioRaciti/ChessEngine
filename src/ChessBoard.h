#pragma once

#include <cstdint>
#include <vector>

#include "ChessMove.h"
#include "LookupTables.h"
#include "PosInfo.h"

class ChessBoard{
public:
    ChessBoard();
    ChessBoard(const ChessBoard &) = default;
    ChessBoard(ChessBoard &&) = default;
    ChessBoard &operator=(const ChessBoard &) = default;
    ChessBoard &operator=(ChessBoard &&) = default;

public:
    bool isIllegal();

    std::vector<ChessMove> getMoveList();
    std::vector<uint64_t> getBitBoards();

    void makeMove(ChessMove t_move);
    void undoMove(ChessMove t_move);

    bool isCheck();

private:
    void initBoard();
    void toggleSideToMove();
    void updateBitBoards(ChessMove t_move, PosInfo& t_state);

    void generatePieceMoves(int pieceType, std::vector<ChessMove>& t_moveList);
    void generatePawnsMoves(std::vector<ChessMove>& t_moveList);
    void generatePawnsCaptures(std::vector<ChessMove>& t_moveList, uint64_t t_pawnsSet, uint64_t t_enemyPcs);
    void generatePawnsPushes(std::vector<ChessMove>& t_moveList, uint64_t t_pawnsSet, uint64_t t_emptySet);
    void generateDoublePushes(std::vector<ChessMove>& t_moveList, uint64_t t_pawnSet, uint64_t t_emptySet);
    void generateEpCaptures(std::vector<ChessMove>& t_moveList, uint64_t t_pawnsSet, int t_epSquare);
    void generateCastles(std::vector<ChessMove>& t_moveList);

    int capturedPiece(int t_square);
    uint64_t getAttackSet(int t_pieceType, uint64_t t_occupied, int t_square);

    
    bool isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide);
    bool isCheck(int t_attackingSide);

private:
    int m_sideToMove;

    uint64_t m_bitBoard[8];

    std::vector<PosInfo> m_posHistory;

    LookupTables& m_lookup = LookupTables::getInstance();
};
