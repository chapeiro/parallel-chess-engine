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
private:
    Board * board;

public:
    BareBoardInterface();
    ~BareBoardInterface();
    virtual void newGame();
    virtual void setBoard(std::string FEN=START_FEN);
    virtual bool makeMove(chapeiro::move m);
    virtual bool printBoard();
    virtual bool go(int depth, time_control tc);
    // virtual void perft();
    virtual void stop();
};

#endif /* BAREBOARDINTERFACE_HPP_ */