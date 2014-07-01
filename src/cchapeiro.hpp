/*
 * cchapeiro.h
 *
 *  Created on: 01 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef CCHAPEIRO_HPP_
#define CCHAPEIRO_HPP_
#include <atomic>
#include <cinttypes>

//fixme remove
#define quiescenceMask (4)

#ifndef CACHE_ALIGN
#define CACHE_LINE  64
#ifdef _MSC_VER
#define CACHE_ALIGN __declspec(align(CACHE_LINE))
#else
#define CACHE_ALIGN __attribute__((aligned(CACHE_LINE)))
#endif
#endif
// #define cache_align CACHE_ALIGN
#define cache_align alignas(CACHE_LINE)

#include <string>

#define version 1
#define fixedShift
#include <iostream>
const std::string ndbgline = "info string Chapeiro Debug : ";

constexpr int maxRookBits = 12;
constexpr int maxBishopBits = 9;
constexpr int maxCheckAvoidanceShiftBits = 6;

constexpr int minPerftDepth = 1;
constexpr int maxPerftDepth = 100;

extern std::atomic<bool> interruption_requested;

extern bool debugcc;
//#define std::cout cout << "info string Chapeiro Debug : "
void debug(std::string);
#define NoPromotion (-1)
#define NO_NEXT_TIME_CONTROL (0)
#define INF (0x7FFFFFF0)
#define STARTING_DEPTH (2)
#define ELAPSED_TIME_FACTOR (2)

constexpr unsigned int MAX_BOARDS(128);

#ifdef STATS
#define statistics(x) x
#else
#define statistics(x)
#endif
namespace chapeiro{
	typedef uint64_t bitboard;
	typedef uint64_t zobrist;
}

#define ff_file(x) ((chapeiro::bitboard (0x0101010101010101ull)) << (x))
#define ff_rank(x) ((chapeiro::bitboard (0x00000000000000FFull)) << (x))
//#define ff_mDiag(x) (((chapeiro::bitboard (0x0102040810204080ull)) << (x-7)) | ((chapeiro::bitboard (0x0102040810204080ull)) >> (x-7)))

#define nf_file(x) (~file(x))
#define nf_rank(x) (~rank(x))

typedef uint64_t U64;
#ifdef _MSC_VER
#define formatBitboard "%#018I64Xull"
#else
#define formatBitboard ("%#018" PRIX64 "ull")
#endif

int communaticate();

#endif /* CCHAPEIRO_HPP_ */
