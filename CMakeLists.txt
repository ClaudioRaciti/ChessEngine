cmake_minimum_required(VERSION 3.5.0)
project(ChessEngine VERSION 0.1.0 LANGUAGES C CXX)

add_executable(
    ChessEngine main.cpp src/ChessBoard.cpp src/ChessMove.cpp src/utils.cpp src/LookupTables.cpp src/PosInfo.cpp
)

