/*
 * communicationCenter.cpp
 *
 *  Created on: 18 ��� 2011
 *      Author: Chrysogelos Periklis
 */
#include "Board.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "testCChapeiro.hpp"
#include "CommunicationProtocols/uciProtocol.hpp"
#include "BoardInterface/BoardInterface.hpp"
#include "Parallel/ThreadBoardInterface.hpp"
#include <cstdio>
using namespace std;
bool debugcc = false;

BoardInterface* board_interface = NULL;

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

int communaticate(){
	//if (debugcc) signFile();
	char modec[256];
	string mode;
	do {
		if (scanf(" %256[^\n]s\n", modec) == EOF) break;//getline(cin, mode);
		mode = modec;
		if (mode.find("uci")!=string::npos){
			return uci();
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
		} else if (mode.find("perft")!=string::npos){
			std::time_t totalTime (0);
			unsigned long long int totalNodes = 0;
			if (mode.compare("perft")==0){
				time_t runningTime (time(NULL));
				bool result = true;
				std::ifstream perftdb ("PerftDatabase.perft");
				if (!perftdb.good()){
					std::cout << ndbgline << "Open perfts' database failed!" << endl;
					continue;
				}
				std::string str;
				while (result && !perftdb.eof()){
					getline(perftdb, str);
					result = perftInterface(str, totalTime, totalNodes);
				}
				if (result){
					runningTime = time(NULL) - runningTime;
					cout << "Perft ended successfully. (Assertions : ";
#ifdef NDEBUG
					cout << "OFF )";
#else
					cout << " ON )";
#endif
					cout << endl;
					cout << "No errors have been found." << endl;
					cout << "Total leaf nodes :\t" << totalNodes << endl;
					cout << "Total search  time :\t" << totalTime << "\tsec" << endl;
					cout << "Total running time :\t" << runningTime << "\tsec" << endl;
					if (totalTime != 0) cout << "Leaf Nodes per Second (search): " << totalNodes / totalTime << endl;
					if (runningTime != 0) cout << "Leaf Nodes per Second (real): " << totalNodes / runningTime << endl;
				}
			} else {
				if (perftInterface(mode, totalTime, totalNodes)){
					cout << "Leaf Nodes : \t" << totalNodes << "\t(time : \t" << totalTime << "\tsec)" << endl;
				}
			}
		} else if (mode.find("pvtest")!=string::npos){
			time_t totalTime (0);
			unsigned long long int totalNodes = 0, totalLeafNodes = 0;
			if (mode.compare("pvtest")==0){
				time_t runningTime (time(NULL));
				bool result = true;
				ifstream pvtestsdb ("PVtestsDatabase.pvt");
				if (!pvtestsdb.good()){
					std::cout << ndbgline << "Open PV tests' database failed!" << endl;
					continue;
				}
				string str;
				while (result && !pvtestsdb.eof()){
					getline(pvtestsdb, str);
					result = pvtestInterface(str, totalTime, totalNodes, totalLeafNodes);
				}
				if (result){
					runningTime = time(NULL) - runningTime;
					cout << "PV's test ended successfully." << endl;
					cout << "No errors have been found." << endl;
					cout << "Total nodes :\t" << totalNodes << endl;
					cout << "Total leaf nodes :\t" << totalLeafNodes << endl;
					cout << "Total search  time :\t" << totalTime << "\tsec" << endl;
					cout << "Total running time :\t" << runningTime << "\tsec" << endl;
					if (totalTime != 0) cout << "Nodes per Second (search): " << totalNodes / totalTime << endl;
					if (runningTime != 0) cout << "Nodes per Second (real): " << totalNodes / runningTime << endl;
					if (totalTime != 0) cout << "Leaf Nodes per Second (search): " << totalLeafNodes / totalTime << endl;
					if (runningTime != 0) cout << "Leaf Nodes per Second (real): " << totalLeafNodes / runningTime << endl;
				}
			} else {
				if (pvtestInterface(mode, totalTime, totalNodes, totalLeafNodes)){
					cout << "Leaf Nodes : \t" << totalNodes << "\t(time : \t" << totalTime << "\tsec)" << endl;
				}
			}
		} else if (mode.compare("Is 64bit compiled") == 0){
			if (sizeof(int *) == 4){
				cout << "No" << endl;
			} else {
				cout << "Yes" << endl;
			}
		} else if (mode.compare("Is bitboard 64bit") == 0){
			if (sizeof(bitboard) == 8){
				cout << "Yes" << endl;
			} else {
				cout << "No." << endl;
				cout << "(Warning: strange results may occur with bitboards of size: " << (sizeof(bitboard)*8) << "bytes)" << endl;
			}
		}
	} while (true);
	return 1;
}

void debug(string a){
	std::cout << ndbgline << a << '\n';
}

int main(){
	board_interface = new ThreadBoardInterface(true);
	((ThreadBoardInterface* ) board_interface)->block();
	return 0;//communaticate();
}
