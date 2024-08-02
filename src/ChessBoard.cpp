#include "ChessBoard.h"

#include <cassert>
#include <algorithm>

enum pieceType {
    white, black, pawns, knights, bishops, rooks, queens, kings
};

enum moveType {
    quiet, doublePush, kingCastle, queenCastle, capture, enPassant, 
    knightPromo = 8, bishopPromo, rookPromo, queenPromo, 
    knightPromoCapture,  bishopPromoCapture, rookPromoCapture, queenPromoCapture
};

ChessBoard::ChessBoard() : m_sideToMove(white)
{
    initBoard();
}

bool ChessBoard::isIllegal()
{
    return isCheck(m_sideToMove);
}

std::vector<CMove> ChessBoard::getMoveList()
{
    std::vector<CMove> moveList;

    generatePieceMoves(knights, moveList);
    generatePieceMoves(bishops, moveList);
    generatePieceMoves(rooks  , moveList);
    generatePieceMoves(queens , moveList);
    generatePieceMoves(kings  , moveList);
    generatePawnsMoves(moveList);    

    // TODO castling (here) and en-passant (in generatePawnMoves)
    // TODO debug double pushes
    return moveList;
}

void ChessBoard::generatePieceMoves(int t_pieceType, std::vector<CMove>& t_moveList)
{
    uint64_t pieceSet = m_bitBoard[t_pieceType] & m_bitBoard[m_sideToMove];
    uint64_t occupiedSquares = m_bitBoard[black] | m_bitBoard[white];
    uint64_t emptySquares = ~occupiedSquares;
    uint64_t enemyPieces = m_bitBoard[1 - m_sideToMove];

    if (pieceSet) do {
        int startingSquare = btw::bitScanForward(pieceSet);
        uint64_t attackSet = m_Lookup.getAttackSet(t_pieceType, occupiedSquares, startingSquare);

        //serialize all captures moves
        uint64_t captures = attackSet & enemyPieces;
        if (captures) do {
            int endSquare = btw::bitScanForward(captures);
            t_moveList.push_back(
                CMove(t_pieceType, startingSquare, endSquare, capture, capturedPiece(endSquare))
            );
        } while (captures &= (captures -1 ));

        //serialize all non-capture moves
        uint64_t quietMoves    = attackSet & emptySquares;
        if (quietMoves) do {
            int endSquare = btw::bitScanForward(quietMoves);
            t_moveList.push_back(
                CMove(t_pieceType, startingSquare, endSquare, quiet)
            );
        } while (quietMoves &= (quietMoves -1 ));
    } while (pieceSet &= (pieceSet - 1));
}

void ChessBoard::generatePawnsMoves(std::vector<CMove> &t_moveList)
{
    uint64_t pawnSet = m_bitBoard[pawns] & m_bitBoard[m_sideToMove];
    uint64_t enemyPieces = m_bitBoard[1 - m_sideToMove];
    uint64_t occupiedSquares = m_bitBoard[black] | m_bitBoard[white];
    uint64_t emptySquares = ~occupiedSquares;

    if (pawnSet) do {
        int startingSquare = btw::bitScanForward(pawnSet);
        generatePawnsCaptures(t_moveList, startingSquare, enemyPieces);
        generatePawnsPushes(t_moveList, startingSquare, emptySquares);
    } while (pawnSet &= (pawnSet - 1));

    generateDoublePushes(t_moveList, pawnSet, emptySquares);

    // TODO generate en-passant
}

void ChessBoard::generatePawnsCaptures(std::vector<CMove> &t_moveList, int t_startingSquare, uint64_t t_enemyPieces)
{
    const uint64_t promoSquares       = (uint64_t) 0xff000000000000ff;
    const uint64_t nonPromoSquares    = (uint64_t) 0x00ffffffffffff00;

    uint64_t pawnCaptures = m_Lookup.pawnAttacks(t_startingSquare,m_sideToMove) & t_enemyPieces & nonPromoSquares;
    if (pawnCaptures) do {
        int endSquare = btw::bitScanForward(pawnCaptures);
        t_moveList.push_back(CMove(pawns, t_startingSquare, endSquare, capture, capturedPiece(endSquare)));
    } while (pawnCaptures &= (pawnCaptures - 1));

    uint64_t promoCaptures = m_Lookup.pawnAttacks(t_startingSquare,m_sideToMove) & t_enemyPieces & promoSquares;
    if (promoCaptures) do {
        int endSquare = btw::bitScanForward(promoCaptures);
        t_moveList.insert(
            t_moveList.end(), 
            {   CMove(pawns, t_startingSquare, endSquare, knightPromoCapture, capturedPiece(endSquare)),
                CMove(pawns, t_startingSquare, endSquare, bishopPromoCapture, capturedPiece(endSquare)),
                CMove(pawns, t_startingSquare, endSquare, rookPromoCapture, capturedPiece(endSquare)),
                CMove(pawns, t_startingSquare, endSquare, queenPromoCapture, capturedPiece(endSquare))    }
        );
    } while (promoCaptures &= (promoCaptures - 1));
}

void ChessBoard::generatePawnsPushes(std::vector<CMove> &t_moveList, int t_startingSquare, uint64_t t_emptySquares)
{
    const uint64_t promoSquares       = (uint64_t) 0xff000000000000ff;
    const uint64_t nonPromoSquares    = (uint64_t) 0x00ffffffffffff00;

    uint64_t pawnMoves = m_Lookup.pawnPushes(t_startingSquare,m_sideToMove) & t_emptySquares & nonPromoSquares;
    if (pawnMoves) do {
        int endSquare = btw::bitScanForward(pawnMoves);
        t_moveList.push_back(CMove(pawns, t_startingSquare, endSquare, quiet));
    } while (pawnMoves &= (pawnMoves - 1));

    uint64_t pawnPromo = m_Lookup.pawnPushes(t_startingSquare,m_sideToMove) & t_emptySquares & promoSquares;
    if (pawnPromo) do {
        int endSquare = btw::bitScanForward(pawnPromo);
        t_moveList.insert(
            t_moveList.end(), 
            {   CMove(pawns, t_startingSquare, endSquare, knightPromo),
                CMove(pawns, t_startingSquare, endSquare, bishopPromo),
                CMove(pawns, t_startingSquare, endSquare, rookPromo),
                CMove(pawns, t_startingSquare, endSquare, queenPromo)    }
        );
    } while (pawnPromo &= (pawnPromo - 1));
}

void ChessBoard::generateDoublePushes(std::vector<CMove> &t_moveList, uint64_t t_pawnSet, uint64_t t_emptySquares)
{
    const uint64_t rank4 = (uint64_t) 0x00000000ff000000;
    const uint64_t rank5 = (uint64_t) 0x00000000ff000000;

    if (m_sideToMove == white) {
        uint64_t pawnDoublePush = (t_pawnSet << 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush << 8) & t_emptySquares & rank4;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare - 16;
            t_moveList.push_back(CMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
    else {
        uint64_t pawnDoublePush = (t_pawnSet >> 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush >> 8) & t_emptySquares & rank5;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare + 16;
            t_moveList.push_back(CMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
}

std::vector<uint64_t> ChessBoard::getBitBoards()
{
    return std::vector<uint64_t>(m_bitBoard, m_bitBoard + 8);
}

void ChessBoard::makeMove(CMove t_move)
{
    uint32_t fromIdx = t_move.getStartingSquare();
    uint32_t toIdx = t_move.getEndSquare();
    uint64_t fromBB = (uint64_t) 1 << fromIdx;
    uint64_t toBB = (uint64_t) 1 << toIdx;
    uint64_t fromToBB = fromBB ^ toBB;
    uint32_t piece = t_move.getPiece();
    

    uint32_t moveFlag = t_move.getFlags();
    
    if (t_move.isCapture()){
            uint32_t capturedPiece = t_move.getCaptured();
            m_bitBoard[1-m_sideToMove] ^= toBB;
            m_bitBoard[capturedPiece] ^= toBB;
    }
    
    if (t_move.isDoublePush()){
        // Update en passant
        if (m_sideToMove == white){
                m_enPassantSquare = fromBB << 8;
            }
            else {
                m_enPassantSquare = fromBB >> 8;
            }
    }   

    if (t_move.isPromo()){
        // Manage promotion
        m_bitBoard[m_sideToMove] ^= fromToBB;
        m_bitBoard[piece] ^= fromBB;

        switch (t_move.getFlags())
        {
        case knightPromo:
        case knightPromoCapture:
            m_bitBoard[knights] ^= toBB;
            break;
        case bishopPromo:
        case bishopPromoCapture:
            m_bitBoard[bishops] ^= toBB;
            break;
        case rookPromo:
        case rookPromoCapture:
            m_bitBoard[rooks] ^= toBB;
            break;
        case queenPromo:
        case queenPromoCapture:
            m_bitBoard[queens] ^= toBB;
            break;
        default:
            break;
        }
    }
    else {
        //Update bitboards
        m_bitBoard[m_sideToMove] ^= fromToBB;
        m_bitBoard[piece] ^= fromToBB;
    }
    toggleSideToMove();
}

void ChessBoard::initBoard(){
    m_bitBoard[white]   = (uint64_t) 0x000000000000ffff;
    m_bitBoard[black]   = (uint64_t) 0xffff000000000000;
    m_bitBoard[pawns]   = (uint64_t) 0x00ff00000000ff00;
    m_bitBoard[knights] = (uint64_t) 0x4200000000000042;
    m_bitBoard[bishops] = (uint64_t) 0x2400000000000024;
    m_bitBoard[rooks]   = (uint64_t) 0x8100000000000081;
    m_bitBoard[queens]  = (uint64_t) 0x0800000000000008;
    m_bitBoard[kings]   = (uint64_t) 0x1000000000000010;
}


void ChessBoard::toggleSideToMove()
{
    m_sideToMove = 1 - m_sideToMove;
}

uint32_t ChessBoard::capturedPiece(int t_square)
{
    uint32_t taken = 8;
    uint64_t mask = (uint64_t) 1 << t_square;

    if ((m_bitBoard[pawns] & mask) != 0) taken = pawns;
    if ((m_bitBoard[knights] & mask) != 0) taken = knights;
    if ((m_bitBoard[bishops] & mask) != 0) taken = bishops;
    if ((m_bitBoard[rooks] & mask) != 0) taken = rooks;
    if ((m_bitBoard[queens] & mask) != 0) taken = queens;        
    
    return taken;
}

bool ChessBoard::isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide)
{
    uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[t_attackingSide];
    if ((m_Lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;

    uint64_t knightsSet = m_bitBoard[knights] & m_bitBoard[t_attackingSide];
    if ((m_Lookup.knightAttacks(t_square) & knightsSet) != 0) return true;

    uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[t_attackingSide];
    if ((m_Lookup.kingAttacks(t_square) & kingSet) != 0) return true;

    uint64_t rooksQueensSet = (m_bitBoard[queens] | m_bitBoard[rooks]) & m_bitBoard[t_attackingSide];
    if ((m_Lookup.rookAttacks(t_occupied, t_square) & rooksQueensSet) != 0) return true;

    uint64_t bishopsQueensSet = (m_bitBoard[queens] | m_bitBoard[bishops]) & m_bitBoard[t_attackingSide];
    if ((m_Lookup.bishopAttacks(t_occupied, t_square) & bishopsQueensSet) != 0) return true;  

    return false;
}

bool ChessBoard::isCheck(int t_attackingSide)
{
    uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[1 - t_attackingSide];
    uint64_t occupied = m_bitBoard[white] | m_bitBoard[black];
    int kingSquare = btw::bitScanForward(kingSet);

    return isSquareAttacked(occupied, kingSquare, t_attackingSide);
}
