/**
 * BareBoardInterface.cpp
 *
 *  Created on: 2014/06/21
 *      Author: Chrysogelos Periklis
 */

#include "BareBoardInterface.hpp"


BareBoardInterface::BareBoardInterface(): board(NULL){
}

BareBoardInterface::~BareBoardInterface(){
    if (board) delete board;
}

void BareBoardInterface::newGame(){
    if (board) delete board;
    ttNewGame();
    board = NULL;
}

void BareBoardInterface::setBoard(std::string FEN){
    if (!board) board = new Board();
    else        board = Board::createBoard(FEN.c_str());
}

bool BareBoardInterface::makeMove(chapeiro::move m){
    if (board) board->make(m);
    return (board);
}

bool BareBoardInterface::printBoard(){
    if (board) board->print();
    return (board);
}

bool BareBoardInterface::go(int depth, time_control tc){
    if (board) board->go(depth, tc);
    return (board);
}

void BareBoardInterface::stop(){
    if (board) board->stop();
}