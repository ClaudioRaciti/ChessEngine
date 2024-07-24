#include <bitset>
#include <iostream>

#include "src/chessBoard.h"


int main(){
    Board cBoard;

    std::vector<CMove> moveList = cBoard.getMoveList();
    cBoard.makeMove(moveList[0]);
    return 0;
}
