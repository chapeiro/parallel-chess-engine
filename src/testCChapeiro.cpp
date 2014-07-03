/*
 * testCChapeiro.cpp
 *
 *  Created on: Aug 11, 2011
 *      Author: Chrysogelos Periklis
 */

#include "testCChapeiro.hpp"
#include <stdio.h>
#include "Board.hpp"
#include <string>
#include <ctime>
#ifdef DIVIDEPERFT
#include <windows.h>
#endif
using namespace std;

bool perftInterface(string input, time_t &totalTime, unsigned long long int &totalNodes){
	return perftInterface(input, totalTime, totalNodes, 0, 1000);
}

bool perftInterface(string input, time_t &totalTime, unsigned long long int &totalNodes, int mindepth, int maxdepth){
	int a (-1), b (-1), c(-1);
	if (input.find("results")!=string::npos){
		if (input.find("[") == string::npos) {
			sscanf(input.c_str(), "perft fen %n%*s %*c %*s %*s %*d %*d%n results : %n", &a, &b, &c);
		} else {
			sscanf(input.c_str(), "perft [ %d , %d ] fen %n%*s %*c %*s %*s %*d %*d%n results : %n", &mindepth, &maxdepth, &a, &b, &c);
		}
		if ( a < 0 || b < 0 || c < 0) {
			cout << "Input's format for Perft must be :" << endl;
			cout << '\t' << perftformat << endl;
			cout << "Or : " << endl;
			cout << '\t' << perftformatext << endl;
			cout << "Or : " << endl;
			cout << '\t' << perftformatsm << endl;
			cout << "(type \"perft\" to run automated perft)" << endl;
			return false;
		}
		Board* board = Board::createBoard(input.substr(a, b-a).c_str());
		cout << board->getFEN() << endl;
		input.erase(0, c);
		unsigned long long int result, solution;
#ifdef _WIN32
		while (sscanf(input.c_str(), " D%d %I64u;%n", &a, &solution, &c) >= 2){
#else
		while (sscanf(input.c_str(), " D%d %llu;%n", &a, &solution, &c) >= 2){
#endif
			input.erase(0, c);
			if (a < mindepth || a > maxdepth) continue;
			cout << "\tDepth : \t" << a << "\tLeaf Nodes : \t";
			cout << setw(20) << solution;
			cout.flush();
			board->dividedepth = -1;
			time_t st = time(NULL);
			result = board->perft(a);
			totalTime += time(NULL) - st;
			if (result == solution){
				totalNodes += result;
				cout << "\tOK\n";
			} else {
				cout << "\tCounted : \t" << result << "\tFailed!" << endl;
#ifdef DIVIDEPERFT
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
				board->pre = "";
#endif
				board->dividedepth = a - 1;
				board->perft(a);
#ifdef DIVIDEPERFT
				WriteFile((board->child_input_write), "quit\n", strlen("quit\n"), &bytes_written, NULL);
#endif
				cout.flush();
				delete board;
				return false;
			}
		}
		cout.flush();
		delete board;
		return true;
	} else {
		if ( sscanf(input.c_str(), "perft fen %n%*s %*c %*s %*s %*d %*d%n D%d", &a, &b, &c) < 1 || a < 0 || b < 0 || c < 0) {
			cout << "Input's format for Perft must be :" << endl;
			cout << '\t' << perftformat << endl;
			cout << "Or : " << endl;
			cout << '\t' << perftformatext << endl;
			cout << "Or : " << endl;
			cout << '\t' << perftformatsm << endl;
			cout << "(type \"perft\" to run automated perft)" << endl;
			return false;
		}
		Board* board = Board::createBoard(input.substr(a, b).c_str());
		unsigned long long int result = 0;
		board->dividedepth = -1;
		time_t st = time(NULL);
		result = board->perft(c);
		totalTime += time(NULL) - st;
		totalNodes += result;
		delete board;
		return true;
	}
}

bool pvtestInterface(string input, time_t &totalTime, unsigned long long int &totalNodes, unsigned long long int &totalLeafNodes){
	int a (-1), b (-1), c(-1);
	char playing;
	sscanf(input.c_str(), "pvtest fen %n%*s %c %*s %*s %*d %*d%n result : %n", &a, &playing, &b, &c);
	if ( a < 0 || b < 0 || c < 0) {
		cout << "Input's format for PV's test must be :" << endl;
		cout << '\t' << pvtestformat << endl;
		cout << "Or : " << endl;
		cout << '\t' << pvtestformatd << endl;
		cout << "(type \"pvtest\" to run automated pvtest)" << endl;
		return false;
	}
	Board* board = Board::createBoard(input.substr(a, b).c_str());
	cout << board->getFEN() << endl;
	input.erase(0, c);
	int depth = 0;
	char mode;
	sscanf(input.c_str(), "%c%d", &mode, &depth);
	int res;
	cout << "\tDepth :\t" << depth;
	cout.flush();
	time_t st = time(NULL);
	//FIXME depth
	res = board->test(depth);
	delete board;
	st = time(NULL) - st;
	totalTime += st;
	totalNodes += gstats.nodes;
	totalLeafNodes += gstats.horizonNodes;
	cout << "\tscore : \t" << res;
	if (st != 0){
		cout << "\t(leafNPS :\t" << gstats.horizonNodes/st << "\t, NPS :\t" << gstats.nodes/st << "\t)";
	}
	if (mode == 'D'){
		cout << endl;
		return true;
	} else if (mode == 'M'){
		if ((playing == 'w' && res >= Value::MAT) || (playing == 'b' && res <= -Value::MAT)){
			cout << "\tOK" << endl;
			return true;
		} else {
			cout << "\tFAILED!" << endl;
			return false;
		}
	} else {
		cout << "Wrong format for PV's test." << endl;
		return false;
	}
}

