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
#include "../MoveEncoding.hpp"
#include "../TimeManagement/TimeManager.hpp"

class BoardInterface{
    public:
        virtual ~BoardInterface() {}
        virtual void setBoard(std::string FEN)      = 0;
        virtual void makeMove(chapeiro::move)       = 0;
        virtual void printBoard()                   = 0;
        virtual void go(int depth, time_control tc) = 0;
        virtual void stop()                         = 0;
};
#endif /* BOARD INTERFACE_HPP_ */