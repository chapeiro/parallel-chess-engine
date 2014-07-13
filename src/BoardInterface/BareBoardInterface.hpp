/**
 * BareBoardInterface.hpp
 *
 *  Created on: 2014/06/21
 *      Author: Chrysogelos Periklis
 */

#ifndef BAREBOARDINTERFACE_HPP_
#define BAREBOARDINTERFACE_HPP_

#include "BoardInterface.hpp"
#include "../Board.hpp"

class BareBoardInterface: public BoardInterface{
protected:
    Board * board;

private:
    std::thread * searchThread;

public:
    BareBoardInterface();
    virtual ~BareBoardInterface();
    virtual void newGame();
    virtual void setBoard(std::string FEN=START_FEN);
    virtual bool makeMove(chapeiro::move m);
    virtual bool printBoard();
    virtual bool go(int depth, time_control tc);
    virtual bool search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, const internal_move &child);
    virtual bool collectNextScore(int &score, unsigned int thrd_id, int depth, internal_move &child);
    virtual bool collectNextScoreUB(int &score, unsigned int thrd_id, int depth, internal_move &child);
    virtual void ui_garbage_collection();
    virtual void updateWindows(int alpha, int beta, unsigned int thrd_id);
    // virtual void perft();
    virtual void stop();
};

#endif /* BAREBOARDINTERFACE_HPP_ */