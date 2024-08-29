#pragma once

#include <cstdint>

class PosInfo
{
private:
    uint32_t info = 0x1e000;
    
public:
    PosInfo()  = default;
    ~PosInfo() = default;

    PosInfo(const PosInfo&)               = default;
    PosInfo& operator=(const PosInfo&)    = default;

public:
    bool isEpPossible() const;
    void setEpState(bool t_state);
    int getEpSquare() const;
    void setEpSquare(int t_square);

    bool getLongCastlingRights(int t_sideToMove) const;
    bool getShortCastlingRights(int t_sideToMove) const;
    void removeLongCastlingRights(int t_sideToMove);
    void removeShortCastlingRights(int t_sideToMove);

    int getHalfmoveClock() const;
    void incrementHalfmoveClock();
    void resetHalfmoveClock();

    inline int getInfo() const {return info >> 13;};
};

