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
#define DEPTH 10


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

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv, bool followPV,int depth, float alpha, float beta){
    
    Value val;
    if(map.getValue(pos, val) && val.depth >= depth){
        if(val.nodeType == pvNode) return val.score;
        if(val.nodeType == allNode && val.score < alpha) return val.score;
        if(val.nodeType == cutNode && val.score > beta) return val.score;
    }
    
    if(depth == 0){
        float score = quiescence(pos, alpha, beta);
        return score;
    }
    
    float bestScore = -INF;

    std::vector<ChessMove> moveList = pos.getMoveList();
    orderMoveList(moveList, pos.getSideToMove());
    if(followPV&&pv.size())std::partition(moveList.begin(),moveList.end(),[pv](ChessMove a){return a==pv.back();});

    int nodeType = allNode;

    for(auto move = moveList.begin(); move != moveList.end() && nodeType != cutNode; ++ move){
        pos.makeMove(*move);
        if(!pos.isIllegal()){
            std::vector<ChessMove> variation;
            if(followPV && pv.size()){variation.assign(pv.begin(), pv.end() - 1); followPV = false;}
            float score = -alphaBeta(pos, map, variation, followPV, depth - 1, -beta, -alpha);

            if(score > bestScore){
                bestScore = score;
                if(bestScore > alpha) {
                    alpha = bestScore;
                    nodeType = alpha >= beta ? cutNode : pvNode;
                    
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

float iterativeDeepening(ChessBoard &pos, int depth){
    TranspositionTable map(1<<20);
    std::vector<ChessMove> pv;
    float res = alphaBeta(pos, map, pv, true, 0, -INF, INF);
    for (int i = 1; i <= depth; i ++){
        float alpha = res - 0.5f;
        float beta = res + 0.5f;
        auto start = std::chrono::high_resolution_clock::now();
        res = alphaBeta(pos, map, pv, true, i, alpha, beta);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "at depth " << i << " evaluation is " << res << "\t(" << elapsed.count() << "s)" << std::endl;
    }
    for (int i = 0; i < pv.size(); i ++) std::cout << depth  - i << ") " << pv[i] << std::endl;
    //std::cout << map.getSize() <<std::endl;
    return res;
}

int main(){
    ChessBoard cBoard;

    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float result = iterativeDeepening(cBoard, DEPTH);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;

    return 0;
}
