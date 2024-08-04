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
    m_posHistory.push_back(PosInfo());
}

// bool ChessBoard::isIllegal()
// {
//     return isCheck(m_sideToMove);
// }

std::vector<ChessMove> ChessBoard::getMoveList()
{
    std::vector<ChessMove> moveList;

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

void ChessBoard::generatePieceMoves(int t_pieceType, std::vector<ChessMove>& t_moveList)
{
    uint64_t pieceSet = m_bitBoard[t_pieceType] & m_bitBoard[m_sideToMove];
    uint64_t occupiedSquares = m_bitBoard[black] | m_bitBoard[white];
    uint64_t emptySquares = ~occupiedSquares;
    uint64_t enemyPieces = m_bitBoard[1 - m_sideToMove];

    if (pieceSet) do {
        int startingSquare = btw::bitScanForward(pieceSet);
        uint64_t attackSet = getAttackSet(t_pieceType, occupiedSquares, startingSquare);

        //serialize all captures moves
        uint64_t captures = attackSet & enemyPieces;
        if (captures) do {
            int endSquare = btw::bitScanForward(captures);
            t_moveList.push_back(
                ChessMove(t_pieceType, startingSquare, endSquare, capture, capturedPiece(endSquare))
            );
        } while (captures &= (captures -1 ));

        //serialize all non-capture moves
        uint64_t quietMoves    = attackSet & emptySquares;
        if (quietMoves) do {
            int endSquare = btw::bitScanForward(quietMoves);
            t_moveList.push_back(
                ChessMove(t_pieceType, startingSquare, endSquare, quiet)
            );
        } while (quietMoves &= (quietMoves -1 ));
    } while (pieceSet &= (pieceSet - 1));
}

void ChessBoard::generatePawnsMoves(std::vector<ChessMove> &t_moveList)
{
    uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[m_sideToMove];
    uint64_t enemyPcs = m_bitBoard[1 - m_sideToMove];
    uint64_t emptySet = ~ (m_bitBoard[black] | m_bitBoard[white]);

    generatePawnsPushes(t_moveList, pawnsSet, emptySet);
    generatePawnsCaptures(t_moveList, pawnsSet, enemyPcs);
    generateDoublePushes(t_moveList, pawnsSet, emptySet);
    // other en passant logic
}

void ChessBoard::generatePawnsPushes(std::vector<ChessMove> &t_moveList, uint64_t t_pawnsSet, uint64_t t_emptySet)
{
    const uint64_t row8 = (uint64_t) 0xff00000000000000;
    const uint64_t row1 = (uint64_t) 0x00000000000000ff;
    uint64_t pushSet, promoSet;
    int offset;

    if(m_sideToMove == white) {
        pushSet = (t_pawnsSet << 8 & t_emptySet) & ~row8;
        promoSet = (t_pawnsSet << 8 & t_emptySet) & row8; 
        offset = -8;
    }
    else {
        pushSet = (t_pawnsSet >> 8 & t_emptySet) & ~row1;
        promoSet = (t_pawnsSet >> 8 & t_emptySet) & row1; 
        offset = 8;
    }

    if (pushSet) do {
        int endSq = btw::bitScanForward(pushSet);
        int startSq = endSq + offset;
        t_moveList.push_back(ChessMove(pawns, startSq, endSq, quiet));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = btw::bitScanForward(pushSet);
        int startSq = endSq + offset;
        t_moveList.insert(
            t_moveList.end(),
            {
                ChessMove(pawns, startSq, endSq, knightPromo),
                ChessMove(pawns, startSq, endSq, bishopPromo),
                ChessMove(pawns, startSq, endSq, rookPromo),
                ChessMove(pawns, startSq, endSq, queenPromo) 
            }
        );
    } while (promoSet &= (promoSet - 1));
}

void ChessBoard::generatePawnsCaptures(std::vector<ChessMove> &t_moveList, uint64_t t_pawnsSet, uint64_t t_enemyPcs)
{
    const uint64_t row8 = (uint64_t) 0xff00000000000000;
    const uint64_t row1 = (uint64_t) 0x00000000000000ff;
    uint64_t eastCaptures, westCaptures, eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;
    
    if(m_sideToMove == white) {
        eastCaptures = (btw::cpyWrapEast(t_pawnsSet) << 8 & t_enemyPcs) & ~row8;
        westCaptures = (btw::cpyWrapWest(t_pawnsSet) << 8 & t_enemyPcs) & ~row8;
        eastPromoCaptures = (btw::cpyWrapEast(t_pawnsSet) << 8 & t_enemyPcs) & row8;
        westPromoCaptures = (btw::cpyWrapWest(t_pawnsSet) << 8 & t_enemyPcs) & row8;
        eastOffset = -7;
        westOffset = -9;
    }
    else {
        eastCaptures = (btw::cpyWrapEast(t_pawnsSet) >> 8 & t_enemyPcs) & ~row1;
        westCaptures = (btw::cpyWrapWest(t_pawnsSet) >> 8 & t_enemyPcs) & ~row1;
        eastPromoCaptures = (btw::cpyWrapEast(t_pawnsSet) >> 8 & t_enemyPcs) & row1;
        westPromoCaptures = (btw::cpyWrapWest(t_pawnsSet) >> 8 & t_enemyPcs) & row1;
        eastOffset = 9;
        westOffset = 7;
    }

    if (westCaptures) do {
        int endSq = btw::bitScanForward(westCaptures);
        int startSq = endSq + westOffset;
        int captured = capturedPiece(endSq);
        t_moveList.push_back(ChessMove(pawns, startSq, endSq, capture, captured));
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = btw::bitScanForward(eastCaptures);
        int startSq = endSq + eastOffset;
        int captured = capturedPiece(endSq);
        t_moveList.push_back(ChessMove(pawns, startSq, endSq, capture, captured));
    } while (eastCaptures &= (eastCaptures - 1));


    if (eastPromoCaptures) do {
        int endSq = btw::bitScanForward(eastPromoCaptures);
        int startSq = endSq + eastOffset;
        int captured = capturedPiece(endSq);
        t_moveList.insert(
            t_moveList.end(),
            {
                ChessMove(pawns, startSq, endSq, knightPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, bishopPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, rookPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, queenPromoCapture, captured) 
            }
        );
    } while (eastPromoCaptures &= (eastPromoCaptures - 1));

    if (westPromoCaptures) do {
        int endSq = btw::bitScanForward(westPromoCaptures);
        int startSq = endSq + westOffset;
        int captured = capturedPiece(endSq);
        t_moveList.insert(
            t_moveList.end(),
            {
                ChessMove(pawns, startSq, endSq, knightPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, bishopPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, rookPromoCapture, captured),
                ChessMove(pawns, startSq, endSq, queenPromoCapture, captured) 
            }
        );
    } while (westPromoCaptures &= (westPromoCaptures - 1));
}


void ChessBoard::generateDoublePushes(std::vector<ChessMove> &t_moveList, uint64_t t_pawnSet, uint64_t t_emptySquares)
{
    const uint64_t rank4 = (uint64_t) 0x00000000ff000000;
    const uint64_t rank5 = (uint64_t) 0x00000000ff000000;

    if (m_sideToMove == white) {
        uint64_t pawnDoublePush = (t_pawnSet << 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush << 8) & t_emptySquares & rank4;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare - 16;
            t_moveList.push_back(ChessMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
    else {
        uint64_t pawnDoublePush = (t_pawnSet >> 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush >> 8) & t_emptySquares & rank5;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare + 16;
            t_moveList.push_back(ChessMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
}

std::vector<uint64_t> ChessBoard::getBitBoards()
{
    return std::vector<uint64_t>(m_bitBoard, m_bitBoard + 8);
}

// void ChessBoard::makeMove(ChessMove t_move)
// {
//     uint32_t fromIdx = t_move.getStartingSquare();
//     uint32_t toIdx = t_move.getEndSquare();
//     uint64_t fromBB = (uint64_t) 1 << fromIdx;
//     uint64_t toBB = (uint64_t) 1 << toIdx;
//     uint64_t fromToBB = fromBB ^ toBB;
//     uint32_t piece = t_move.getPiece();
    

//     uint32_t moveFlag = t_move.getFlags();
    
//     if (t_move.isCapture()){
//             uint32_t capturedPiece = t_move.getCaptured();
//             m_bitBoard[1-m_sideToMove] ^= toBB;
//             m_bitBoard[capturedPiece] ^= toBB;
//     }
    
//     if (t_move.isDoublePush()){
//         // Update en passant
//         if (m_sideToMove == white){
//                 m_enPassantSquare = fromBB << 8;
//             }
//             else {
//                 m_enPassantSquare = fromBB >> 8;
//             }
//     }   

//     if (t_move.isPromo()){
//         // Manage promotion
//         m_bitBoard[m_sideToMove] ^= fromToBB;
//         m_bitBoard[piece] ^= fromBB;

//         switch (t_move.getFlags())
//         {
//         case knightPromo:
//         case knightPromoCapture:
//             m_bitBoard[knights] ^= toBB;
//             break;
//         case bishopPromo:
//         case bishopPromoCapture:
//             m_bitBoard[bishops] ^= toBB;
//             break;
//         case rookPromo:
//         case rookPromoCapture:
//             m_bitBoard[rooks] ^= toBB;
//             break;
//         case queenPromo:
//         case queenPromoCapture:
//             m_bitBoard[queens] ^= toBB;
//             break;
//         default:
//             break;
//         }
//     }
//     else {
//         //Update bitboards
//         m_bitBoard[m_sideToMove] ^= fromToBB;
//         m_bitBoard[piece] ^= fromToBB;
//     }
//     toggleSideToMove();
// }

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
    else if ((m_bitBoard[knights] & mask) != 0) taken = knights;
    else if ((m_bitBoard[bishops] & mask) != 0) taken = bishops;
    else if ((m_bitBoard[rooks] & mask) != 0) taken = rooks;
    else if ((m_bitBoard[queens] & mask) != 0) taken = queens;        
    
    return taken;
}

uint64_t ChessBoard::getAttackSet(int t_pieceType, uint64_t t_occupied, int t_square)
{
    uint64_t attackSet;
    switch (t_pieceType) {
        case knights: attackSet = m_lookup.knightAttacks(t_square); break;
        case bishops: attackSet = m_lookup.bishopAttacks(t_occupied, t_square);break;
        case rooks: attackSet = m_lookup.rookAttacks(t_occupied, t_square);break;
        case queens: attackSet = m_lookup.queenAttacks(t_occupied, t_square);break;
        case kings: attackSet = m_lookup.kingAttacks(t_square);break;
        default: attackSet = 0;
    }
    return attackSet;
}

// bool ChessBoard::isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide)
// {
//     uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[t_attackingSide];
//     if ((m_Lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;

//     uint64_t knightsSet = m_bitBoard[knights] & m_bitBoard[t_attackingSide];
//     if ((m_Lookup.knightAttacks(t_square) & knightsSet) != 0) return true;

//     uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[t_attackingSide];
//     if ((m_Lookup.kingAttacks(t_square) & kingSet) != 0) return true;

//     uint64_t rooksQueensSet = (m_bitBoard[queens] | m_bitBoard[rooks]) & m_bitBoard[t_attackingSide];
//     if ((m_Lookup.rookAttacks(t_occupied, t_square) & rooksQueensSet) != 0) return true;

//     uint64_t bishopsQueensSet = (m_bitBoard[queens] | m_bitBoard[bishops]) & m_bitBoard[t_attackingSide];
//     if ((m_Lookup.bishopAttacks(t_occupied, t_square) & bishopsQueensSet) != 0) return true;  

//     return false;
// }

// bool ChessBoard::isCheck(int t_attackingSide)
// {
//     uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[1 - t_attackingSide];
//     uint64_t occupied = m_bitBoard[white] | m_bitBoard[black];
//     int kingSquare = btw::bitScanForward(kingSet);

//     return isSquareAttacked(occupied, kingSquare, t_attackingSide);
// }
