#pragma once

#include <cstdint>

class CMove{
public:
    CMove(uint32_t from, uint32_t to, uint32_t flag);

    void operator= (CMove otherObj);
    friend bool operator== (const CMove& thisObj, const CMove& otherObj);
    friend bool operator!= (const CMove& thisObj, const CMove& otherObj);


    uint32_t getEndSquare();
    uint32_t getStartingSquare();
    uint32_t getFlags();

    void setEndSquare(uint32_t to);
    void setStartingSquare(uint32_t from);
    void setFlags(uint32_t flag);

    uint32_t getButterflyIndex();
    uint16_t asShort();

private:
    uint32_t m_Move;

};