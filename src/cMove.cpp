#include "cMove.h"


CMove::CMove(uint32_t from, uint32_t to, uint32_t flag)
{
    m_Move = ((flag & 0x0f) << 12|(from & 0x3f) << 6 |(to & 0x3f));
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

void CMove::setEndSquare(uint32_t to) 
{
    m_Move &= ~0x3f; 
    m_Move |= (to & 0x3f);
}

void CMove::setStartingSquare(uint32_t from)
{
    m_Move &= ~0xfc0; 
    m_Move |= (from & 0x3f) << 6;
}

void CMove::setFlags(uint32_t flag)
{
    m_Move &= ~0xf000;
    m_Move |= (flag & 0x0f) << 12;
}

uint32_t CMove::getButterflyIndex()
{
    return m_Move & 0x0fff;
}

uint16_t CMove::asShort()
{
    return (uint16_t) m_Move;
}
