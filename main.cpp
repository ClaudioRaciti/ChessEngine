#include <bitset>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <math.h>
#include <limits>
#include <vector>
#include <cassert>
#include <algorithm>
#include <random>

#include "src/ChessBoard.h"
#include "src/TranspositionTable.h"
#include "src/notation.h"

#define INF std::numeric_limits<float>::infinity()
#define DEPTH 9


int staticExchangeEval(const ChessMove &move, int sideToMove){
    int res = eval::pieceVal(1 - sideToMove, move.getCaptured(), move.getEndSquare()) 
        - eval::pieceVal(sideToMove, move.getPiece(), move.getStartingSquare());

    return res + (move.isPromo() ? eval::pieceVal(sideToMove, move.getPromoPiece(), move.getEndSquare()) : 0);
}

int staticMoveEval(const ChessMove &move, int sideToMove){
    return eval::pieceVal(sideToMove, move.isPromo() ? move.getPromoPiece() : move.getPiece(), move.getEndSquare())
        - eval::pieceVal(sideToMove, move.getPiece(), move.getStartingSquare());
}


void orderMoveList(std::vector<ChessMove> &moveList, int sideToMove){
    std::vector<ChessMove>::iterator it = std::partition(
        moveList.begin(), moveList.end(), [](ChessMove a){return a.isCapture();}
    );

    std::sort(
        moveList.begin(), it, [sideToMove](ChessMove a, ChessMove b){
            return staticExchangeEval(a, sideToMove) > staticExchangeEval(b, sideToMove);
        }
    );

    std::sort(
        it, moveList.end(), [sideToMove](ChessMove a, ChessMove b){
            return staticMoveEval(a, sideToMove) > staticMoveEval(b, sideToMove);
        }
    );
}

float quiescence(ChessBoard &pos, float alpha, float beta){
    int sign = (1 - 2*pos.getSideToMove());
    bool evadeChecks = pos.isCheck();
    float bestScore =  evadeChecks ? -INF : sign * eval::evaluate(pos.getBitBoards()); 

    if(bestScore >= beta) return bestScore;
    if(bestScore > alpha) alpha = bestScore;

    std::vector<ChessMove> moveList = pos.getMoveList();
    orderMoveList(moveList, pos.getSideToMove());

    for(auto move = moveList.begin(); move != moveList.end() && alpha < beta; ++ move){
        if(evadeChecks || move->isCapture()){
            pos.makeMove(*move);
            if(pos.isLegal()){
                float score = - quiescence(pos, -beta, -alpha);
                if(score > bestScore){
                bestScore = score;
                    if(score > alpha) alpha = bestScore;
                }
            }        
            pos.undoMove(*move);
        }
    }

    return bestScore;
}

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv, int depth, float alpha, float beta){
    
    if(map.doesContain(pos) && map.getDepth(pos) >= depth){
        int nodeType = map.getNodeType(pos);
        float score = map.getScore(pos);
        if(nodeType == pvNode) return score;
        if(nodeType == allNode && score < alpha) return score;
        if(nodeType == cutNode && score > beta) return score;
    }
    
    if(depth == 0){
        float score = quiescence(pos, alpha, beta);
        map.insert(pos, score, depth, pvNode);
        return score;
    }
    
    float bestScore = -INF;
    std::vector<ChessMove> moveList = pos.getMoveList();
    orderMoveList(moveList, pos.getSideToMove());
    int nodeType = allNode;

    for(auto move = moveList.begin(); move != moveList.end() && nodeType != cutNode; ++ move){
        pos.makeMove(*move);
        if(!pos.isIllegal()){
            std::vector<ChessMove> variation;
            float score = -alphaBeta(pos, map, variation, depth - 1, -beta, -alpha);

            if(score > bestScore){
                bestScore = score;
                if(bestScore >= beta) nodeType = cutNode;
                else if(bestScore > alpha) {
                    alpha = bestScore;
                    nodeType = pvNode;
                    variation.push_back(*move);
                    pv = variation;
                }
            }
        }
        pos.undoMove(*move);
    }

    map.insert(pos, bestScore, depth, nodeType);

    return bestScore;
}

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv,
    bool followingPV, int depth, float alpha, float beta)
{
    if(depth == 0){
        if(map.doesContain(pos)) alpha = map.getScore(pos);
        else{ 
            alpha = quiescence(pos, alpha, beta);
            map.insert(pos, alpha, depth, pvNode);
        } 
    }
    else{
        std::vector<ChessMove> moveList = pos.getMoveList();
        orderMoveList(moveList, pos.getSideToMove());
        if(followingPV && pv.size() != 0)
            std::partition(moveList.begin(),moveList.end(),[pv](ChessMove a){return a == pv.back();});
        bool exitCondition = false;

        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            ChessMove candidateMove = moveList[i]; 
            pos.makeMove(candidateMove);
            if(!pos.isIllegal()){
                std::vector<ChessMove> bestContinuation;
                if(followingPV && pv.size() != 0) bestContinuation.assign(pv.begin(), pv.end() - 1);
                float alphaTmp = -alphaBeta(pos, map, bestContinuation, followingPV, depth - 1, -beta, -alpha);

                if(alphaTmp >= beta){ 
                    alpha = alphaTmp;
                    exitCondition = true;
                }
                else if(alphaTmp > alpha){ 
                    alpha = alphaTmp;
                    bestContinuation.push_back(candidateMove);
                    pv = bestContinuation;
                }
                followingPV = false;
            }
            pos.undoMove(candidateMove);
        }
    }
    return alpha;
}

float iterativeDeepening(ChessBoard &pos, int depth){
    TranspositionTable map(1<<20);
    std::vector<ChessMove> pv;
    float res;
    for (int i = 1; i <= depth; i ++){
        res = alphaBeta(pos, map, pv, true, i, -INF, INF);
    }
    for (int i = 0; i < pv.size(); i ++) std::cout << depth - i << ") " << pv[i] << std::endl;
    std::cout << map.getSize() <<std::endl;
    return res;
}

int main(){
    ChessBoard cBoard;
    std::vector<ChessMove> pv;
    TranspositionTable map(1<<20);

    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    //float result = iterativeDeepening(cBoard, DEPTH);
    float result = alphaBeta(cBoard, map, pv, DEPTH, -INF, INF);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    for (int i = 0; i < pv.size(); i ++) std::cout << DEPTH - i << ") " << pv[i] << std::endl;
    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;
    std::cout << "Transposition table size: " << map.getSize() << std::endl;

    return 0;
}
