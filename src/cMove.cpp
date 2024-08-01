#include "cMove.h"

enum moveType {
    quiet, doublePush, kingCastle, queenCastle, capture, enPassant, 
    knightPromo = 8, bishopPromo, rookPromo, queenPromo, 
    knightPromoCapture,  bishopPromoCapture, rookPromoCapture, queenPromoCapture
};

CMove::CMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag)
{
    m_Move = ((t_piece & 0x0f) << 16|(t_flag & 0x0f) << 12|(t_from & 0x3f) << 6 |(t_to & 0x3f));
}

CMove::CMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag, uint32_t t_captured)
{
    m_Move = ((t_captured & 0x0f) << 20|(t_piece & 0x0f) << 16|(t_flag & 0x0f) << 12|
        (t_from & 0x3f) << 6 |(t_to & 0x3f));
}

void CMove::operator= (CMove otherObj) 
{
    m_Move = otherObj.m_Move;
}

bool operator== (const CMove& thisObj, const CMove& otherObj)
{
    return (thisObj.m_Move & 0xffff) == (otherObj.m_Move & 0xffff);
}

bool operator!= (const CMove& thisObj, const CMove& otherObj)
{
    return (thisObj.m_Move & 0xffff) != (otherObj.m_Move & 0xffff);
}

uint32_t CMove::getPiece()
{
    return (m_Move >> 16) & 0x0f;
}

uint32_t CMove::getEndSquare()
{
    return m_Move & 0x3f;
}

uint32_t CMove::getStartingSquare()
{
    return (m_Move >> 6) & 0x3f;
}

uint32_t CMove::getFlags()
{
    return (m_Move >> 12) & 0x0f;
}

uint32_t CMove::getCaptured()
{
    return (m_Move >> 20) & 0x0f;
}

void CMove::setPiece(uint32_t t_piece)
{
    m_Move &= 0xf0000;
    m_Move |= (t_piece & 0x0f) << 16;

}

void CMove::setEndSquare(uint32_t t_to)
{
    m_Move &= ~0x3f; 
    m_Move |= (t_to & 0x3f);
}

void CMove::setStartingSquare(uint32_t t_from)
{
    m_Move &= ~0xfc0; 
    m_Move |= (t_from & 0x3f) << 6;
}

void CMove::setFlags(uint32_t t_flag)
{
    m_Move &= ~0xf000;
    m_Move |= (t_flag & 0x0f) << 12;
}

void CMove::setCaptured(uint32_t t_taken)
{
    m_Move &= 0xf00000;
    m_Move |= (t_taken & 0x0f) << 20;
}

bool CMove::isCapture()
{   
    if ((getFlags() & 0x04)!= 0) return true;
    return false;
}

bool CMove::isDoublePush()
{   
    if (getFlags() == (uint32_t) doublePush) return true;
    return false;
}

bool CMove::isPromo()
{
    if ((getFlags() & 0x08)!= 0) return true;
    return false;
}

bool CMove::isEnPassant()
{
    if (getFlags() == (uint32_t) enPassant) return true;
    return false;
}

uint32_t CMove::getButterflyIndex()
{
    return m_Move & 0x0fff;
}

uint16_t CMove::asShort()
{
    return (uint16_t) m_Move;
}
