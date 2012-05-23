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
#define NoPromotion (-1)
#define NO_NEXT_TIME_CONTROL (-1)
#define INF (0x7FFFFFF0)
#define STARTING_DEPTH (2)
#define ELAPSED_TIME_FACTOR (2)

#ifdef STATS
#define statistics(x) x
#else
#define statistics(x)
#endif
namespace chapeiro{
	typedef unsigned long long int bitboard;
	typedef unsigned long long int zobrist;
}

#define ff_file(x) ((chapeiro::bitboard (0x0101010101010101ull)) << (x))
#define ff_rank(x) ((chapeiro::bitboard (0x00000000000000FFull)) << (x))
//#define ff_mDiag(x) (((chapeiro::bitboard (0x0102040810204080ull)) << (x-7)) | ((chapeiro::bitboard (0x0102040810204080ull)) >> (x-7)))

#define nf_file(x) (~file(x))
#define nf_rank(x) (~rank(x))

typedef unsigned long long int U64;
#ifdef _MSC_VER
#define formatBitboard "%#018I64Xull"
#else
#define formatBitboard "%#018llXull"
#endif

#endif /* CCHAPEIRO_H_ */
