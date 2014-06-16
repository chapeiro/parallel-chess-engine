/*
 * MagicsAndPrecomputedData.h
 *
 *  Created on: 17 Ιουν 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef MAGICSANDPRECOMPUTEDDATA_HPP_
#define MAGICSANDPRECOMPUTEDDATA_HPP_
#include "cchapeiro.hpp"
#include "precompd_PrecomputedData.hpp"
#include "precompd_magics.hpp"

typedef chapeiro::bitboard bitboard;

#include <stdio.h>
void precomputeData();
void MagicGenerator(int maxBitsRook, int maxBitsBishop, int maxCheckAvoidanceShiftBits, bitboard raysl[64][64], FILE* out);

// namespace filled{
//  	extern constexpr bitboard normal[64];//, a1h8[64], h1a8[64], vertical[64];
// 	extern constexpr bitboard rank[8], file[8], mainDiag[15], antiDiag[15];
// }

// namespace notFilled{
// 	extern constexpr bitboard normal[64];//, a1h8[64], h1a8[64], vertical[64];
// 	extern constexpr bitboard rank[8], file[8];
// }

// extern constexpr bitboard KnightMoves[64];
// extern constexpr bitboard KingMoves[64];
// extern constexpr bitboard magic;
// extern constexpr unsigned int magictable[64];
// extern constexpr bitboard RookMagic[64];
// extern constexpr bitboard BishopMagic[64];
// extern constexpr bitboard RookMask[64];
// extern constexpr bitboard BishopMask[64];
// #ifndef fixedShift
// extern constexpr int RookShift[64];
// extern constexpr int BishopShift[64];
// #endif
// extern constexpr bitboard RookAttacks[64][4096];
// extern constexpr bitboard BishopAttacks[64][512];
// extern constexpr bitboard rays[64][64];
// extern constexpr int direction[64][64];
// extern constexpr bitboard XRayOFCMask[64][64];
// extern constexpr bitboard XRayOFCMagic[64][64];
// extern constexpr bitboard XRayOFCChecker[64][64][64];
#endif /* MAGICSANDPRECOMPUTEDDATA_HPP_ */
