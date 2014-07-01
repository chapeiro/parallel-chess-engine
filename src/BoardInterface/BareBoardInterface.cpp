/**
 * BareBoardInterface.cpp
 *
 *  Created on: 2014/06/21
 *      Author: Chrysogelos Periklis
 */

#include "BareBoardInterface.hpp"


BareBoardInterface::BareBoardInterface(): board(NULL), searchThread(NULL){
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
    if (board) {
        assert(searchThread == NULL);
        delete board;
    }
    board = Board::createBoard(FEN.c_str());
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
    assert(!(board && searchThread));
    if (board) searchThread = new thread(&Board::go, board, depth, tc);
    //board->go(depth, tc);
    return (board);
}

bool BareBoardInterface::search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, chapeiro::bitboard tf, int prom){
    assert(false);
    return false;
}

bool BareBoardInterface::collectNextScore(int &score, unsigned int thrd_id){
    assert(false);
    return false;
}


void BareBoardInterface::stop(){
    if (board && searchThread){ //board->stop();
        interruption_requested = true;
        searchThread->join();
        delete searchThread;
        searchThread = NULL;
    }
}

void BareBoardInterface::ui_garbage_collection(){}