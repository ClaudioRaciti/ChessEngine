#include "TranspositionTable.h"
#include "notation.h"

Value TranspositionTable::getValue(const ChessBoard &t_key)
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second;
    }
    throw std::runtime_error("ChessBoard not found");
}

bool TranspositionTable::getValue(const ChessBoard &t_key, Value &t_out)
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        t_out = it->second->second;
        return true;
    }
    return false;
}

float TranspositionTable::getScore(const ChessBoard &t_key)
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second.score;
    }
    throw std::runtime_error("ChessBoard not found");
}

int TranspositionTable::getDepth(const ChessBoard &t_key) 
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second.depth;
    }
    throw std::runtime_error("ChessBoard not found");
}

int TranspositionTable::getNodeType(const ChessBoard &t_key)
{
    auto it = m_cache.find(t_key);
    if (it != m_cache.end()){
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
        return it->second->second.nodeType;
    }
    throw std::runtime_error("ChessBoard not found");
}

void TranspositionTable::insert(const ChessBoard &t_key, float t_score, int t_depth, int t_nodeType)
{
    auto it = m_cache.find(t_key);
    if (it == m_cache.end()){
        m_contents.emplace_front(t_key, Value{t_score, t_depth, t_nodeType});
        m_cache[t_key] = m_contents.begin();
        if (m_contents.size() == m_maxSize){
            m_cache.erase(m_contents.back().first);
            m_contents.pop_back();
        }
    }
    else if(t_depth >= it->second->second.depth){
        it->second->second = Value{t_score, t_depth, t_nodeType};
        m_contents.splice(m_contents.begin(), m_contents, it -> second);
    }
}

size_t TranspositionTable::getSize() const
{
    return m_contents.size();
}
