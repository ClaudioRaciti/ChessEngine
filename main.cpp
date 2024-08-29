#include <bitset>
#include <iostream>
#include <chrono>
#include <unordered_map>

#include "src/ChessBoard.h"

uint64_t perft(uint64_t depth, ChessBoard& chessPosition, std::unordered_map<ChessBoard, uint64_t, HashFunction> db){
    if (depth == 0) 
        return 1UL;
    if(db.find(chessPosition) == db.end()){
        std::vector<ChessMove> move_list = chessPosition.getMoveList();
        int n_moves = move_list.size();
        uint64_t nodes = 0;

        for (int i = 0; i < n_moves; i++) {
            chessPosition.makeMove(move_list[i]);
            if (!chessPosition.isIllegal())
                nodes += perft(depth - 1, chessPosition, db);
            chessPosition.undoMove(move_list[i]);
        }
        db[chessPosition] = nodes;
    }
  return db[chessPosition];
}

int main(){
    ChessBoard cBoard;
    std::unordered_map<ChessBoard, uint64_t, HashFunction> transpositionTable;
    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << perft(6, cBoard, transpositionTable) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    return 0;
}
