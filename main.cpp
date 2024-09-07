#include <bitset>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <math.h>
#include <limits>
#include <vector>
#include <cassert>

#include "src/ChessBoard.h"
#include "src/TranspositionTable.h"

#define INF std::numeric_limits<float>::infinity()
#define DEPTH 6

std::vector<int> moveListOrder(std::vector<ChessMove> &moveList){
    enum pieceType {white, black, pawns, knights, bishops, rooks, queens, kings};
    std::vector<int> order;

    for(int i = 0; i < moveList.size(); i ++) 
        if(moveList[i].getPiece() == pawns && moveList[i].isCapture()) order.emplace_back(i);
    for(int i = 0; i < moveList.size(); i ++) 
        if(moveList[i].getPiece() != pawns && moveList[i].isCapture()) order.emplace_back(i);
    for(int i = 0; i < moveList.size(); i ++) 
        if(!moveList[i].isCapture()) order.emplace_back(i);

    assert(order.size() == moveList.size());
    return order;
}

float quiescence(ChessBoard &pos, float alpha, float beta){
    int sign = (1 - 2*pos.getSideToMove());
    float standPat = sign * eval::material(pos.getBitBoards()); 
    if(standPat >= beta) alpha = standPat;
    else {
        if(standPat > alpha) alpha = standPat;
        std::vector<ChessMove> moveList = pos.getMoveList();
        std::vector<int> moveOrder = moveListOrder(moveList);
        bool exitCondition = false;

        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            ChessMove candidateMove = moveList[moveOrder[i]];

            if(candidateMove.isCapture()){
                pos.makeMove(candidateMove);

                if(!pos.isIllegal()){
                    float tmp = - quiescence(pos, -beta, -alpha);
                    if(tmp > alpha) alpha = tmp;
                    if(alpha >= beta) exitCondition = true;
                }

                pos.undoMove(candidateMove);
            }
        }
    }
    return alpha;
}

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &var, int depth, float alpha, float beta){
    if(depth == 0){
        if(map.doesContain(pos)) alpha = map.get(pos);
        else{ 
            if(pos.getSideToMove() == 0) alpha = quiescence(pos, alpha, beta);
            else alpha = quiescence(pos, -beta, -alpha);
            map.insert(pos, alpha);
        } 
    }
    else{
        std::vector<ChessMove> moveList = pos.getMoveList();
        std::vector<int> moveOrder = moveListOrder(moveList);
        bool exitCondition = false;

        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            ChessMove candidateMove = moveList[moveOrder[i]]; 
            pos.makeMove(candidateMove);
            if(!pos.isIllegal()){
                std::vector<ChessMove> bestContinuation;
                float alphaTmp = -alphaBeta(pos, map, bestContinuation, depth - 1, -beta, -alpha);

                if(alphaTmp >= beta){ 
                    alpha = alphaTmp;
                    exitCondition = true;
                }
                else if(alphaTmp > alpha){ 
                    alpha = alphaTmp;
                    bestContinuation.push_back(candidateMove);
                    var = bestContinuation;
                }
            }
            pos.undoMove(candidateMove);
        }
    }
    return alpha;
}


int main(){
    ChessBoard cBoard;
    TranspositionTable map;
    std::vector<ChessMove> variation;
    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float result = alphaBeta(cBoard, map, variation, DEPTH, -INF, INF);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    // for (int i = 0; i < result.seq.size(); i ++) std::cout << result.seq[i] << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    for (int i = 0; i < variation.size(); i ++) std::cout  << DEPTH - i << ") " << variation[i] << std::endl;
    std::cout << "Elements in table: " << map.getSize() << std::endl;
    return 0;
}
