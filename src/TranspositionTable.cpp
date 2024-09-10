#include "TranspositionTable.h"


float TranspositionTable::getValue(const ChessBoard &t_key) 
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second.value;
    }
    throw std::runtime_error("ChessBoard not found");
}

float TranspositionTable::getDepth(const ChessBoard &t_key) 
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second.depth;
    }
    throw std::runtime_error("ChessBoard not found");
}

void TranspositionTable::insert(const ChessBoard &t_key, float t_value, int t_depth)
{
    auto it = m_cache.find(t_key);
    if (it == m_cache.end()){
        m_contents.emplace_front(t_key, Node{t_value, t_depth});
        m_cache[t_key] = m_contents.begin();
        if (m_contents.size() == m_maxSize){
            m_cache.erase(m_contents.back().first);
            m_contents.pop_back();
        }
    }
    else if (t_depth > it->second->second.depth){
        it->second->second = {t_value, t_depth};
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
    }
}

size_t TranspositionTable::getSize() const
{
    return m_contents.size();
}
