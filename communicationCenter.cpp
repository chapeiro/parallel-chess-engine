/*
 * communicationCenter.cpp
 *
 *  Created on: 18 ��� 2011
 *      Author: Chrysogelos Periklis
 */
#include "Board.h"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <time.h>

typedef Board Board;
using namespace std;
bool debugcc = true;

int hashSize = 1;

bool initializeEngine(){
	return true;
}

move UCIconvertMove(string s){
	if (s.compare("0000")==0){
		return Board::getNullMove();
	}
	move ret;
	ret.fromX = s[0]-'a';
	ret.fromY = s[1]-'1';
	ret.toX = s[2]-'a';
	ret.toY = s[3]-'1';
	if (s.length()==5){
		ret.promoteTo = Board::convertPromotion(s[4]);
	} else {
		ret.promoteTo = NoPromotion;
	}
	return ret;
}

/**void signEndOfFile(string reason){
	char ct[20];
	time_t rawtime;
	time(&rawtime);;
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	dbgstream << "################# DEBUG ENDED #################\n";
	dbgstream << '(' << reason << ")\n" << ct << '\n';
}

void signFile(){
	char ct[20];
	time_t rawtime;
	time(&rawtime);
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	dbgstream << "CChapeiro - Log File\n";
	dbgstream << "\tDebug Details:\n";
	dbgstream << "\t\tChapeiro Version : " << version << '\n';
	dbgstream << "\t\tFile Created On  : " << ct << '\n';
	dbgstream << "################# DEBUG INFO #################\n";
}**/

int main(){
	//if (debugcc) signFile();
	string mode;
	do {
		getline(cin, mode);
		if (mode.compare("uci")==0){
			cout << "id name Chapeiro" << endl;
			cout << "id author Chrysogelos Periklis" << endl;
			cout << "option name Hash type spin default 1 min 1 max 512" << endl;
			cout << "uciok" << endl;
			bool initialized = false;
			string input;
			do {
				getline(cin, input);
				if (input.compare("isready")==0){
					if (!initialized) initialized = initializeEngine();
					cout << "readyok" << endl;
				} else if (input.find("position")!=string::npos && initialized){
					input.erase(0, 9);
					char fenBoard[71], fenEnP[3];
					char fenCastling[] = { '-', '-', '-', '-', '\0'};
					int fenHC, fenFM, fenEnPX, fenEnPY;
					char fenPlaying;
					stringstream feninput;
					if (input.find("startpos")!=string::npos){
						input.replace(0, 8, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
					} else {
						input.erase(0, 4);
					}
					size_t moves = input.find("moves ");
					if (moves!=string::npos){
						sscanf(input.c_str(), "%71s %c %4s %2s %d %d moves ", fenBoard, &fenPlaying, fenCastling, fenEnP, &fenHC, &fenFM);
					} else {
						sscanf(input.c_str(), "%71s %c %4s %2s %d %d\n", fenBoard, &fenPlaying, fenCastling, fenEnP, &fenHC, &fenFM);
					}
					if (fenEnP[0]=='-'){
						fenEnPX = -1;
						fenEnPY = -1;
					} else {
						fenEnPX = fenEnP[0]-'a';
						fenEnPY = fenEnP[1]-'1';
					}
					try {
						Board board (fenBoard, fenPlaying, fenCastling, fenEnPX, fenEnPY, fenHC, fenFM);
						if (moves!=string::npos) {
							input.erase(0, moves+6);
							stringstream mv;
							mv << input;
							string m;
							do {
								mv >> m;
								board.make(UCIconvertMove(m));
							} while (!mv.eof());
						}
						cout << "passed"<< endl;
						board.print();
					} catch (int e) {
						if (e==1) {
							cout << "Invalid number of White Kings." << endl;
						} else if (e==2) {
							cout << "Invalid number of Black Kings." << endl;
						}
					}
				} else if (input.find("setoption name ")!=string::npos){
					input.erase(0, 15);
					if (input.find("Hash value ")!=string::npos){
						input.erase(0, 11);
						hashSize = atoi(input.c_str());
					}
				} else if (input.find("debug ")!=string::npos){
					input.erase(0, 6);
					if (input.compare("on")==0){
						//if (!debugcc) signFile();
						debugcc = true;
					} else if (input.compare("off")==0){
						//if (debugcc) signEndOfFile("UCI request");
						debugcc = false;
					}
				} else if (input.find("ucinewgame")!=string::npos){
					Board::newGame();
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
		} else if (mode.compare("xboard")==0){
			cout << "xboard protocol currently not supported" << endl;
		} else if (mode.find("exit")!=string::npos || mode.find("quit")!=string::npos){
			return 0;
		} else if (mode.find("precompute data")!=string::npos){
			cout << "Generating Data. Please Wait..." << endl;
			initializeEngine();
			precomputeData();
			cout << "Finished!" << endl;
			return 0;
		} else if (mode.find("generate zobrist")!=string::npos){
			cout << "Generating Zobrist's keys. Please Wait..." << endl;
			initializeEngine();
			generateZobristKeys();
			cout << "Finished!" << endl;
			return 0;
		}
	} while (true);
}

void debug(string a){
	dbgstream << ndbgline << a << '\n';
}
