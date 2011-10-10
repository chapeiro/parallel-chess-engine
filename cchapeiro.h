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

#define dbgstream cout
#define ndbgline "info string Chapeiro Debug : "

#define maxRookBits 12
#define maxBishopBits 9

const int minPerftDepth = 1;
const int maxPerftDepth = 100;

extern bool debugcc;
//#define dbgstream cout << "info string Chapeiro Debug : "
void debug(std::string);
#define NoPromotion -1;
struct move{
	short fromX, fromY, toX, toY;
	short promoteTo;
	bool operator ==(move m){
		if (fromX == m.fromX && fromY == m.fromY && toX==m.toX && toY==m.toY && promoteTo==m.promoteTo){
			return true;
		}
		return false;
	}
};

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
