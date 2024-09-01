#pragma once

#include <unordered_map>

#include "ChessBoard.h"

class TranspositionTable
{
private:
    std::unordered_map<ChessBoard, float, HashFunction> m_hashTable;
public:
    TranspositionTable() = default;
    ~TranspositionTable() = default;

public:
    bool doesContain(const ChessBoard &t_key) const;
    float get(const ChessBoard &t_key) const;
    void insert(const ChessBoard &t_key, float t_value);
    size_t getSize() const;
};
