/*
 * cchapeiro.h
 *
 *  Created on: 01 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef CCHAPEIRO_H_
#define CCHAPEIRO_H_

#include <string>

#define version 1
#define fixedShift
#include <iostream>
const std::string ndbgline = "info string Chapeiro Debug : ";

const int maxRookBits = 12;
const int maxBishopBits = 9;
const int maxCheckAvoidanceShiftBits = 6;

const int minPerftDepth = 1;
const int maxPerftDepth = 100;

extern bool debugcc;
//#define std::cout cout << "info string Chapeiro Debug : "
void debug(std::string);
#define NoPromotion -1;

namespace chapeiro{
	typedef unsigned long long int bitboard;
	typedef unsigned long long int zobrist;
}

typedef unsigned long long int U64;
#ifdef _WIN32
#define formatBitboard "%#018I64Xull"
#else
#define formatBitboard "%#018llXull"
#endif

#endif /* CCHAPEIRO_H_ */
