#include <bitset>
#include <iostream>

#include "src/ChessBoard.h"

// uint64_t perft(int depth, ChessBoard& chessPosition){
//     std::vector<ChessMove> move_list = chessPosition.getMoveList();
//     int n_moves = move_list.size();
//     uint64_t nodes = 0;

//     if (depth == 0) 
//         return 1UL;

//     for (int i = 0; i < n_moves; i++) {
//         chessPosition.makeMove(move_list[i]);
//         if (!chessPosition.isIllegal())
//             nodes += perft(depth - 1, chessPosition);
//         chessPosition.unmakeLastMove();
//     }
//   return nodes;
// }

int main(){
    ChessBoard cBoard;

    std::vector<ChessMove> moveList = cBoard.getMoveList();
    return 0;
}
