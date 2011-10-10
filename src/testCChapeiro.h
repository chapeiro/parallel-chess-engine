/*
 * testCChapeiro.h
 *
 *  Created on: Aug 11, 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef TESTCCHAPEIRO_H_
#define TESTCCHAPEIRO_H_
#include <string>
#include <iostream>
using namespace std;

const string perftformat = "perft fen <fen> results : D<depth1> <solution1>; D<depth2> <solution2>; ... ; D<depthN> <solutionN>;";
const string perftformatext = "perft [ <mindepth> , <maxdepth> ] fen <fen> results : D<depth1> <solution1>; D<depth2> <solution2>; ... ; D<depthN> <solutionN>;";
const string perftformatsm = "perft fen <fen> D<depth>";
const string pvtestformat = "pvtest fen <fen> result : M<depth>";
const string pvtestformatd = "pvtest fen <fen> result : D<depth>";
bool perftInterface(string input, time_t &totalTime, unsigned long long int &totalNodes, int mindepth = 0, int maxdepth = 1000);
bool pvtestInterface(string input, time_t &totalTime, unsigned long long int &totalNodes, unsigned long long int &totalLeafNodes);

#endif /* TESTCCHAPEIRO_H_ */
