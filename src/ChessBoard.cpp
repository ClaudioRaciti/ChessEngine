#include "ChessBoard.h"

#include <cassert>
#include <algorithm>
#include <unordered_map>

#include "utils.h"
#include "notation.h"

ChessBoard::ChessBoard() : m_sideToMove(white)
{
    initBoard();
    m_kingSquare[white] = btw::bitScanForward(m_bitBoard[white] & m_bitBoard[kings]);
    m_kingSquare[black] = btw::bitScanForward(m_bitBoard[black] & m_bitBoard[kings]);
    m_posHistory.emplace_back(PosInfo());
}

ChessBoard::ChessBoard(const ChessBoard &t_other) :  
    m_sideToMove{
        t_other.m_sideToMove
    },  
    m_kingSquare{
        t_other.m_kingSquare[0], t_other.m_kingSquare[1]
    },
    m_bitBoard{
        t_other.m_bitBoard[0], t_other.m_bitBoard[1], t_other.m_bitBoard[2], t_other.m_bitBoard[3],
        t_other.m_bitBoard[4], t_other.m_bitBoard[5], t_other.m_bitBoard[6], t_other.m_bitBoard[7]
    }
{
    m_posHistory.reserve(1);
    m_posHistory.emplace_back(t_other.m_posHistory.back());
}

bool ChessBoard::operator==(const ChessBoard &t_other) const
{
    return m_sideToMove == t_other.m_sideToMove 
        && std::equal(std::begin(m_bitBoard), std::end(m_bitBoard), std::begin(t_other.m_bitBoard), std::end(t_other.m_bitBoard))
        && m_posHistory.back().getInfo() == t_other.m_posHistory.back().getInfo();
}

bool ChessBoard::isIllegal()
{
    return isCheck(m_sideToMove);
}

bool ChessBoard::isLegal()
{
    return !isIllegal();
}

std::vector<ChessMove> ChessBoard::getMoveList()
{
    std::vector<ChessMove> moveList;
    moveList.reserve(256); // over the maximum number of moves possible for any legal position

    generatePawnsMoves(moveList);
    generatePieceMoves(knights, moveList);
    generatePieceMoves(bishops, moveList);
    generatePieceMoves(rooks  , moveList);
    generatePieceMoves(queens , moveList);
    generatePieceMoves(kings  , moveList);    
    generateCastles(moveList);

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
            t_moveList.emplace_back(
                ChessMove(t_pieceType, startingSquare, endSquare, capture, capturedPiece(endSquare))
            );
        } while (captures &= (captures -1 ));

        //serialize all non-capture moves
        uint64_t quietMoves    = attackSet & emptySquares;
        if (quietMoves) do {
            int endSquare = btw::bitScanForward(quietMoves);
            t_moveList.emplace_back(
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

    if(m_posHistory.back().isEpPossible())
        generateEpCaptures(t_moveList, pawnsSet, m_posHistory.back().getEpSquare());
}

void ChessBoard::generatePawnsPushes(std::vector<ChessMove> &t_moveList, 
    uint64_t t_pawnsSet, uint64_t t_emptySet)
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
        t_moveList.emplace_back(ChessMove(pawns, startSq, endSq, quiet));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = btw::bitScanForward(promoSet);
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

void ChessBoard::generatePawnsCaptures(std::vector<ChessMove> &t_moveList,
    uint64_t t_pawnsSet, uint64_t t_enemyPcs)
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
        eastOffset = -9;
        westOffset = -7;
    }
    else {
        eastCaptures = (btw::cpyWrapEast(t_pawnsSet) >> 8 & t_enemyPcs) & ~row1;
        westCaptures = (btw::cpyWrapWest(t_pawnsSet) >> 8 & t_enemyPcs) & ~row1;
        eastPromoCaptures = (btw::cpyWrapEast(t_pawnsSet) >> 8 & t_enemyPcs) & row1;
        westPromoCaptures = (btw::cpyWrapWest(t_pawnsSet) >> 8 & t_enemyPcs) & row1;
        eastOffset = 7;
        westOffset = 9;
    }

    if (westCaptures) do {
        int endSq = btw::bitScanForward(westCaptures);
        int startSq = endSq + westOffset;
        int captured = capturedPiece(endSq);
        t_moveList.emplace_back(ChessMove(pawns, startSq, endSq, capture, captured));
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = btw::bitScanForward(eastCaptures);
        int startSq = endSq + eastOffset;
        int captured = capturedPiece(endSq);
        t_moveList.emplace_back(ChessMove(pawns, startSq, endSq, capture, captured));
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


void ChessBoard::generateDoublePushes(std::vector<ChessMove> &t_moveList, 
    uint64_t t_pawnSet, uint64_t t_emptySquares)
{
    const uint64_t rank4 = (uint64_t) 0x00000000ff000000;
    const uint64_t rank5 = (uint64_t) 0x000000ff00000000;

    if (m_sideToMove == white) {
        uint64_t pawnDoublePush = (t_pawnSet << 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush << 8) & t_emptySquares & rank4;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare - 16;
            t_moveList.emplace_back(ChessMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
    else {
        uint64_t pawnDoublePush = (t_pawnSet >> 8) & t_emptySquares;
        pawnDoublePush = (pawnDoublePush >> 8) & t_emptySquares & rank5;

        if (pawnDoublePush) do {
            int endSquare = btw::bitScanForward(pawnDoublePush);
            int startingSquare = endSquare + 16;
            t_moveList.emplace_back(ChessMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));
    }
}

void ChessBoard::generateEpCaptures(std::vector<ChessMove> &t_moveList, uint64_t t_pawnsSet, int t_epSquare)
{
    int endSquare;
    uint64_t epMask = (uint64_t) 1 << t_epSquare;

    if (m_sideToMove == white) endSquare = t_epSquare + 8;     
    else endSquare = t_epSquare - 8;

    if(btw::cpyWrapEast(epMask) & t_pawnsSet)
        t_moveList.emplace_back(ChessMove(pawns, t_epSquare + 1, endSquare, enPassant, pawns));

    if(btw::cpyWrapWest(epMask) & t_pawnsSet)
        t_moveList.emplace_back(ChessMove(pawns, t_epSquare - 1, endSquare, enPassant, pawns));
}

void ChessBoard::generateCastles(std::vector<ChessMove> &t_moveList)
{
    uint64_t occupied = (m_bitBoard[black] | m_bitBoard[white]);
    uint64_t emptySet = ~occupied;

    if (m_sideToMove == white){
        const uint64_t longCastleSquares = (uint64_t) 0x000000000000000e;
        const uint64_t shortCastleSquares = (uint64_t) 0x0000000000000060;
        uint64_t intersection = longCastleSquares & emptySet;
        if(
            m_posHistory.back().getLongCastlingRights(white) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(occupied, c1, black) &&
            ! isSquareAttacked(occupied, d1, black) &&
            ! isSquareAttacked(occupied, e1, black)
        ) t_moveList.emplace_back(ChessMove(kings, e1, c1, queenCastle));
        
        intersection = shortCastleSquares & emptySet; 
        if(
            m_posHistory.back().getShortCastlingRights(white) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(occupied, e1, black) &&
            ! isSquareAttacked(occupied, f1, black) &&
            ! isSquareAttacked(occupied, g1, black)
        ) t_moveList.emplace_back(ChessMove(kings, e1, g1, kingCastle));
    }
    else{
        const uint64_t longCastleSquares = (uint64_t) 0x0e00000000000000;
        const uint64_t shortCastleSquares = (uint64_t) 0x6000000000000000;
        uint64_t intersection = longCastleSquares & emptySet; 
        if(
            m_posHistory.back().getLongCastlingRights(black) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(occupied, c8, white) &&
            ! isSquareAttacked(occupied, d8, white) &&
            ! isSquareAttacked(occupied, e8, white)
        ) t_moveList.emplace_back(ChessMove(kings, e8, c8, queenCastle));

        intersection = shortCastleSquares & emptySet;
        if(
            m_posHistory.back().getShortCastlingRights(black) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(occupied, e8, white) &&
            ! isSquareAttacked(occupied, f8, white) &&
            ! isSquareAttacked(occupied, g8, white)
        ) t_moveList.emplace_back(ChessMove(kings, e8, g8, kingCastle));
    }
}

std::vector<uint64_t> ChessBoard::getBitBoards() const
{
    return std::vector<uint64_t>(m_bitBoard, m_bitBoard + 8);
}

void ChessBoard::makeMove(ChessMove t_move){
    PosInfo newPosInfo(m_posHistory.back());
    newPosInfo.incrementHalfmoveClock();
    newPosInfo.setEpState(false);

    uint64_t moveMask, promoMask, captureMask;

    if(t_move.getPiece() == kings) m_kingSquare[m_sideToMove] = t_move.getEndSquare();

    switch (t_move.getStartingSquare())
    {
    case a1: newPosInfo.removeLongCastlingRights(white); break;
    case a8: newPosInfo.removeLongCastlingRights(black); break;
    case h1: newPosInfo.removeShortCastlingRights(white); break;
    case h8: newPosInfo.removeShortCastlingRights(black); break;
    case e1: 
        newPosInfo.removeShortCastlingRights(white);
        newPosInfo.removeLongCastlingRights(white);
         break;
    case e8: 
        newPosInfo.removeShortCastlingRights(black);
        newPosInfo.removeLongCastlingRights(black); 
        break;
    }
    switch (t_move.getEndSquare())
    {
    case a1: newPosInfo.removeLongCastlingRights(white); break;
    case a8: newPosInfo.removeLongCastlingRights(black); break;
    case h1: newPosInfo.removeShortCastlingRights(white); break;
    case h8: newPosInfo.removeShortCastlingRights(black); break;
    case e1: 
        newPosInfo.removeShortCastlingRights(white);
        newPosInfo.removeLongCastlingRights(white);
         break;
    case e8: 
        newPosInfo.removeShortCastlingRights(black);
        newPosInfo.removeLongCastlingRights(black); 
        break;
    }

    switch (t_move.getFlags())
    {
    case quiet:
        if(t_move.getPiece() == pawns) newPosInfo.resetHalfmoveClock();
        moveMask  = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    case capture:
        newPosInfo.resetHalfmoveClock();
        captureMask = (uint64_t) 1 << t_move.getEndSquare();
        moveMask = captureMask | (uint64_t) 1 << t_move.getStartingSquare();
        
        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;

        m_bitBoard[t_move.getCaptured()] ^= captureMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;        
        break;
    case queenCastle: 
        if(m_sideToMove == white){
            m_bitBoard[white] ^= 0x000000000000001d;
            m_bitBoard[kings] ^= 0x0000000000000014;
            m_bitBoard[rooks] ^= 0x0000000000000009;
        }
        else {
            m_bitBoard[black] ^= 0x1d00000000000000;
            m_bitBoard[kings] ^= 0x1400000000000000;
            m_bitBoard[rooks] ^= 0x0900000000000000;
        }
        break;
    case kingCastle:
        if(m_sideToMove == white){
            m_bitBoard[white] ^= 0x00000000000000f0;
            m_bitBoard[kings] ^= 0x0000000000000050;
            m_bitBoard[rooks] ^= 0x00000000000000a0;
        }
        else {
            m_bitBoard[black] ^= 0xf000000000000000;
            m_bitBoard[kings] ^= 0x5000000000000000;
            m_bitBoard[rooks] ^= 0xa000000000000000;
        }
        break;
    case enPassant:
        newPosInfo.resetHalfmoveClock();
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        if(m_sideToMove == white) captureMask = (uint64_t) 1 << (t_move.getEndSquare() - 8);
        else captureMask = (uint64_t) 1 << (t_move.getEndSquare() + 8);

        m_bitBoard[pawns] ^= captureMask;
        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        newPosInfo.resetHalfmoveClock();
        captureMask = (uint64_t) 1 << t_move.getEndSquare();
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        promoMask= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getCaptured()] ^= captureMask;
        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[t_move.getPromoPiece()] ^= promoMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;
        m_bitBoard[m_sideToMove] ^= moveMask | promoMask;
        break;
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        newPosInfo.resetHalfmoveClock();
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        promoMask= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[t_move.getPromoPiece()] ^= promoMask;
        m_bitBoard[m_sideToMove] ^= moveMask | promoMask;
        break;
    case doublePush:
        newPosInfo.resetHalfmoveClock();
        newPosInfo.setEpState(true);
        newPosInfo.setEpSquare(t_move.getEndSquare());
        moveMask  = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    }

    m_posHistory.emplace_back(newPosInfo);
    toggleSideToMove();
}

void ChessBoard::undoMove(ChessMove t_move)
{
    toggleSideToMove();

    if(t_move.getPiece() == kings) m_kingSquare[m_sideToMove] = t_move.getStartingSquare();

    uint64_t moveMask, promoMask, captureMask;
    switch (t_move.getFlags())
    {
    case quiet:
        moveMask  = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    case capture:
        captureMask = (uint64_t) 1 << t_move.getEndSquare();
        moveMask = captureMask | (uint64_t) 1 << t_move.getStartingSquare();
        
        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;

        m_bitBoard[t_move.getCaptured()] ^= captureMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;        
        break;
    case queenCastle: 
        if(m_sideToMove == white){
            m_bitBoard[white] ^= 0x000000000000001d;
            m_bitBoard[kings] ^= 0x0000000000000014;
            m_bitBoard[rooks] ^= 0x0000000000000009;
        }
        else {
            m_bitBoard[black] ^= 0x1d00000000000000;
            m_bitBoard[kings] ^= 0x1400000000000000;
            m_bitBoard[rooks] ^= 0x0900000000000000;
        }
        break;
    case kingCastle:
        if(m_sideToMove == white){
            m_bitBoard[white] ^= 0x00000000000000f0;
            m_bitBoard[kings] ^= 0x0000000000000050;
            m_bitBoard[rooks] ^= 0x00000000000000a0;
        }
        else {
            m_bitBoard[black] ^= 0xf000000000000000;
            m_bitBoard[kings] ^= 0x5000000000000000;
            m_bitBoard[rooks] ^= 0xa000000000000000;
        }
        break;
    case enPassant:
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        if(m_sideToMove == white) captureMask = (uint64_t) 1 << (t_move.getEndSquare() - 8);
        else captureMask = (uint64_t) 1 << (t_move.getEndSquare() + 8);

        m_bitBoard[pawns] ^= captureMask;
        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        captureMask = (uint64_t) 1 << t_move.getEndSquare();
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        promoMask= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getCaptured()] ^= captureMask;
        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[t_move.getPromoPiece()] ^= promoMask;
        m_bitBoard[1 - m_sideToMove] ^= captureMask;
        m_bitBoard[m_sideToMove] ^= moveMask | promoMask;
        break;
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        moveMask = (uint64_t) 1 << t_move.getStartingSquare();
        promoMask= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[pawns] ^= moveMask;
        m_bitBoard[t_move.getPromoPiece()] ^= promoMask;
        m_bitBoard[m_sideToMove] ^= moveMask | promoMask;
        break;
    case doublePush:
        moveMask  = (uint64_t) 1 << t_move.getStartingSquare();
        moveMask |= (uint64_t) 1 << t_move.getEndSquare();

        m_bitBoard[t_move.getPiece()] ^= moveMask;
        m_bitBoard[m_sideToMove] ^= moveMask;
        break;
    }

    m_posHistory.pop_back();
}

bool ChessBoard::isCheck()
{
    return isCheck(1 - m_sideToMove);
}

void ChessBoard::initBoard(){
    // m_bitBoard[white]   = (uint64_t) 0x000000000000ffff;
    // m_bitBoard[black]   = (uint64_t) 0xffff000000000000;
    // m_bitBoard[pawns]   = (uint64_t) 0x00ff00000000ff00;
    // m_bitBoard[knights] = (uint64_t) 0x4200000000000042;
    // m_bitBoard[bishops] = (uint64_t) 0x2400000000000024;
    // m_bitBoard[rooks]   = (uint64_t) 0x8100000000000081;
    // m_bitBoard[queens]  = (uint64_t) 0x0800000000000008;
    // m_bitBoard[kings]   = (uint64_t) 0x1000000000000010;

    m_bitBoard[white]   = (uint64_t) 0x000000181024ff91;
    m_bitBoard[black]   = (uint64_t) 0x917d730002800000;
    m_bitBoard[pawns]   = (uint64_t) 0x002d50081280e700;
    m_bitBoard[knights] = (uint64_t) 0x0000221000040000;
    m_bitBoard[bishops] = (uint64_t) 0x0040010000001800;
    m_bitBoard[rooks]   = (uint64_t) 0x8100000000000081;
    m_bitBoard[queens]  = (uint64_t) 0x0010000000200000;
    m_bitBoard[kings]   = (uint64_t) 0x1000000000000010;

    // m_bitBoard[white]   = (uint64_t) 0x1000000000004000;
    // m_bitBoard[black]   = (uint64_t) 0x0080000000000000;
    // m_bitBoard[pawns]   = (uint64_t) 0x0000000000000000;
    // m_bitBoard[knights] = (uint64_t) 0x0000000000000000;
    // m_bitBoard[bishops] = (uint64_t) 0x0000000000000000;
    // m_bitBoard[rooks]   = (uint64_t) 0x0000000000000000;
    // m_bitBoard[queens]  = (uint64_t) 0x1000000000000000;
    // m_bitBoard[kings]   = (uint64_t) 0x0080000000004000;
}


void ChessBoard::toggleSideToMove()
{
    m_sideToMove = 1 - m_sideToMove;
}

int ChessBoard::capturedPiece(int t_square)
{
    int taken = 8;
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
    using AttackFunction = uint64_t (LookupTables::*)(uint64_t, int) const;

    static const AttackFunction attackFunctions[] = {
        &LookupTables::knightAttacks, // knights
        &LookupTables::bishopAttacks, // bishops
        &LookupTables::rookAttacks, // rooks
        &LookupTables::queenAttacks, // queens
        &LookupTables::kingAttacks // kings
    };
    
    return (m_lookup.*attackFunctions[t_pieceType - 3])(t_occupied, t_square);
}

bool ChessBoard::isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide)
{
    uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[t_attackingSide];
    if ((m_lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;

    uint64_t knightsSet = m_bitBoard[knights] & m_bitBoard[t_attackingSide];
    if ((m_lookup.knightAttacks(t_square) & knightsSet) != 0) return true;

    uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[t_attackingSide];
    if ((m_lookup.kingAttacks(t_square) & kingSet) != 0) return true;

    uint64_t rooksQueensSet = (m_bitBoard[queens] | m_bitBoard[rooks]) & m_bitBoard[t_attackingSide];
    if ((m_lookup.rookAttacks(t_occupied, t_square) & rooksQueensSet) != 0) return true;

    uint64_t bishopsQueensSet = (m_bitBoard[queens] | m_bitBoard[bishops]) & m_bitBoard[t_attackingSide];
    if ((m_lookup.bishopAttacks(t_occupied, t_square) & bishopsQueensSet) != 0) return true;  

    return false;
}

bool ChessBoard::isCheck(int t_attackingSide)
{
    uint64_t occupied = m_bitBoard[white] | m_bitBoard[black];

    return isSquareAttacked(occupied, m_kingSquare[1 - t_attackingSide], t_attackingSide);
}

std::ostream &operator<<(std::ostream &os, const ChessBoard &cb)
{   
    for (int rank = 7; rank >= 0; rank --){
        int emptySquares = 0;
        for (int file = 0; file < 8; file ++){
            uint64_t squareMask = (uint64_t) 1 << (rank * 8 + file);

            if (squareMask & cb.m_bitBoard[pawns]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "P";
                else if (squareMask & cb.m_bitBoard[black]) os << "p";
            }
            else if(squareMask & cb.m_bitBoard[knights]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "N";
                else if (squareMask & cb.m_bitBoard[black]) os << "n";
            }
            else if(squareMask & cb.m_bitBoard[bishops]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "B";
                else if (squareMask & cb.m_bitBoard[black]) os << "b";
            }
            else if(squareMask & cb.m_bitBoard[rooks]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "R";
                else if (squareMask & cb.m_bitBoard[black]) os << "r";
            }
            else if(squareMask & cb.m_bitBoard[queens]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "Q";
                else if (squareMask & cb.m_bitBoard[black]) os << "q";
            }
            else if(squareMask & cb.m_bitBoard[kings]){
                if(emptySquares) os << emptySquares;
                emptySquares = 0;
                if (squareMask & cb.m_bitBoard[white]) os << "K";
                else if (squareMask & cb.m_bitBoard[black]) os << "k";
            }
            else emptySquares ++;
        }
        if (emptySquares) os << emptySquares;
        if (rank != 0) os << "/";
    }

    if (cb.m_sideToMove == white) os << " w ";
    else os << " b ";

    PosInfo info(cb.m_posHistory.back());
    if(info.getShortCastlingRights(white)) os << "K";
    if(info.getLongCastlingRights(white)) os << "Q";
    if(info.getShortCastlingRights(black)) os << "k";
    if(info.getLongCastlingRights(black)) os << "q";

    if(info.isEpPossible()) os << " " << info.getEpSquare() << " ";
    else os << " - ";

    return os;
}

std::size_t HashFunction::operator()(const ChessBoard &obj) const
{
    std::size_t seed = 10009507;
    seed ^= std::hash<int>()(obj.m_sideToMove) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    for (const auto& bitboard : obj.m_bitBoard) {
        seed ^= std::hash<uint64_t>()(bitboard) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    seed ^= std::hash<int>()(obj.m_posHistory.back().getInfo()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
}
