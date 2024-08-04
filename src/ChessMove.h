#pragma once

#include <cstdint>

class ChessMove{
public:
    ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag);
    ChessMove(uint32_t t_piece, uint32_t t_from, uint32_t t_to, uint32_t t_flag, uint32_t t_captured);

    void operator= (ChessMove otherObj);
    friend bool operator== (const ChessMove& thisObj, const ChessMove& otherObj);
    friend bool operator!= (const ChessMove& thisObj, const ChessMove& otherObj);


    uint32_t getPiece();
    uint32_t getEndSquare();
    uint32_t getStartingSquare();
    uint32_t getFlags();
    uint32_t getCaptured();

    void setPiece(uint32_t t_piece);
    void setEndSquare(uint32_t t_to);
    void setStartingSquare(uint32_t t_from);
    void setFlags(uint32_t t_flag);
    void setCaptured(uint32_t t_taken);

    bool isCapture();
    bool isDoublePush();
    bool isPromo();
    bool isEnPassant();

    uint32_t getButterflyIndex();
    uint16_t asShort();

private:
    uint32_t m_Move = 0;
};