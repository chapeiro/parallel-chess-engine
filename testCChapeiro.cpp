/*
 * testCChapeiro.cpp
 *
 *  Created on: Aug 11, 2011
 *      Author: Chrysogelos Periklis
 */

#include "testCChapeiro.h"
#include <stdio.h>
#include "Board.h"
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;

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
		Board* board = Board::createBoard(input.substr(a, b).c_str());
		cout << board->getFEN() << endl;
		input.erase(0, c);
		unsigned long long int result, solution;
		while (sscanf(input.c_str(), " D%d %llu;%n", &a, &solution, &c) >= 2){
			input.erase(0, c);
			if (a < mindepth || a > maxdepth) continue;
			cout << "\tDepth : \t" << a << "\tLeaf Nodes : \t" << solution;
			cout.flush();
			board->dividedepth = -1;
			time_t st = time(NULL);
			result = board->perft(a);
			totalTime += time(NULL) - st;
			if (result == solution){
				totalNodes += result;
				cout << "\t\t\tOK\n";
			} else {
				cout << "\tCounted : \t" << result << "\tFailed!" << endl;
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
				board->dividedepth = a - 1;
				board->pre = "";
				board->perft(a);
#ifdef WIN32
				WriteFile((board->child_input_write), "quit\n", strlen("quit\n"), &bytes_written, NULL);
#endif
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
	st = time(NULL) - st;
	totalTime += st;
	totalNodes += board->nodes;
	totalLeafNodes += board->horizonNodes;
	cout << "\tscore : \t" << res;
	if (st != 0){
		cout << "\t(leafNPS :\t" << board->horizonNodes/st << "\t, NPS :\t" << board->nodes/st << "\t)";
	}
	if (mode == 'D'){
		cout << endl;
		return true;
	} else if (mode == 'M'){
		if (res >= Value::MAT){
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

