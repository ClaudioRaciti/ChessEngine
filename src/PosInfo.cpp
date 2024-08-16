#include "PosInfo.h"

enum pieceType {
    white, black
};

bool PosInfo::isEpPossible() const
{
    return m_enPassantState;
}

void PosInfo::setEpState(bool t_state)
{
    m_enPassantState = t_state;
}

int PosInfo::getEpSquare() const
{
    return m_enPassantSq;
}

void PosInfo::setEpSquare(int t_square)
{
    m_enPassantSq = t_square;
}

bool PosInfo::getLongCastlingRights(int t_sideToMove) const
{
    bool longCastlingRights;
    
    if(t_sideToMove == white){
        longCastlingRights = m_wLongRights;
    }
    else {
        longCastlingRights = m_bLongRights;
    }

    return longCastlingRights;
}

bool PosInfo::getShortCastlingRights(int t_sideToMove) const
{
    bool shortCastlingRights;

    if(t_sideToMove == white){
        shortCastlingRights = m_wShortRights;
    }
    else {
        shortCastlingRights = m_bShortRights;
    }

    return shortCastlingRights;
}

void PosInfo::removeLongCastlingRights(int t_sideToMove) 
{
    if(t_sideToMove == white){
        m_wLongRights = false;
    }
    else {
        m_bLongRights = false;
    }
}

void PosInfo::removeShortCastlingRights(int t_sideToMove) 
{
    if(t_sideToMove == white){
        m_wShortRights = false;
    }
    else {
        m_bShortRights = false;
    }
}

int PosInfo::getHalfmoveClock() const
{
    return m_halfmoveClock;
}

void PosInfo::incrementHalfmoveClock()
{
    m_halfmoveClock += 1;
}

void PosInfo::resetHalfmoveClock()
{
    m_halfmoveClock = 0;
}
