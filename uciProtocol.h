/*
 * uciProtocol.h
 *
 *  Created on: Aug 12, 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef UCIPROTOCOL_H_
#define UCIPROTOCOL_H_
#include <string>

enum UCI_command {
	UCI_uci = 0,
	UCI_debug,
	UCI_isready,
	UCI_newGame,
	UCI_position,
	UCI_quit
};

const std::string UCI_commandFormat[] = {
	"uci",
	"debug [on | off]"
	"isready",
	"ucinewgame",
	"position [fen <fenstring> | startpos] moves <move1> .... <movei>",
	"quit"
};

void help(UCI_command com);
bool initializeEngine();
int uci();

#endif /* UCIPROTOCOL_H_ */
