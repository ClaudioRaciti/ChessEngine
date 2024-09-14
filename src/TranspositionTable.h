#pragma once

#include <unordered_map>
#include <list>

#include "ChessBoard.h"

struct Value
{
    float score;
    int depth;
    int nodeType;
};


class TranspositionTable
{
public:
    TranspositionTable(int size):m_maxSize(size){};
    ~TranspositionTable() = default;

public:
    inline bool doesContain(const ChessBoard &t_key) const {return m_cache.find(t_key) != m_cache.end();};
    
    Value getValue(const ChessBoard &t_key);
    bool getValue(const ChessBoard &t_key, Value &t_out);
    float getScore(const ChessBoard &t_key);
    int getDepth(const ChessBoard &t_key);
    int getNodeType(const ChessBoard & t_key);
    void insert(const ChessBoard &t_key, float t_value, int t_depth, int t_nodeType);
    size_t getSize() const;

private:
    int m_maxSize;
    std::list<std::pair<ChessBoard, Value>> m_contents;
    std::unordered_map<ChessBoard, std::list<std::pair<ChessBoard, Value>>::iterator, HashFunction> m_cache;
};
