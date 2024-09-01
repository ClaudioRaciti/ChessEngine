#include <bitset>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <math.h>

#include "src/ChessBoard.h"

class HashMap {
public:
    void insert(const ChessBoard& key, uint64_t value) {
        map[key] = value;
    }

    uint64_t get(const ChessBoard& key) const {
        auto it = map.find(key);
        if (it != map.end()) {
            return it->second;
        }
        throw std::runtime_error("ChessBoard not found");
    }

    bool contains(const ChessBoard &key) const{
        if (map.find(key) == map.end()) return false;
        return true;
    }

    size_t getSize() const {
        return map.size();
    }

private:
    std::unordered_map<ChessBoard, uint64_t, HashFunction> map;
};

uint64_t perft(uint64_t depth, ChessBoard& chessPosition){
    if (depth == 0) 
        return 1UL;
    std::vector<ChessMove> move_list = chessPosition.getMoveList();
    int n_moves = move_list.size();
    uint64_t nodes = 0;

    for (int i = 0; i < n_moves; i++) {
        chessPosition.makeMove(move_list[i]);
        if (!chessPosition.isIllegal())
            nodes += perft(depth - 1, chessPosition);
        chessPosition.undoMove(move_list[i]);
    }
    return nodes;
}

uint64_t perft(uint64_t depth, ChessBoard& chessPosition, HashMap &hashMap){
    if (depth == 0) 
        return 1UL;
    if(!hashMap.contains(chessPosition)){
        std::vector<ChessMove> move_list = chessPosition.getMoveList();
        int n_moves = move_list.size();
        uint64_t nodes = 0;

        for (int i = 0; i < n_moves; i++) {
            chessPosition.makeMove(move_list[i]);
            if (!chessPosition.isIllegal())
                nodes += perft(depth - 1, chessPosition, hashMap);
            chessPosition.undoMove(move_list[i]);
        }
        hashMap.insert(chessPosition, nodes);
    }        
    return hashMap.get(chessPosition);
}

int main(){
    ChessBoard cBoard;
    HashMap hashMap;
    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << perft(7, cBoard, hashMap) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    std::cout << "Numero di oggetti: " << hashMap.getSize() << std::endl;
    return 0;
}
