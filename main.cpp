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
#define DEPTH 8

float minimax(ChessBoard &pos, int depth, TranspositionTable &table){
    if (table.doesContain(pos)) return table.get(pos);

    enum Pieces {white, black};
    float res;

    if (depth == 0) res = eval::material(pos.getBitBoards());
    else {   
        std::vector<ChessMove> moveList = pos.getMoveList();
        if (pos.getSideToMove() == white){
            res = -std::numeric_limits<float>::infinity();
            for (int i = 0; i < moveList.size(); i ++){
                pos.makeMove(moveList[i]);
                if(!pos.isIllegal()){
                    float tmp = minimax(pos, depth - 1, table);
                    if (tmp > res) res = tmp;
                }
                pos.undoMove(moveList[i]);
            }
        }
        else{
            res = std::numeric_limits<float>::infinity();
            for (int i = 0; i < moveList.size(); i ++){
                pos.makeMove(moveList[i]);
                if(!pos.isIllegal()){
                    float tmp = minimax(pos, depth - 1, table);
                    if (tmp < res) res = tmp;
                }
                pos.undoMove(moveList[i]);
            }
        }
    }

    table.insert(pos, res);
    return res;
}

float alphaBetaMin(ChessBoard &, int, float, float);

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

float alphaBetaMax(ChessBoard &position, int depth, float alpha, float beta){
    if(depth == 0)alpha = eval::material(position.getBitBoards());
    else {
        std::vector<ChessMove> moveList = position.getMoveList();
        std::vector<int> moveOrder = moveListOrder(moveList);
        assert(moveList.size() == moveOrder.size());
        bool exitCondition = false;
        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            position.makeMove(moveList[moveOrder[i]]);
            if(!position.isIllegal()){
                float alphaTmp = alphaBetaMin(position, depth - 1, alpha, beta);
                if(alphaTmp >= beta){ 
                    alpha = alphaTmp;
                    exitCondition = true;
                }
                else if(alphaTmp > alpha){ 
                    alpha = alphaTmp;
                }
            }
            position.undoMove(moveList[moveOrder[i]]);
        }
    }
    return alpha;
}

float alphaBetaMin(ChessBoard &position, int depth, float alpha, float beta){
    
    if(depth == 0) beta = eval::material(position.getBitBoards());
    else {
        std::vector<ChessMove> moveList = position.getMoveList();
        std::vector<int> moveOrder = moveListOrder(moveList);
        bool exitCondition = false;
        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            position.makeMove(moveList[moveOrder[i]]);
            if(!position.isIllegal()){
                float betaTmp = alphaBetaMax(position, depth - 1, alpha, beta);
                if(alpha >= betaTmp){
                    beta = betaTmp;
                    exitCondition = true;
                }
                else if(betaTmp < beta){
                    beta = betaTmp;                    
                }
            }
            position.undoMove(moveList[moveOrder[i]]);
        }
    }
    return beta;
}

float alphaBeta(ChessBoard &position, int depth){
    float result;
    enum Pieces {white, black};
    if(position.getSideToMove() == white) result = alphaBetaMax(position, depth, -INF, INF);
    else result = alphaBetaMin(position, depth, -INF, INF);
    return result;
}

int main(){
    ChessBoard cBoard;
    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float result = alphaBeta(cBoard, DEPTH);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    // for (int i = 0; i < result.seq.size(); i ++) std::cout << result.seq[i] << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    // std::cout << "Elements in table: " << table.getSize() << std::endl;
    return 0;
}
