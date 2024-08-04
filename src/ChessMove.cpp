#include "ChessMove.h"

enum moveType {
    quiet, doublePush, kingCastle, queenCastle, capture, enPassant, 
    knightPromo = 8, bishopPromo, rookPromo, queenPromo, 
    knightPromoCapture,  bishopPromoCapture, rookPromoCapture, queenPromoCapture
};

ChessMove::ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag)
{
    m_Move = ((t_piece & 0x0f) << 16|(t_flag & 0x0f) << 12|(t_from & 0x3f) << 6 |(t_to & 0x3f));
}

ChessMove::ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag, uint32_t t_captured)
{
    m_Move = ((t_captured & 0x0f) << 20|(t_piece & 0x0f) << 16|(t_flag & 0x0f) << 12|
        (t_from & 0x3f) << 6 |(t_to & 0x3f));
}

void ChessMove::operator= (ChessMove otherObj) 
{
    m_Move = otherObj.m_Move;
}

bool operator== (const ChessMove& thisObj, const ChessMove& otherObj)
{
    return (thisObj.m_Move & 0xffff) == (otherObj.m_Move & 0xffff);
}

bool operator!= (const ChessMove& thisObj, const ChessMove& otherObj)
{
    return (thisObj.m_Move & 0xffff) != (otherObj.m_Move & 0xffff);
}

uint32_t ChessMove::getPiece()
{
    return (m_Move >> 16) & 0x0f;
}

uint32_t ChessMove::getEndSquare()
{
    return m_Move & 0x3f;
}

uint32_t ChessMove::getStartingSquare()
{
    return (m_Move >> 6) & 0x3f;
}

uint32_t ChessMove::getFlags()
{
    return (m_Move >> 12) & 0x0f;
}

uint32_t ChessMove::getCaptured()
{
    return (m_Move >> 20) & 0x0f;
}

void ChessMove::setPiece(uint32_t t_piece)
{
    m_Move &= 0xf0000;
    m_Move |= (t_piece & 0x0f) << 16;

}

void ChessMove::setEndSquare(uint32_t t_to)
{
    m_Move &= ~0x3f; 
    m_Move |= (t_to & 0x3f);
}

void ChessMove::setStartingSquare(uint32_t t_from)
{
    m_Move &= ~0xfc0; 
    m_Move |= (t_from & 0x3f) << 6;
}

void ChessMove::setFlags(uint32_t t_flag)
{
    m_Move &= ~0xf000;
    m_Move |= (t_flag & 0x0f) << 12;
}

void ChessMove::setCaptured(uint32_t t_taken)
{
    m_Move &= 0xf00000;
    m_Move |= (t_taken & 0x0f) << 20;
}

bool ChessMove::isCapture()
{   
    if ((getFlags() & 0x04)!= 0) return true;
    return false;
}

bool ChessMove::isDoublePush()
{   
    if (getFlags() == (uint32_t) doublePush) return true;
    return false;
}

bool ChessMove::isPromo()
{
    if ((getFlags() & 0x08)!= 0) return true;
    return false;
}

bool ChessMove::isEnPassant()
{
    if (getFlags() == (uint32_t) enPassant) return true;
    return false;
}

uint32_t ChessMove::getButterflyIndex()
{
    return m_Move & 0x0fff;
}

uint16_t ChessMove::asShort()
{
    return (uint16_t) m_Move;
}
