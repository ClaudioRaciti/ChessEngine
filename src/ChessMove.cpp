#include "ChessMove.h"
#include "notation.h"
#include <map>
#include <limits>

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

std::ostream &operator<<(std::ostream &os, const ChessMove &cm)
{
    std::map<int, std::string> squares{
        { 0,"a1"},{ 1,"b1"},{ 2,"c1"},{ 3,"d1"},{ 4,"e1"},{ 5,"f1"},{ 6,"g1"},{ 7,"h1"},
        { 8,"a2"},{ 9,"b2"},{10,"c2"},{11,"d2"},{12,"e2"},{13,"f2"},{14,"g2"},{15,"h2"},
        {16,"a3"},{17,"b3"},{18,"c3"},{19,"d3"},{20,"e3"},{21,"f3"},{22,"g3"},{23,"h3"},
        {24,"a4"},{25,"b4"},{26,"c4"},{27,"d4"},{28,"e4"},{29,"f4"},{30,"g4"},{31,"h4"},
        {32,"a5"},{33,"b5"},{34,"c5"},{35,"d5"},{36,"e5"},{37,"f5"},{38,"g5"},{39,"h5"},
        {40,"a6"},{41,"b6"},{42,"c6"},{43,"d6"},{44,"e6"},{45,"f6"},{46,"g6"},{47,"h6"},
        {48,"a7"},{49,"b7"},{50,"c7"},{51,"d7"},{52,"e7"},{53,"f7"},{54,"g7"},{55,"h7"},
        {56,"a8"},{57,"b8"},{58,"c8"},{59,"d8"},{60,"e8"},{61,"f8"},{62,"g8"},{63,"h8"}
    };
    
    std::map<int, std::string> pieces{
        {0,"white"},{1,"black"},{2,"pawn"},{3,"knight"},{4,"bishop"},{5,"rook"},
        {6,"queen"},{7,"king"}
    };

    std::map<int, std::string> flags{
        {0,"quiet"},{1,"double push"},{2,"kingside castle"},{3,"queenside castle"},
        {4,"capture"},{5,"en passant"},{8,"promotion to knight"},{9,"promotion to bishop"},
        {10,"promotion to rook"},{11,"promotion to queen"},{12,"capture and promotion\
        to knight"},{13,"capture and promotion to bishop"},{14,"capture and promotion\
        to rook"},{15,"capture and promotion to queen"}
    };

    os << pieces[cm.getPiece()] << " from " << squares[cm.getStartingSquare()] <<
        " to " << squares[cm.getEndSquare()] << " flag " << flags[cm.getFlags()];

    return os;
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

uint32_t ChessMove::getButterflyIndex()
{
    return m_Move & 0x0fff;
}

uint16_t ChessMove::asShort()
{
    return (uint16_t) m_Move;
}

float ChessMove::getExpectedValue()
{
    static std::map<int, float> pieceValue{{pawns, 1.0f}, {knights, 2.9f}, {bishops, 3.0f}, {rooks, 5.0f}, {queens, 10.0f}};
    if (isCapture())return pieceValue[getCaptured()] - pieceValue[getPiece()];
    return -std::numeric_limits<float>::infinity();
}
