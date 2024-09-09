#pragma once

#include <cstdint>
#include <iostream>

class ChessMove{
public:
    ChessMove() = default;
    ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag);
    ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag, uint32_t t_captured);

    void operator= (ChessMove otherObj);
    friend bool operator== (const ChessMove& thisObj, const ChessMove& otherObj);
    friend bool operator!= (const ChessMove& thisObj, const ChessMove& otherObj);
    friend std::ostream& operator<< (std::ostream& os, const ChessMove& cm);

public:
    inline uint32_t getPiece() const {return (m_Move >> 16) & 0x0f;};
    inline uint32_t getEndSquare() const {return m_Move & 0x3f;};
    inline uint32_t getStartingSquare() const {return (m_Move >> 6) & 0x3f;};
    inline uint32_t getFlags() const {return (m_Move >> 12) & 0x0f;};
    inline uint32_t getCaptured() const {return (m_Move >> 20) & 0x0f;};
    inline uint32_t getPromoPiece() const {return (getFlags() & 0x03) + 3;};

    void setPiece(uint32_t t_piece);
    void setEndSquare(uint32_t t_to);
    void setStartingSquare(uint32_t t_from);
    void setFlags(uint32_t t_flag);
    void setCaptured(uint32_t t_taken);

    inline bool isCapture() const {return (getFlags() & 0x04) != 0;};
    inline bool isDoublePush() const {return getFlags() == 1;};
    inline bool isPromo() const {return (getFlags() & 0x08) != 0;};
    inline bool isEnPassant() const {return getFlags() == 5;};

    uint32_t getButterflyIndex();
    uint16_t asShort();

    float getExpectedValue();

private:
    uint32_t m_Move = 0;
};