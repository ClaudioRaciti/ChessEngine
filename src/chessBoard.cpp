#include "chessBoard.h"

#include <cassert>
#include <bitset>
#include <iostream>

enum rayDirections {
    soWe, sout, soEa, west, east, noWe, nort, noEa
};

enum enumSquare {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum pieceType {
    white, black, pawns, knights, bishops, rooks, queens, kings
};

enum moveType {
    quiet, doublePush, kingCastle, queenCastle, capture, enPassant, 
    knightPromo = 8, bishopPromo, rookPromo, queenPromo, 
    knightPromoCapture,  bishopPromoCapture, rookPromoCapture, queenPromoCapture
};

Board::Board()
{
    initBoard();
    initRayAttacks();
    initKnightAttacks();
    initKingAttacks();
    initPawnAttacks();
    m_sideToMove = white;
    generateMoveList();
}

bool Board::isIllegal()
{
    return isCheck(m_sideToMove);
}

std::vector<CMove> Board::getMoveList()
{
    return m_moveList;
}

std::vector<uint64_t> Board::getBitBoards()
{
    return std::vector<uint64_t>(m_bitBoard, m_bitBoard + 8);
}

void Board::makeMove(CMove t_move)
{
    uint32_t fromIdx = t_move.getStartingSquare();
    uint32_t toIdx = t_move.getEndSquare();
    uint64_t fromBB = (uint64_t) 1 << fromIdx;
    uint64_t toBB = (uint64_t) 1 << toIdx;
    uint64_t fromToBB = fromBB ^ toBB;
    uint32_t piece = t_move.getPiece();
    
    
    if (t_move.isCapture()){
        if (t_move.isEnPassant()){
            uint32_t taken = pawns;

            t_move.setTaken(taken);
            if (m_sideToMove == white){
                m_bitBoard[1-m_sideToMove] ^= toBB >> 8;
                m_bitBoard[taken] ^= toBB >> 8;
            }
            else {
                m_bitBoard[1-m_sideToMove] ^= toBB << 8;
                m_bitBoard[taken] ^= toBB << 8;
            }
        }
        else {
            uint32_t taken = takenPiece(toIdx);

            // Taken piece must be valid
            assert(taken >= 2 && taken <=6);

            t_move.setTaken(taken);
            m_bitBoard[1-m_sideToMove] ^= toBB;
            m_bitBoard[taken] ^= toBB;
        }    
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
        // Update bitboards
        m_bitBoard[m_sideToMove] ^= fromToBB;
        m_bitBoard[piece] ^= fromToBB;
    }
    
    m_moveHist.push_back(t_move);
    toggleSideToMove();
    generateMoveList();
}

void Board::initBoard(){
    m_bitBoard[white]   = (uint64_t) 0x000000000000ffff;
    m_bitBoard[black]   = (uint64_t) 0xffff000000000000;
    m_bitBoard[pawns]   = (uint64_t) 0x00ff00000000ff00;
    m_bitBoard[knights] = (uint64_t) 0x4200000000000042;
    m_bitBoard[bishops] = (uint64_t) 0x2400000000000024;
    m_bitBoard[rooks]   = (uint64_t) 0x8100000000000081;
    m_bitBoard[queens]  = (uint64_t) 0x0800000000000008;
    m_bitBoard[kings]   = (uint64_t) 0x1000000000000010;
}

void Board::initRayAttacks(){
    uint64_t nortRay =  (uint64_t) 0x0101010101010100;
    for (int sq = 0; sq < 64; sq ++, nortRay <<= 1)
        m_rayAttacks[sq][nort] = nortRay;


    uint64_t noEaRay = (uint64_t) 0x8040201008040200;
    for (int file = 0; file < 8; file ++, wrapEast(noEaRay)){
        uint64_t wrappedRay = noEaRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noEa] = wrappedRay;
    }

    uint64_t noWeRay = (uint64_t) 0x102040810204000;
    for (int file = 7; file >= 0; file --, wrapWest(noWeRay)){
        uint64_t wrappedRay = noWeRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][noWe] = wrappedRay;
    }

    uint64_t eastRay = (uint64_t) 0x0000000000000fe;
    for (int file = 0; file < 8; file ++, wrapEast(eastRay)){
        uint64_t wrappedRay = eastRay;
        for (int rank = 0; rank < 8; rank ++, wrappedRay <<= 8)
            m_rayAttacks[rank * 8 + file][east] = wrappedRay;
    }

    uint64_t soutRay = (uint64_t) 0x0080808080808080;
    for (int sq = 63; sq >= 0; sq --, soutRay >>= 1)
        m_rayAttacks[sq][sout] = soutRay;

    uint64_t soEaRay = (uint64_t) 0x0002040810204080;
    for (int file = 0; file < 8; file ++, wrapEast(soEaRay)){
        uint64_t wrappedRay = soEaRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soEa] = wrappedRay;
    }

    uint64_t soWeRay = (uint64_t) 0x0040201008040201;
    for (int file = 7; file >= 0; file --, wrapWest(soWeRay)){
        uint64_t wrappedRay = soWeRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][soWe] = wrappedRay;
    }

    uint64_t westRay = (uint64_t) 0x7f00000000000000;
    for (int file = 7; file >= 0; file --, wrapWest(westRay)){
        uint64_t wrappedRay = westRay;
        for (int rank = 7; rank >= 0; rank --, wrappedRay >>= 8)
            m_rayAttacks[rank * 8 + file][west] = wrappedRay;
    }
}

void Board::initKnightAttacks(){
    uint64_t knightPos = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, knightPos <<= 1){
        m_knightAttacks[sq] = (uint64_t) 0;

        uint64_t eastDir = cpyWrapEast(knightPos);
        uint64_t eaEaDir = cpyWrapEast(eastDir);
        m_knightAttacks[sq] |= eastDir << 16 | eaEaDir << 8 
            | eastDir >> 16 | eaEaDir >> 8;

        uint64_t westDir = cpyWrapWest(knightPos);
        uint64_t weWeDir = cpyWrapWest(westDir);
        m_knightAttacks[sq] |= westDir << 16 | weWeDir << 8
            | westDir >> 16 | weWeDir >> 8;
    }
}

void Board::initKingAttacks(){
    uint64_t kingPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++, kingPosition <<= 1){
        m_kingAttacks[sq] = kingPosition << 8 | kingPosition >> 8;
        uint64_t eastDir = cpyWrapEast(kingPosition);
        m_kingAttacks[sq] |= eastDir | eastDir << 8 | eastDir >> 8;
        uint64_t westDir = cpyWrapWest(kingPosition);
        m_kingAttacks[sq] |= westDir | westDir << 8 | westDir >> 8;
    }
}

void Board::initPawnAttacks(){
    uint64_t pawnPosition = (uint64_t) 1;
    for (int sq = 0; sq < 64; sq ++ , pawnPosition <<= 1){
        if (sq < 8 || sq > 55){
            m_pawnAttacks[sq][white] = (uint64_t) 0;
            m_pawnAttacks[sq][black] = (uint64_t) 0;
        }
        else {
            uint64_t nortDir = pawnPosition << 8;
            uint64_t soutDir = pawnPosition >> 8;

            m_pawnAttacks[sq][white] = cpyWrapEast(nortDir) | cpyWrapWest(nortDir);
            m_pawnAttacks[sq][black] = cpyWrapEast(soutDir) | cpyWrapWest(soutDir);
        }
    }    
}

void Board::toggleSideToMove()
{
    m_sideToMove = 1 - m_sideToMove;
}

void Board::generateMoveList()
{
    m_moveList.clear();
    uint64_t enemyPcs = m_bitBoard[1 - m_sideToMove];

    generatePawnMoves();

    generatePiecesMoves(enemyPcs, knights);
    generatePiecesMoves(enemyPcs, bishops);
    generatePiecesMoves(enemyPcs, rooks);
    generatePiecesMoves(enemyPcs, queens);
    generatePiecesMoves(enemyPcs, kings);
}

void Board::generatePawnMoves()
{
    uint64_t promoMask[2] = {0x00ff000000000000, 0x000000000000ff00};
    uint64_t enPassantMask[2] = {0x0000000000000001, 0x8000000000000000};

    uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[m_sideToMove];
    uint64_t promoSet = pawnsSet & promoMask[m_sideToMove];
    uint64_t enPassantSet = m_enPassantSquare | enPassantMask[m_sideToMove];
    uint64_t enemySet = m_bitBoard[1 - m_sideToMove];
    uint64_t emptySet = ~ (m_bitBoard[white] | m_bitBoard[black]);

    pawnsSet &= ~promoSet;

    switch (m_sideToMove){
    case white:
        serializePawnMoves(wDoublePushablePawns(pawnsSet, emptySet), 16, doublePush);

        serializePawnMoves(wPushablePawns(pawnsSet, emptySet), 8, quiet);
        serializePawnMoves(wPawnsCapturingEast(pawnsSet, enemySet), 9, capture);
        serializePawnMoves(wPawnsCapturingWest(pawnsSet, enemySet), 7, capture);

        serializePawnMoves(wPawnsCapturingEast(pawnsSet, enPassantSet), 9, enPassant);
        serializePawnMoves(wPawnsCapturingWest(pawnsSet, enPassantSet), 7, enPassant);

        serializePawnPromo(wPushablePawns(promoSet, emptySet), 8, false);
        serializePawnPromo(wPawnsCapturingEast(promoSet, enemySet), 9, true);
        serializePawnPromo(wPawnsCapturingWest(promoSet, enemySet), 7, true);
        break;
    case black:
        serializePawnMoves(bDoublePushablePawns(pawnsSet, emptySet), -16, doublePush);

        serializePawnMoves(bPushablePawns(pawnsSet, emptySet), -8, quiet);
        serializePawnMoves(bPawnsCapturingEast(pawnsSet, enemySet), -7, capture);
        serializePawnMoves(bPawnsCapturingWest(pawnsSet, enemySet), -9, capture);
        
        serializePawnMoves(bPawnsCapturingEast(pawnsSet, enPassantSet), -7, enPassant);
        serializePawnMoves(bPawnsCapturingWest(pawnsSet, enPassantSet), -9, enPassant);

        serializePawnPromo(bPushablePawns(promoSet, emptySet), -8, false);
        serializePawnPromo(bPawnsCapturingEast(promoSet, enemySet), -7, true);
        serializePawnPromo(bPawnsCapturingWest(promoSet, enemySet), -9, true);
        break;
    }
}

void Board::generatePiecesMoves(uint64_t t_enemyPcs, int t_pieceType)
{
    uint64_t pieceSet = m_bitBoard[t_pieceType] & (~t_enemyPcs);
    uint64_t occupied = m_bitBoard[white] | m_bitBoard[black];
    uint64_t empty = ~occupied;

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet;

        switch (t_pieceType)
        {
        case knights: attackSet = knightAttacks(startingSquare); break;
        case bishops: attackSet = bishopAttacks(occupied, startingSquare); break;
        case rooks  : attackSet = rookAttacks(occupied, startingSquare); break;
        case queens : attackSet = queenAttacks(occupied, startingSquare); break;
        case kings  : attackSet = kingAttacks(startingSquare); break;
        default     : attackSet = 0; break;
        }

        serializeMoves(attackSet & t_enemyPcs, t_pieceType, startingSquare, capture);
        serializeMoves(attackSet & empty     , t_pieceType, startingSquare, quiet);
    } while (pieceSet &= (pieceSet -1));
}

void Board::serializePawnMoves(uint64_t t_pawns, int t_offset, int t_moveType)
{
    if(t_pawns) do {
        int startingSquare = bitScanForward(t_pawns);
        m_moveList.push_back(
            CMove(pawns, startingSquare, startingSquare + t_offset, t_moveType)
            );
    } while (t_pawns &= (t_pawns - 1));
}

void Board::serializePawnPromo(uint64_t t_pawns, int t_offset, bool t_isCapture)
{
    if (t_pawns) do {
        int startingSquare = bitScanForward(t_pawns);
        if (t_isCapture){
            m_moveList.insert (m_moveList.end(), {
                    CMove(pawns, startingSquare, startingSquare + t_offset, knightPromoCapture),
                    CMove(pawns, startingSquare, startingSquare + t_offset, bishopPromoCapture),
                    CMove(pawns, startingSquare, startingSquare + t_offset, rookPromoCapture),
                    CMove(pawns, startingSquare, startingSquare + t_offset, queenPromoCapture)
                });
        }
        else {
            m_moveList.insert (m_moveList.end(), {
                    CMove(pawns, startingSquare, startingSquare + t_offset, knightPromo),
                    CMove(pawns, startingSquare, startingSquare + t_offset, bishopPromo),
                    CMove(pawns, startingSquare, startingSquare + t_offset, rookPromo),
                    CMove(pawns, startingSquare, startingSquare + t_offset, queenPromo)
                });
        }
    } while (t_pawns &= (t_pawns - 1));
}

void Board::serializeMoves(uint64_t t_moves, int t_pieceType, int t_startingSquare, int t_moveType)
{
    if(t_moves) do {
        int endSquare = bitScanForward(t_moves);
        m_moveList.push_back(
            CMove(t_pieceType, t_startingSquare, endSquare, t_moveType)
            );
    } while (t_moves &= (t_moves - 1));
}

void Board::wrapEast(uint64_t &t_bitBoard){
    uint64_t mask = (uint64_t) 0x7f7f7f7f7f7f7f7f;
    t_bitBoard &= mask;
    t_bitBoard <<= 1;
}

uint64_t Board::cpyWrapEast(uint64_t t_bitBoard){
    wrapEast(t_bitBoard);
    return t_bitBoard;
}

void Board::wrapWest(uint64_t &t_bitBoard){
    uint64_t mask = (uint64_t) 0xfefefefefefefefe;
    t_bitBoard &= mask;
    t_bitBoard >>= 1;
}

uint64_t Board::cpyWrapWest(uint64_t t_bitBoard){
    wrapWest(t_bitBoard);
    return t_bitBoard;
}

// Finds position of Least Significant 1 Bit
int Board::bitScanForward(uint64_t t_bitBoard){
    //Reference table for bitscans
    const int index64[64] = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (t_bitBoard != 0);
    return index64[((t_bitBoard ^ (t_bitBoard-1)) * deBrujin64) >> 58];
}

// Finds position of Most Significant 1 Bit
int Board::bitScanReverse(uint64_t t_bitBoard){
    // Reference table for bitscans
    const int index64[64] = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    const uint64_t deBrujin64 = (uint64_t) 0x03f79d71b4cb0a89;
    assert (t_bitBoard != 0);
    t_bitBoard |= t_bitBoard >> 1;
    t_bitBoard |= t_bitBoard >> 2;
    t_bitBoard |= t_bitBoard >> 4;
    t_bitBoard |= t_bitBoard >> 8;
    t_bitBoard |= t_bitBoard >> 16;
    t_bitBoard |= t_bitBoard >> 32;
    return index64[(t_bitBoard * deBrujin64) >> 58];
}

uint64_t Board::getRayAttacks(uint64_t t_occupied, int t_direction, int t_square){
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
    int blockerSq = bitScanReverse(blocker);
    return (attacks ^ m_rayAttacks[blockerSq][t_direction]);
}

uint64_t Board::rookAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, nort, t_square) | getRayAttacks(t_occupied, sout, t_square)
        | getRayAttacks(t_occupied, east, t_square) | getRayAttacks(t_occupied, west, t_square);
}

uint64_t Board::bishopAttacks(uint64_t t_occupied, int t_square){
    return getRayAttacks(t_occupied, noWe, t_square) | getRayAttacks(t_occupied, noEa, t_square)
        | getRayAttacks(t_occupied, soWe, t_square) | getRayAttacks(t_occupied, soEa, t_square);
}

uint64_t Board::queenAttacks(uint64_t t_occupied, int t_square){
    return bishopAttacks(t_occupied, t_square) | rookAttacks(t_occupied, t_square);
}

uint64_t Board::knightAttacks(int t_square)
{
    return m_knightAttacks[t_square];
}

uint64_t Board::kingAttacks(int t_square)
{
    return m_kingAttacks[t_square];
}

uint64_t Board::wPawnAttacks(int t_square)
{
    return m_pawnAttacks[t_square][white];
}

uint64_t Board::bPawnAttacks(int t_square)
{
    return m_pawnAttacks[t_square][black];
}

uint32_t Board::takenPiece(int t_square)
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

bool Board::isSquareAttacked(uint64_t t_occupied, int t_square, int t_attackingSide)
{
    uint64_t pawnsSet = m_bitBoard[pawns] & m_bitBoard[t_attackingSide];
    if ((m_pawnAttacks[t_square][1 - t_attackingSide] & pawnsSet) != 0) return true;

    uint64_t knightsSet = m_bitBoard[knights] & m_bitBoard[t_attackingSide];
    if ((m_knightAttacks[t_square] & knightsSet) != 0) return true;

    uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[t_attackingSide];
    if ((m_kingAttacks[t_square] & kingSet) != 0) return true;

    uint64_t rooksQueensSet = (m_bitBoard[queens] | m_bitBoard[rooks]) & m_bitBoard[t_attackingSide];
    if ((rookAttacks(t_occupied, t_square) & rooksQueensSet) != 0) return true;

    uint64_t bishopsQueensSet = (m_bitBoard[queens] | m_bitBoard[bishops]) & m_bitBoard[t_attackingSide];
    if ((bishopAttacks(t_occupied, t_square) & bishopsQueensSet) != 0) return true;  

    return false;
}

bool Board::isCheck(int t_attackingSide)
{
    uint64_t kingSet = m_bitBoard[kings] & m_bitBoard[1 - t_attackingSide];
    uint64_t occupied = m_bitBoard[white] | m_bitBoard[black];
    int kingSquare = bitScanForward(kingSet);

    return isSquareAttacked(occupied, kingSquare, t_attackingSide);
}

uint64_t Board::wPawnsCapturingEast(uint64_t t_wPawns, uint64_t t_bPieces)
{
    return (cpyWrapWest(t_bPieces) >> 8) & t_wPawns;
}

uint64_t Board::wPawnsCapturingWest(uint64_t t_wPawns, uint64_t t_bPieces)
{
    return (cpyWrapEast(t_bPieces) >> 8) & t_wPawns;
}

uint64_t Board::bPawnsCapturingEast(uint64_t t_bPawns, uint64_t t_wPieces)
{
    return (cpyWrapWest(t_wPieces) << 8) & t_bPawns;
}

uint64_t Board::bPawnsCapturingWest(uint64_t t_bPawns, uint64_t t_wPieces)
{
    return (cpyWrapEast(t_wPieces) << 8) & t_bPawns;
}

uint64_t Board::wPushablePawns(uint64_t t_wPawns, uint64_t t_empty)
{
    return (t_empty >> 8) & t_wPawns;
}

uint64_t Board::wDoublePushablePawns(uint64_t t_wPawns, uint64_t t_empty)
{
    const uint64_t rank4 = (uint64_t) 0x00000000FF000000;
    uint64_t emptyRow3 = ((t_empty & rank4) >> 8) & t_empty;
    return wPushablePawns(t_wPawns, emptyRow3);
}

uint64_t Board::bPushablePawns(uint64_t t_bPawns, uint64_t t_empty)
{
    return (t_empty << 8) & t_bPawns;
}

uint64_t Board::bDoublePushablePawns(uint64_t t_bPawns, uint64_t t_empty)
{
    const uint64_t rank5 = (uint64_t) 0x000000FF00000000;
    uint64_t emptyRow6 = ((t_empty & rank5) << 8) & t_empty;
    return bPushablePawns(t_bPawns, emptyRow6);
}
