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
	cout << "option name Hash type spin default 1 min 1 max 512" << endl;
	cout << "uciok" << endl;
	bool initialized = false;
	Board* board = 0;
	string input;
	do {
		getline(cin, input);
		if (input.compare("isready")==0){
			if (!initialized) initialized = initializeEngine();
			cout << "readyok" << endl;
		} else if (input.substr(0, 8).compare("position")==0){
			if (!initialized){
				cout << "\"isready\" should have been send before \"position\"" << endl;
				continue;
			}
			int b(-1);
			if (input.find("startpos")!=string::npos){
				board = new Board();
				sscanf(input.c_str(), "position startpos %n", &b);
			} else {
				int a (-1), c(-1);
				sscanf(input.c_str(), "position fen %n%*s %*c %*s %*s %*d %*d%n %n", &a, &c, &b);
				if (a < 0 || b < 0 || c < 0) {
					help(UCI_position);
					continue;
				}
				try {
					board = Board::createBoard(input.substr(a, c).c_str());
				} catch (exception* e) {
					cerr << e->what() << endl;
				}
			}
			input.erase(0, b);
			if (input.substr(0, 5).compare("moves")==0) {
				input.erase(0, 5);
				char m[6];
				while (sscanf(input.c_str(), " %5s%n", m, &b) >= 1){
					input.erase(0, b);
					board->make(chapeiro::convertUCImove(m));
				}
			}
			board->print();
		} else if (input.find("setoption name ")!=string::npos){
			input.erase(0, 15);
			if (input.find("Hash value ")!=string::npos){
				input.erase(0, 11);
				Board::hashSize = atoi(input.c_str());
			}
		} else if (input.find("debug ")!=string::npos){
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
		} else if (input.find("go ")!=string::npos){
			//input.erase(0, 3);
			bool infinite = input.find("infinite") != string::npos;
			bool ponder = input.find("ponder") != string::npos;
			int movetime = -1;
			sscanf(input.c_str(), "movetime %i", &movetime);
			int mate = -1;
			sscanf(input.c_str(), "mate %i", &mate);
			int nodes = -1;
			sscanf(input.c_str(), "nodes %i", &nodes);
			int depth = -1;
			sscanf(input.c_str(), "depth %i", &depth);
			int movestogo = -1;
			sscanf(input.c_str(), "movestogo %i", &movestogo);
			int winc = 0;
			sscanf(input.c_str(), "winc %i", &winc);
			int binc = 0;
			sscanf(input.c_str(), "binc %i", &binc);
			int wtime = 0;
			sscanf(input.c_str(), "wtime %i", &wtime);
			int btime = 0;
			sscanf(input.c_str(), "btime %i", &btime);
			//TODO support searchmoves ...
		}
	} while (input.compare("quit")!=0);
	//if (debugcc) signEndOfFile("CChapeiro Terminated");
	return 0;
}

