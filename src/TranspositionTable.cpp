#include "TranspositionTable.h"

bool TranspositionTable::doesContain(const ChessBoard &t_key) const
{
    if (m_hashTable.find(t_key) != m_hashTable.end()) return true;
    
    return false;
}

float TranspositionTable::get(const ChessBoard &t_key) const
{
    auto it = m_hashTable.find(t_key);
    if (it != m_hashTable.end()){
        return it -> second;
    }
    throw std::runtime_error("ChessBoard not found");
}

void TranspositionTable::insert(const ChessBoard &t_key, float t_value)
{
    m_hashTable[t_key] = t_value;
}

size_t TranspositionTable::getSize() const
{
    return m_hashTable.size();
}
