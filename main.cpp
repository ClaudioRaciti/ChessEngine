#include <bitset>
#include <iostream>
#include <chrono>

#include "src/ChessBoard.h"

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

int main(){
    ChessBoard cBoard;
    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << perft(5, cBoard) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    return 0;
}
