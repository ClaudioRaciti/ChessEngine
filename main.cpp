#include <bitset>
#include <iostream>

#include "src/ChessBoard.h"

uint64_t perft(uint64_t depth, ChessBoard& chessPosition){
    std::vector<ChessMove> move_list = chessPosition.getMoveList();
    int n_moves = move_list.size();
    uint64_t nodes = 0;

    if (depth == 0) 
        return 1UL;

    for (int i = 0; i < n_moves; i++) {
        chessPosition.makeMove(move_list[i]);
        if (!chessPosition.isIllegal())
            nodes += perft(depth - 1, chessPosition);
        chessPosition.undoMove(move_list[i]);
    }
  return nodes;
}

uint64_t perft(uint64_t depth, int &tmp, ChessBoard& chessPosition){
    std::vector<ChessMove> move_list = chessPosition.getMoveList();
    int n_moves = move_list.size();
    uint64_t nodes = 0;

    if (depth == 0) 
        return 1UL;

    for (int i = 0; i < n_moves; i++) {
        chessPosition.makeMove(move_list[i]);
        if (!chessPosition.isIllegal()){
            nodes += perft(depth - 1, chessPosition);
            if (move_list[i].isCapture()) tmp ++;    
        }
        chessPosition.undoMove(move_list[i]);
    }
  return nodes;
}

int main(){
    ChessBoard cBoard;
    int captures = 0;
    std::cout << cBoard  << std::endl;
    std::cout << perft(6, cBoard) << std::endl;
    
    return 0;
}
