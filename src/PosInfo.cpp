#include "PosInfo.h"
#include "notation.h"

bool PosInfo::isEpPossible() const
{
    bool enPassant = false;
    if (info & (1<<23)) enPassant = true;
    return enPassant;
}

void PosInfo::setEpState(bool t_state)
{
    info &= ~(1 << 23);
    if(t_state) info |= 1 << 23;
}

int PosInfo::getEpSquare() const
{
    return (info >> 17) & 0x3f;
}

void PosInfo::setEpSquare(int t_square)
{
    info &= ~(0x3f << 17);
    info |= (t_square & 0x3f) << 17;
}

bool PosInfo::getLongCastlingRights(int t_sideToMove) const
{
    bool rights = false;

    if(
        ((t_sideToMove == white) && (info & (1<<15))) ||
        ((t_sideToMove != white) && (info & (1<<16)))
    ) rights = true;

    return rights;
}

bool PosInfo::getShortCastlingRights(int t_sideToMove) const
{
    bool rights = false;

    if(
        ((t_sideToMove == white) && (info & (1<<13))) ||
        ((t_sideToMove != white) && (info & (1<<14)))
    ) rights = true;

    return rights;
}

void PosInfo::removeLongCastlingRights(int t_sideToMove) 
{
    if(t_sideToMove == white){
        info &= ~(1<<15);
    }
    else {
        info &= ~(1<<16);
    }
}

void PosInfo::removeShortCastlingRights(int t_sideToMove) 
{
    if(t_sideToMove == white){
        info &= ~(1<<13);
    }
    else {
        info &= ~(1<<14);
    }
}

int PosInfo::getHalfmoveClock() const
{
    return info & 0x1fff;
}

void PosInfo::incrementHalfmoveClock()
{
    info += 1;
}

void PosInfo::resetHalfmoveClock()
{
    info &= 0xffffe000;
}
