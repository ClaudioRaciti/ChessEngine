#include "chessBoard.h"

#include <cassert>
#include <algorithm>

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
}

bool Board::isIllegal()
{
    return isCheck(m_sideToMove);
}

std::vector<CMove> Board::getMoveList()
{
    std::vector<CMove> moveList;

    generatePieceMoves(knights, moveList);
    generatePieceMoves(bishops, moveList);
    generatePieceMoves(rooks  , moveList);
    generatePieceMoves(queens , moveList);
    generatePieceMoves(kings  , moveList);
    generatePawnsMoves(moveList);    

    return moveList;
}

void Board::generatePieceMoves(int t_pieceType, std::vector<CMove>& t_moveList)
{
    uint64_t pieceSet = m_bitBoard[t_pieceType] & m_bitBoard[m_sideToMove];
    uint64_t occupiedSquares = m_bitBoard[black] | m_bitBoard[white];
    uint64_t emptySquares = ~occupiedSquares;
    uint64_t enemyPieces = m_bitBoard[1 - m_sideToMove];

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = getAttackSet(t_pieceType, occupiedSquares, startingSquare);

        //serialize all captures moves
        uint64_t captures = attackSet & enemyPieces;
        if (captures) do {
            int endSquare = bitScanForward(captures);
            t_moveList.push_back(
                CMove(t_pieceType, startingSquare, endSquare, capture, capturedPiece(endSquare))
            );
        } while (captures &= (captures -1 ));

        //serialize all non-capture moves
        uint64_t quietMoves    = attackSet & emptySquares;
        if (quietMoves) do {
            int endSquare = bitScanForward(quietMoves);
            t_moveList.push_back(
                CMove(t_pieceType, startingSquare, endSquare, quiet)
            );
        } while (quietMoves &= (quietMoves -1 ));
    } while (pieceSet &= (pieceSet - 1));
}

void Board::generatePawnsMoves(std::vector<CMove> &moveList)
{
    // generate pawn moves
    uint64_t pawnSet = m_bitBoard[pawns] & m_bitBoard[m_sideToMove];
    uint64_t enemyPieces = m_bitBoard[1 - m_sideToMove];
    uint64_t occupiedSquares = m_bitBoard[black] | m_bitBoard[white];
    uint64_t emptySquares = ~occupiedSquares;
    uint64_t promoSquares = (uint64_t) 0xff000000000000ff;
    uint64_t nonPromoSquares = (uint64_t) 0x00ffffffffffff00;
    uint64_t rank4 = (uint64_t) 0x00000000ff000000;
    uint64_t rank5 = (uint64_t) 0x00000000ff000000;

    // loop over all pawns
    if (pawnSet) do {
        int startingSquare = bitScanForward(pawnSet);

        //serialize pawn captures
        uint64_t pawnCaptures = m_pawnAttacks[startingSquare][m_sideToMove] & enemyPieces & nonPromoSquares;
        if (pawnCaptures) do {
            int endSquare = bitScanForward(pawnCaptures);
            moveList.push_back(CMove(pawns, startingSquare, endSquare, capture, capturedPiece(endSquare)));
        } while (pawnCaptures &= (pawnCaptures - 1));

        //serialize pawn pushes
        uint64_t pawnMoves = m_pawnPushes[startingSquare][m_sideToMove] & emptySquares & nonPromoSquares;
        if (pawnMoves) do {
            int endSquare = bitScanForward(pawnMoves);
            moveList.push_back(CMove(pawns, startingSquare, endSquare, quiet));
        } while (pawnMoves &= (pawnMoves - 1));

        //serialize pawn captures with promotion
        uint64_t promoCaptures = m_pawnAttacks[startingSquare][m_sideToMove] & enemyPieces & promoSquares;
        if (promoCaptures) do {
            int endSquare = bitScanForward(promoCaptures);
            moveList.insert(
                moveList.end(), 
                {   CMove(pawns, startingSquare, endSquare, knightPromoCapture, capturedPiece(endSquare)),
                    CMove(pawns, startingSquare, endSquare, bishopPromoCapture, capturedPiece(endSquare)),
                    CMove(pawns, startingSquare, endSquare, rookPromoCapture, capturedPiece(endSquare)),
                    CMove(pawns, startingSquare, endSquare, queenPromoCapture, capturedPiece(endSquare))    }
            );
        } while (promoCaptures &= (promoCaptures - 1));

        //serialize pawn promotions
        uint64_t pawnPromo = m_pawnPushes[startingSquare][m_sideToMove] & emptySquares & promoSquares;
        if (pawnPromo) do {
            int endSquare = bitScanForward(pawnPromo);
            moveList.insert(
                moveList.end(), 
                {   CMove(pawns, startingSquare, endSquare, knightPromo),
                    CMove(pawns, startingSquare, endSquare, bishopPromo),
                    CMove(pawns, startingSquare, endSquare, rookPromo),
                    CMove(pawns, startingSquare, endSquare, queenPromo)    }
            );
        } while (pawnPromo &= (pawnPromo - 1));

        uint64_t pawnDoublePush;
        if (m_sideToMove == white) {
            pawnDoublePush = m_pawnPushes[startingSquare][m_sideToMove] & emptySquares;
            pawnDoublePush <<= 8;
            pawnDoublePush &= (emptySquares & rank4);
        }
        else {
            pawnDoublePush = m_pawnPushes[startingSquare][m_sideToMove] & emptySquares;
            pawnDoublePush >>= 8;
            pawnDoublePush &= (emptySquares & rank5);
        }
        if (pawnDoublePush) do {
            int endSquare = bitScanForward(pawnDoublePush);
            moveList.push_back(CMove(pawns, startingSquare, endSquare, doublePush));
        } while (pawnDoublePush  &= (pawnDoublePush - 1));

        //TO-DO: manage en passant
        //
        //
        //
        //
        //

    } while (pawnSet &= (pawnSet - 1));
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
        uint64_t nortDir = pawnPosition << 8;
        uint64_t soutDir = pawnPosition >> 8;

        m_pawnAttacks[sq][white] = cpyWrapEast(nortDir) | cpyWrapWest(nortDir);
        m_pawnAttacks[sq][black] = cpyWrapEast(soutDir) | cpyWrapWest(soutDir);
        m_pawnPushes [sq][white] = nortDir;
        m_pawnPushes [sq][black] = soutDir;
    }    
}

void Board::toggleSideToMove()
{
    m_sideToMove = 1 - m_sideToMove;
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

uint64_t Board::getAttackSet(int t_pieceType, uint64_t t_occupied, int t_square)
{
    uint64_t attackSet;
    switch (t_pieceType) {
        case knights: attackSet = knightAttacks(t_square); break;
        case bishops: attackSet = bishopAttacks(t_occupied, t_square);break;
        case rooks: attackSet = rookAttacks(t_occupied, t_square);break;
        case queens: attackSet = queenAttacks(t_occupied, t_square);break;
        case kings: attackSet = kingAttacks(t_square);break;
        default: attackSet = 0;
    }
    return attackSet;
}

uint32_t Board::capturedPiece(int t_square)
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
