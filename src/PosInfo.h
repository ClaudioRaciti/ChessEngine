#pragma once

class PosInfo
{
private:
    int  m_halfmoveClock = 0;
    bool m_wLongRights = true;
    bool m_wShortRights = true;
    bool m_bLongRights = true;
    bool m_bShortRights = true;
    bool m_enPassantState = false;
    int m_enPassantSq = 0;
    
public:
    PosInfo()  = default;
    ~PosInfo() = default;

    PosInfo(const PosInfo&)               = default;
    PosInfo& operator=(const PosInfo&)    = default;

public:
    bool getEnPassantState();
    void setEnPassantState(bool t_state);
    int getEnPassantSq();
    void setEnPassantSq(int t_square);

    bool getLongCastlingRights(int t_sideToMove);
    bool getShortCastlingRights(int t_sideToMove);
    void removeLongCastlingRights(int t_sideToMove);
    void removeShortCastlingRights(int t_sideToMove);

    int getHalfmoveClock();
    void incrementHalfmoveClock();
    void resetHalfmoveClock();
};

