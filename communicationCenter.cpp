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
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;
bool debugcc = true;
int hashSize = 1;

bool initializeEngine(){
	return true;
}

/**void signEndOfFile(string reason){
	char ct[20];
	time_t rawtime;
	time(&rawtime);;
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	std::cout << "################# DEBUG ENDED #################\n";
	std::cout << '(' << reason << ")\n" << ct << '\n';
}

void signFile(){
	char ct[20];
	time_t rawtime;
	time(&rawtime);
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	std::cout << "CChapeiro - Log File\n";
	std::cout << "\tDebug Details:\n";
	std::cout << "\t\tChapeiro Version : " << version << '\n';
	std::cout << "\t\tFile Created On  : " << ct << '\n';
	std::cout << "################# DEBUG INFO #################\n";
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
			Board* board = new Board();
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
					size_t moves;
					stringstream feninput;
					try {
						if (input.find("startpos")!=string::npos){
							board = new Board();//input.replace(0, 8, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
							moves = input.find("moves ");
						} else {
							input.erase(0, 4);
							moves = input.find("moves ");
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
							board = new Board(fenBoard, fenPlaying, fenCastling, fenEnPX, fenEnPY, fenHC, fenFM);
						}
						if (moves!=string::npos) {
							input.erase(0, moves+6);
							stringstream mv;
							mv << input;
							string m;
							do {
								mv >> m;
								board->make(convertUCImove(m));
							} while (!mv.eof());
						}
						board->print();
					} catch (int e) {
						if (e==1) {
							cout << "Invalid number of White Kings." << endl;
						} else if (e==2) {
							cout << "Invalid number of Black Kings." << endl;
						} else if (e==3) {
							cout << "Invalid En Passant square." << endl;
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
		} else if (mode.compare("xboard")==0){
			cout << "xboard protocol currently not supported" << endl;
		} else if (mode.find("exit")!=string::npos || mode.find("quit")!=string::npos){
			return 0;
		} else if (mode.find("precompute data")!=string::npos){
			std::cout << ndbgline << "Generating Data. Please Wait..." << endl;
			initializeEngine();
			precomputeData();
			std::cout << ndbgline << "Finished!" << endl;
			return 0;
		} else if (mode.find("generate zobrist")!=string::npos){
			std::cout << ndbgline << "Generating Zobrist's keys. Please Wait..." << endl;
			initializeEngine();
			generateZobristKeys();
			std::cout << ndbgline << "Finished!" << endl;
			return 0;
		} else if (mode.compare("perft")==0){
			FILE* perftdb;
			perftdb = fopen("PerftDatabase.perft", "r");
			if (perftdb==NULL){
				std::cout << ndbgline << "Open perfts' database failed!" << endl;
				continue;
			}
			char str[1024];
			int i = 0;
			unsigned long long int totalMoves=0;
			time_t totalTime=time(NULL);
			string end = "EndOfPerft";
			while (true) {
				++i;
				fscanf(perftdb, " %[^;]s", str);
				if (end.compare(str)==0) break;
				char fenBoard[71], fenEnP[3];
				char fenCastling[] = { '-', '-', '-', '-', '\0'};
				int fenHC, fenFM, fenEnPX, fenEnPY;
				char fenPlaying;
				stringstream feninput;
				sscanf(str, "%71s %c %4s %2s %d %d", fenBoard, &fenPlaying, fenCastling, fenEnP, &fenHC, &fenFM);
				if (fenEnP[0]=='-'){
					fenEnPX = -1;
					fenEnPY = -1;
				} else {
					fenEnPX = fenEnP[0]-'a';
					fenEnPY = fenEnP[1]-'1';
				}
				try {
					Board* board = new Board(fenBoard, fenPlaying, fenCastling, fenEnPX, fenEnPY, fenHC, fenFM);
					cout << board->getFEN() << endl;
					while (fgetc(perftdb)!='\n'){
						int depth, leafnodes;
						if (fscanf(perftdb, "D%d %d", &depth, &leafnodes)==0) break;
						if (depth < minPerftDepth || depth > maxPerftDepth) continue;
						board->dividedepth = -1;
						int bperft = board->perft(depth);
						totalMoves += bperft;
						if (bperft == leafnodes){
							cout << "\t depth : " << depth << "\tleaf nodes : " << bperft << endl;
						} else {
							cout << "\t depth : " << depth << "\tleaf nodes : " << leafnodes << "\tleaf nodes counted : " << bperft << "\tFAILED!" << endl;
#ifdef WIN32
							HANDLE child_input_read;
							HANDLE child_output_write;
							PROCESS_INFORMATION process_info;
							STARTUPINFO startup_info;
							SECURITY_ATTRIBUTES security_attributes;

							// Set the security attributes for the pipe handles created
							security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
							security_attributes.bInheritHandle = TRUE;
							security_attributes.lpSecurityDescriptor = NULL;
							CreatePipe(&(board->child_output_read), &child_output_write, &security_attributes, 0);
							CreatePipe(&child_input_read, &(board->child_input_write), &security_attributes, 0);

							// Create the child process
							ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
							ZeroMemory(&startup_info, sizeof(STARTUPINFO));
							startup_info.cb = sizeof(STARTUPINFO);
							startup_info.hStdInput = child_input_read;
							startup_info.hStdOutput = child_output_write;
							startup_info.hStdError = child_output_write;
							startup_info.dwFlags |= STARTF_USESTDHANDLES;
							CreateProcess("C:\\Users\\User\\Program Files\\Chess\\Sharper\\Sharper\\Sharper.exe", NULL, NULL, NULL, TRUE, 0, NULL, "C:\\Users\\User\\Program Files\\Chess\\Sharper\\Sharper", &startup_info, &process_info);
							DWORD bytes_written;
							CHAR buffer[4096];
							ReadFile( board->child_output_read, buffer, sizeof(buffer), &bytes_written, NULL);
#endif
							board->dividedepth = depth - 1;
							board->pre = "";
							board->perft(depth);
#ifdef WIN32
							WriteFile((board->child_input_write), "quit\n", strlen("quit\n"), &bytes_written, NULL);
#endif
							return -1;
						}
					}
					delete board;
				} catch (int e) {
					if (e==1) {
						cout << "Invalid number of White Kings." << endl;
					} else if (e==2) {
						cout << "Invalid number of Black Kings." << endl;
					}
					cout << "test " << i << "skipped!" << endl;
				}
			}
			totalTime = time(NULL) - totalTime;
			cout << "Perft ended." << endl;
			cout << "No errors have been found." << endl;
			cout << "Total leaf nodes : " << totalMoves << endl;
			cout << "Total time : " << totalTime << "sec" << endl;
			if (totalTime != 0) cout << "Leaf Nodes per Second : " << totalMoves / totalTime << endl;
		} else if (mode.compare("infperft")==0){
			Board* board = new Board();
			int depth = 0;
			U64 bperft;
			U64 totalMoves = 0;
			time_t st = time(NULL);
			time_t et;
			while (true){
				et = time(NULL);
				++depth;
				bperft = board->perft(depth);
				totalMoves += bperft;
				cout << "depth :\t" << depth << "\tleaf nodes :\t" << bperft;
				cout << "\ttotal nodes :\t" << totalMoves << "\ttotal time :\t" << time(NULL)-st;
				et = time(NULL) - et;
				if (et!=0) cout << "\tnode per second :\t" << (totalMoves/et);
				cout << endl;
			}
			delete board;
		}
	} while (true);
	return 1;
}

void debug(string a){
	std::cout << ndbgline << a << '\n';
}
