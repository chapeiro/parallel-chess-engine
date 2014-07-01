/**
 * BoardInterface.hpp
 *
 *  Created on: 2014/06/17
 *      Author: Chrysogelos Periklis
 */

#ifndef BOARDINTERFACE_HPP_
#define BOARDINTERFACE_HPP_
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include "../cchapeiro.hpp"
#include "../MoveEncoding.hpp"
#include "../TimeManagement/TimeManager.hpp"

#define START_FEN ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

class Board;

class BoardInterface{
    public:
        virtual ~BoardInterface() {}
        virtual void newGame()                                              = 0;
        virtual void setBoard(std::string FEN=START_FEN)                    = 0;
        virtual bool makeMove(chapeiro::move m)                             = 0;
        virtual bool printBoard()                                           = 0;
        virtual void ui_garbage_collection()                                = 0;
        virtual bool go(int depth, time_control tc)                         = 0;
        virtual bool search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, chapeiro::bitboard tf, int prom)     = 0;
        virtual bool collectNextScore(int &score, unsigned int thrd_id)     = 0;
        // virtual void perft()                                = 0;
        virtual void stop()                                 = 0;
};

extern BoardInterface* board_interface;
#endif /* BOARD INTERFACE_HPP_ */