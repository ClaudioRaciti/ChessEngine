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

#define INF std::numeric_limits<float>::infinity()
#define DEPTH 9

enum pieceType {
    white, black, pawns, knights, bishops, rooks, queens, kings
};

enum algebraicNotation{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum moveType {
    quiet, doublePush, kingCastle, queenCastle, capture, enPassant, 
    knightPromo = 8, bishopPromo, rookPromo, queenPromo, 
    knightPromoCapture,  bishopPromoCapture, rookPromoCapture, queenPromoCapture
};


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
    float standPat = sign * eval::evaluate(pos.getBitBoards()); 
    if(standPat >= beta) alpha = standPat;
    else {
        if(standPat > alpha) alpha = standPat;
        std::vector<ChessMove> moveList = pos.getMoveList();
        orderMoveList(moveList, pos.getSideToMove());
        bool exitCondition = false;

        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            ChessMove candidateMove = moveList[i];

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
            alpha = quiescence(pos, alpha, beta);
            map.insert(pos, alpha);
        } 
    }
    else{
        std::vector<ChessMove> moveList = pos.getMoveList();
        orderMoveList(moveList, pos.getSideToMove());
        bool exitCondition = false;

        for(int i = 0; i < moveList.size() && !exitCondition; i ++){
            ChessMove candidateMove = moveList[i]; 
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

float alphaBeta(ChessBoard &pos, TranspositionTable &map, std::vector<ChessMove> &pv,
    bool followingPV, int depth, float alpha, float beta)
{
    if(depth == 0){
        if(map.doesContain(pos)) alpha = map.get(pos);
        else{ 
            alpha = quiescence(pos, alpha, beta);
            map.insert(pos, alpha);
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
    TranspositionTable map;
    std::vector<ChessMove> pv;
    float res;
    for (int i = 1; i <= depth; i ++){
        res = alphaBeta(pos, map, pv, true, i, -INF, INF);
    }
    for (int i = 0; i < pv.size(); i ++) std::cout << depth - i << ") " << pv[i] << std::endl;
    return res;
}

int main(){
    ChessBoard cBoard;
    std::vector<ChessMove> pv;
    TranspositionTable map;

    std::cout << cBoard  << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float result = iterativeDeepening(cBoard, DEPTH);
    // float result = alphaBeta(cBoard, map, pv, DEPTH, -INF, INF);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    for (int i = 0; i < pv.size(); i ++) std::cout << DEPTH - i << ") " << pv[i] << std::endl;
    std::cout << "Evaluation during search at depth " << DEPTH << " is " << result  << std::endl;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi" << std::endl;

    return 0;
}
