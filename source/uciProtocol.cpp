/*
 * uciInterface.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: Chrysogelos Periklis
 */

#include <iostream>
#include "Board.h"
#include <string>
#include <stdlib.h>
#include "uciProtocol.h"
#include "TranspositionTable.h"

using namespace std;

void help(UCI_command com){
	cout << '\"' << UCI_commandFormat[com] << "\" : " << endl;
	switch(com){
	case UCI_uci :
		cout << "\tTell engine to use the uci (universal chess interface)" << endl;
		break;
	case UCI_debug :
		cout << "\tSwitch the debug mode of the engine on and off." << endl;
		cout << "\tIn debug mode the engine should send additional infos using the \"info string\" command,";
			cout << " to help debugging." << endl;
		cout << "\tThis mode should is switched off by default." << endl;
		cout << "\t(this command can be sent at any time, even while the engine is thinking.)" << endl;
		break;
	case UCI_isready :
		cout << "\tThis is used to synchronize the engine." << endl;
		cout << "\tThis command can be used to wait for the engine to be ready again";
			cout << " or to ping the engine to find out if it is still alive." << endl;
		cout << "\tThis command will always be answered with \"readyok\"";
			cout << " and can be sent also when the engine is calculating";
			cout << " in which case the engine should also immediately answer";
			cout << " without stopping the search." << endl;
		break;
	case UCI_position :
		cout << "\tSets up the position described in <fenstring> on the internal board";
			cout << " and plays the moves on the internal chess board." << endl;
		cout << "\tIf the game was played from the start position the string \"startpos\" can be sent." << endl;
		break;
	case UCI_newGame :
		cout << "\tthis is sent to the engine when the next search";
		cout << "will be from a different game." << endl;
		break;
	case UCI_quit :
		cout << "\tQuits the program as soon as possible." << endl;
		break;
	}
}

bool initializeEngine(){
	Board::initialize();
	return true;
}

int uci(){
	cout << "id name Chapeiro" << endl;
	cout << "id author Chrysogelos Periklis" << endl;
	//cout << "option name Hash type spin default 1 min 1 max 512" << endl;
	cout << "uciok" << endl;
	bool initialized = false;
	Board* board = NULL;
	string input;
	do {
		getline(cin, input);
		if (input.find("isready")!=string::npos){
			if (!initialized) initialized = initializeEngine();
			cout << "readyok" << endl;
		} else if (input.find("position")==0){
			if (!initialized){
				cerr << "\"isready\" should have been send before \"position\"" << endl;
				continue;
			}
			int b(-1);
			if (board) delete board;
			if (input.find("startpos")!=string::npos){
				board = new Board();
				sscanf(input.c_str(), "position startpos%n", &b);
			} else {
				int a (-1);
				sscanf(input.c_str(), "position fen %n%*s %*c %*s %*s %*d %*d%n", &a, &b);
				if (a < 0 || b < 0) {
					help(UCI_position);
					continue;
				}
				try {
					board = Board::createBoard(input.substr(a, b).c_str());
				} catch (exception* e) {
					cerr << e->what() << endl;
					continue;
				}
			}
			input.erase(0, b);
			if (input.length() > 6 && input.substr(1, 5).compare("moves")==0) {
				input.erase(0, 6);
				char m[6];
				while (sscanf(input.c_str(), " %5s%n", m, &b) >= 1){
					input.erase(0, b);
					board->make(chapeiro::convertUCImove(m));
				}
			}
			board->print();
		/**} else if (input.find("setoption name ")!=string::npos){
			input.erase(0, 15);
			if (input.find("Hash value ")!=string::npos){
				input.erase(0, 11);
				Board::hashSize = atoi(input.c_str());
			}**/
		} else if (input.find("debug ")==0){
			input.erase(0, 6);
			if (input.compare("on")==0){
				//if (!debugcc) signFile();
				debugcc = true;
			} else if (input.compare("off")==0){
				//if (debugcc) signEndOfFile("UCI request");
				debugcc = false;
			} else {
				help(UCI_debug);
			}
		} else if (input.find("ucinewgame")!=string::npos){
			delete board;
			ttNewGame();
			board = NULL;
		} else if (input.find("go")==0){
			input.erase(0, 3);
			if (!board){
				cerr << "Unknown position! Search can not start. Use position command." << endl;
				ttNewGame();
				board = new Board();
			}
			bool infinite = input.find("infinite") != string::npos;
			//TODO bool ponder = input.find("ponder") != string::npos;
			U64 movetime = INF;
			size_t index;
			if ((index = input.find("movetime")) != string::npos) sscanf(input.substr(index).c_str(), "movetime %llu", &movetime);
			int mate = -1;
			if ((index = input.find("mate")) != string::npos) sscanf(input.substr(index).c_str(), "mate %i", &mate);
			int nodes = INF;
			if ((index = input.find("nodes")) != string::npos) sscanf(input.substr(index).c_str(), "nodes %i", &nodes);
			int depth = INF;
			if ((index = input.find("depth")) != string::npos) sscanf(input.substr(index).c_str(), "depth %i", &depth);
			int movestogo = NO_NEXT_TIME_CONTROL;
			if ((index = input.find("movestogo")) != string::npos) sscanf(input.substr(index).c_str(), "movestogo %i", &movestogo);
			U64 winc = 0;
			if ((index = input.find("winc")) != string::npos) sscanf(input.substr(index).c_str(), "winc %llu", &winc);
			U64 binc = 0;
			if ((index = input.find("binc")) != string::npos) sscanf(input.substr(index).c_str(), "binc %llu", &binc);
			U64 wtime = 40000;
			if ((index = input.find("wtime")) != string::npos) sscanf(input.substr(index).c_str(), "wtime %llu", &wtime);
			U64 btime = 40000;
			if ((index = input.find("btime")) != string::npos) sscanf(input.substr(index).c_str(), "btime %llu", &btime);
			if (!wtime) wtime = 1000;
			if (!btime) btime = 1000;
			board->go(depth, wtime, btime, winc, binc, movestogo, movetime, infinite);
		} else if (input.find("stop")!=string::npos){
			if (board) board->stop();
#ifdef STATS
		} else if (input.compare("stats") == 0){
			U64 ttHits = (ttaccesses - ttmisses);
			U64 ttHitsNCO = (ttHits - hashHitCutOff);
			std::cerr << ndbgline << "Raw Stats Data : " << std::endl;
			std::cerr << ndbgline << "Beta Cut-Offs : \t\t" << betaCutOff << std::endl;
			std::cerr << ndbgline << "HashHits Cut-Offs : \t" << hashHitCutOff << std::endl;
			std::cerr << ndbgline << "TT misses : \t\t" << ttmisses << std::endl;
			std::cerr << ndbgline << "TT accesses : \t\t" << ttaccesses << std::endl;
			std::cerr << ndbgline << "Killer's Cut-Offs : \t\t" << cutOffByKillerMove << std::endl;
			std::cerr << ndbgline << "TT Type 1 Errors (min) (Hash Key Collisions ) : \t\t" << ttError_Type1_SameHashKey << std::endl;
			std::cerr << ndbgline << "Processed Data : " << std::endl;
			std::cerr << ndbgline << "TT hits : \t\t" << ttHits << "(" << (ttHits*100.0/ttaccesses) << "% of TT accesses)" << std::endl;
			std::cerr << ndbgline << "TT HashHits Cut-Offs : \t\t" << (hashHitCutOff*100.0/ttHits) << "% of TT hits" << std::endl;
			std::cerr << ndbgline << "TT Type 1 Errors (min) (Hash Key Collisions ) : \t\t" << (ttError_Type1_SameHashKey*100.0/ttHitsNCO) << "% of TT hits Which Do Not Produced Immediate Cut Off" << std::endl;
			std::cerr << ndbgline << "TT KillerMove Cut-Offs : \t\t" << (cutOffByKillerMove*100.0/ttHitsNCO) << "% of TT Hits Which Do Not Produced Immediate Cut Off" << std::endl;
#endif
		} else {
			cout << input << endl;
		}
	} while (input.find("quit")==string::npos);
	//if (debugcc) signEndOfFile("CChapeiro Terminated");
	return 0;
}

