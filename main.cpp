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
    bool unableToMove = true;
    float bestScore =  evadeChecks ? -INF : sign * eval::evaluate(pos.getBitBoards()); 

    if(bestScore >= beta) return bestScore;
    if(bestScore > alpha) alpha = bestScore;

    std::vector<ChessMove> moveList = pos.getMoveList();
    orderMoveList(moveList, pos.getSideToMove());

    for(auto move = moveList.begin(); move != moveList.end() && alpha < beta; ++ move){
        if(evadeChecks || move->isCapture()){
            pos.makeMove(*move);
            if(pos.isLegal()){
                unableToMove = false;
                float score = - quiescence(pos, -beta, -alpha);
                if(score > bestScore){
                bestScore = score;
                    if(score > alpha) alpha = bestScore;
                }
            }        
            pos.undoMove(*move);
        }
    }

    if(evadeChecks && unableToMove) bestScore = - CHECKMATE;

    return bestScore;
}

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv, bool followPV,int depth, float alpha, float beta)
{
    
    Value val;
    if(map.getValue(pos, val) && (val.depth >= depth || val.nodeType == endNode)){
        if(val.nodeType == pvNode) return val.score;
        if(val.nodeType == allNode && val.score < alpha) return val.score;
        if(val.nodeType == cutNode && val.score > beta) return val.score;
        if(val.nodeType == endNode){
            if(val.score < 0) return -(CHECKMATE + depth);
            else return 0.0f;
        }
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
    bool unableToMove = true;

    for(auto move = moveList.begin(); move != moveList.end() && nodeType != cutNode; ++ move){
        pos.makeMove(*move);
        if(!pos.isIllegal()){
            unableToMove = false;
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

    if(unableToMove) {
        bestScore = pos.isCheck() ? -(CHECKMATE + depth) : 0.0f;
        nodeType = endNode;
    }
    
    map.insert(pos, bestScore, depth, nodeType);
    return bestScore;
}

float iterativeDeepening(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv, int depth, int maxDepth, float alpha, float beta)
{
    auto start = std::chrono::high_resolution_clock::now();
    float res = alphaBeta(pos, map, pv, true, depth, alpha, beta);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "at ply " << depth << " evaluation is ";

    bool mating = false, gettingMated = false;
    if(res <= -CHECKMATE){
        gettingMated = true;
        int pliesToMate = pos.getSideToMove() + ((depth-int(res-CHECKMATE))/2) + ((depth-int(res-CHECKMATE))%2);
        std::cout << "mate in " << pliesToMate << "\t(" << elapsed.count() << "s)";

    }
    else if(res >= CHECKMATE){
        mating = true;
        int pliesToMate = pos.getSideToMove() + ((depth-int(res-CHECKMATE))/2) + ((depth-int(res-CHECKMATE))%2);
        std::cout << "mate in " << pliesToMate << "\t(" << elapsed.count() << "s)";
    }
    else std::cout << res << "\t(" << elapsed.count() << "s)";

    
    if(res < alpha){
        std::cout << " Fail low" << std::endl;
        return iterativeDeepening(pos, map, pv, depth, maxDepth, -INF, beta);
    }
    else if(res > beta){ 
        std::cout << " Fail high"<< std::endl;
        return iterativeDeepening(pos, map, pv, depth, maxDepth, alpha, INF);
    }
    else std::cout << std::endl;

    if (depth == maxDepth) return res;

    alpha = gettingMated ? res - 1.1f : res - 0.3f;
    beta = mating ? res + 1.1f : res + 0.3f;
    return iterativeDeepening(pos, map, pv, depth + 1, maxDepth, alpha, beta);
}

int main(){
    TranspositionTable map(1<<20);
    std::vector<ChessMove> pv;
    ChessBoard cBoard;

    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float result = iterativeDeepening(cBoard, map, pv, 0, DEPTH, -INF, INF);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    for(auto move = pv.begin(); move != pv.end(); ++move) std::cout << *move << std::endl;
    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;

    return 0;
}
