/*
 * uciProtocol.h
 *
 *  Created on: Aug 12, 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef UCIPROTOCOL_HPP_
#define UCIPROTOCOL_HPP_
#include <string>
#include <iostream>

enum UCI_command {
	UCI_uci       	= 0,
	UCI_debug       = 1,
	UCI_isready     = 2,
	UCI_newGame     = 3,
	UCI_position    = 4,
	UCI_quit        = 5,
	UCI_stop        = 6
};

const std::string UCI_commandFormat[] = {
	"uci",
	"debug [on | off]",
	"isready",
	"ucinewgame",
	"position [fen <fenstring> | startpos] moves <move1> .... <movei>",
	"quit"
};

constexpr char UCI_go_infinite[]  =  "infinite";
constexpr char UCI_go_depth[]     =     "depth";
constexpr char UCI_go_movetime[]  =  "movetime";
constexpr char UCI_go_movestogo[] = "movestogo";
constexpr char UCI_go_wtime[]     =     "wtime";
constexpr char UCI_go_btime[]     =     "btime";
constexpr char UCI_go_winc[]      =      "winc";
constexpr char UCI_go_binc[]      =      "binc";

void help(UCI_command com, std::ostream &out = std::cout);
bool initializeEngine();
int uci();

#endif /* UCIPROTOCOL_HPP_ */
