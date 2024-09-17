#include "Evaluation.h"
#include "utils.h"



int pieceValue(int piece, int sideToMove, float gamePhase, int square)
{
    if(sideToMove == white) mirror(square);
    return (mgValue[piece - 2] + mgSquareTables[piece - 2][square]) * gamePhase 
        + (egValue[piece - 2] + egSquareTables[piece - 2][square]) * (1 - gamePhase);
}

float evaluate(const std::vector<uint64_t> &bitBoards, float gamePhase)
{
    int res = 0;
    for(int piece = pawns; piece <= kings; piece ++){
        res += countMaterial(piece, white, gamePhase, bitBoards[piece] & bitBoards[white]);
        res -= countMaterial(piece, black, gamePhase, bitBoards[piece] & bitBoards[black]);
    }
    return float(res) / 100;
}

int countMaterial(int piece, int sideToMove, float gamePhase, uint64_t bitBoard)
{
    int res = 0;

    if(bitBoard) do{
        res += pieceValue(piece, sideToMove, gamePhase, btw::bitScanForward(bitBoard));
    } while (bitBoard &= (bitBoard - 1));

    return res;
}
