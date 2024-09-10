#pragma once

#include <unordered_map>
#include <list>

#include "ChessBoard.h"

struct Node
{
    float value;
    int depth;
};


class TranspositionTable
{
public:
    TranspositionTable(int size):m_maxSize(size){};
    ~TranspositionTable() = default;

public:
    inline bool doesContain(const ChessBoard &t_key) const {return m_cache.find(t_key) != m_cache.end();};
    
    float getValue(const ChessBoard &t_key);
    float getDepth(const ChessBoard &t_key);
    void insert(const ChessBoard &t_key, float t_value, int depth);
    size_t getSize() const;

private:
    int m_maxSize;
    std::list<std::pair<ChessBoard, Node>> m_contents;
    std::unordered_map<ChessBoard, std::list<std::pair<ChessBoard, Node>>::iterator, HashFunction> m_cache;
};



// struct CacheEntry {
//     int value;
//     int depth;
// };

// class LRUCache {
// public:
//     LRUCache(int capacity) : capacity_(capacity) {}

//     int getValue(int key) {
//         auto it = cache_.find(key);
//         if (it == cache_.end()) {
//             return -1; // Key non trovato
//         }
//         // Sposta l'elemento usato recentemente in testa alla lista
//         items_.splice(items_.begin(), items_, it->second);
//         return it->second->second.value;
//     }

//     int getDepth(int key) {
//         auto it = cache_.find(key);
//         if (it == cache_.end()) {
//             return -1; // Key non trovato
//         }
//         // Sposta l'elemento usato recentemente in testa alla lista
//         items_.splice(items_.begin(), items_, it->second);
//         return it->second->second.depth;
//     }

//     void put(int key, int value, int depth) {
//         auto it = cache_.find(key);
//         if (it != cache_.end()) {
//             // Aggiorna il valore e la profondità, poi sposta l'elemento in testa alla lista
//             it->second->second = {value, depth};
//             items_.splice(items_.begin(), items_, it->second);
//             return;
//         }
//         if (items_.size() == capacity_) {
//             // Trova l'elemento meno recentemente usato con la profondità maggiore
//             auto lru = std::max_element(items_.begin(), items_.end(),  {
//                 return a.second.depth < b.second.depth;
//             });
//             cache_.erase(lru->first);
//             items_.erase(lru);
//         }
//         // Inserisci il nuovo elemento in testa alla lista
//         items_.emplace_front(key, CacheEntry{value, depth});
//         cache_[key] = items_.begin();
//     }

// private:
//     int capacity_;
//     std::list<std::pair<int, CacheEntry>> items_;
//     std::unordered_map<int, std::list<std::pair<int, CacheEntry>>::iterator> cache_;
// };
