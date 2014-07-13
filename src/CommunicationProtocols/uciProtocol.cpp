/*
 * uciInterface.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: Chrysogelos Periklis
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include <regex>
#include "uciProtocol.hpp"
#include "../TranspositionTable.hpp"
#include "../BoardInterface/BareBoardInterface.hpp"
#include "../BoardInterface/BoardInterface.hpp"

using namespace std;

void help(UCI_command com, ostream &out){
	out << '\"' << UCI_commandFormat[com] << "\" : " << endl;
	switch(com){
	case UCI_uci :
		out << "\tTell engine to use the uci (universal chess interface)" << endl;
		break;
	case UCI_debug :
		out << "\tSwitch the debug mode of the engine on and off." << endl;
		out << "\tIn debug mode the engine should send additional infos using the \"info string\" command,";
			out << " to help debugging." << endl;
		out << "\tThis mode should is switched off by default." << endl;
		out << "\t(this command can be sent at any time, even while the engine is thinking.)" << endl;
		break;
	case UCI_isready :
		out << "\tThis is used to synchronize the engine." << endl;
		out << "\tThis command can be used to wait for the engine to be ready again";
			out << " or to ping the engine to find out if it is still alive." << endl;
		out << "\tThis command will always be answered with \"readyok\"";
			out << " and can be sent also when the engine is calculating";
			out << " in which case the engine should also immediately answer";
			out << " without stopping the search." << endl;
		break;
	case UCI_position :
		out << "\tSets up the position described in <fenstring> on the internal board";
			out << " and plays the moves on the internal chess board." << endl;
		out << "\tIf the game was played from the start position the string \"startpos\" can be sent." << endl;
		break;
	case UCI_newGame :
		out << "\tThis is sent to the engine when the next search";
		out << "will be from a different game." << endl;
		break;
	case UCI_stop :
		out << "\tStops calculating as soon as possible." << endl;
		break;
	case UCI_quit :
		out << "\tQuits the program as soon as possible." << endl;
		break;
	}
}

bool initializeEngine(){
	Board::initialize();
	return true;
}

std::pair<int, time_control> getTimeControl(string t){
	stringstream inp(t);

	//default values
	time_control tc;
	tc.infinite_search  = false;
	tc.auto_search_time =  true;
	tc.movestogo = NO_NEXT_TIME_CONTROL;
	tc.whitep.remaining = static_cast<std::chrono::milliseconds>(40000);
	tc.blackp.remaining = static_cast<std::chrono::milliseconds>(40000);
	tc.whitep.increment = static_cast<std::chrono::milliseconds>(0);
	tc.blackp.increment = static_cast<std::chrono::milliseconds>(0);
	int depth = INF;

	//check arguments
	string tmp;
	unsigned long long int tmpi;
	while (!inp.eof() && !inp.bad()){
		inp.clear();
		inp >> tmp;
		//chech for booleans first...
		if (UCI_go_infinite         == tmp){
			tc.infinite_search = true;
		} else { //check for commands followed by int...
			inp >> tmpi;
			if (!inp.good()) continue;

			if (UCI_go_depth     == tmp){
				depth = (int) tmpi;
			} else if (UCI_go_movetime  == tmp){
				tc.auto_search_time = false;
				tc.search_time = std::chrono::milliseconds(tmpi);
			} else if (UCI_go_movestogo == tmp){
				tc.movestogo = (int) tmpi;
			} else if (UCI_go_wtime     == tmp){
				if (!tmpi) tmpi = 1000;
				tc.whitep.remaining = std::chrono::milliseconds(tmpi);
			} else if (UCI_go_btime     == tmp){
				if (!tmpi) tmpi = 1000;
				tc.blackp.remaining = std::chrono::milliseconds(tmpi);
			} else if (UCI_go_winc      == tmp){
				tc.whitep.increment = std::chrono::milliseconds(tmpi);
			} else if (UCI_go_binc      == tmp){
				tc.blackp.increment = std::chrono::milliseconds(tmpi);
			}
		}
	}

	// //TODO bool ponder = input.find("ponder") != string::npos;
	// // int mate = -1;
	// // if ((index = input.find("mate")) != string::npos) sscanf(input.substr(index).c_str(), "mate %i", &mate);
	// // int nodes = INF;
	// // if ((index = input.find("nodes")) != string::npos) sscanf(input.substr(index).c_str(), "nodes %i", &nodes);

	return make_pair(depth, tc);
}

int uci(){
	cout << "id name Chapeiro" << endl;
	cout << "id author Chrysogelos Periklis" << endl;
	//cout << "option name Hash type spin default 1 min 1 max 512" << endl;
	cout << "uciok" << endl;
	bool initialized = false;
	string input;
	do {
		getline(cin, input);
		board_interface->ui_garbage_collection();
		if (input.find("isready")!=string::npos){
			if (!initialized) initialized = initializeEngine();
			cout << "readyok" << endl;
			cout.flush();
		} else if (input.find("position")==0){
			if (!initialized){
				cerr << "\"isready\" should have been send before \"position\"" << endl;
				continue;
			}
			int b(-1);
			if (input.find("startpos")!=string::npos){
				board_interface->setBoard();
				sscanf(input.c_str(), "position startpos%n", &b);
			} else {
				int a(-1);
				sscanf(input.c_str(), "position fen %n%*s %*c %*s %*s %*d %*d%n", &a, &b);
				if (a < 0 || b < 0) {
					help(UCI_position);
					continue;
				}
				try {
					board_interface->setBoard(input.substr(a, b));
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
					board_interface->makeMove(chapeiro::convertUCImove(m));
				}
			}
			board_interface->printBoard();
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
			board_interface->newGame();
		} else if (input.find("go")==0){
			input.erase(0, 2);
			std::pair<int, time_control> p(getTimeControl(input));
			if (!board_interface->go(p.first, p.second)){
				cerr << "Unknown position or a search is still running" << endl;
				cerr << "Search can not start. " << endl;
				cerr << "Use stop or position command." << endl;
				help(UCI_stop, cerr);
				help(UCI_position, cerr);
			}
		} else if (input.find("stop")!=string::npos){
			board_interface->stop();
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

