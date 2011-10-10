/*
 * MagicsAndPrecomputedData.h
 *
 *  Created on: 17 Ιουν 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef MAGICSANDPRECOMPUTEDDATA_H_
#define MAGICSANDPRECOMPUTEDDATA_H_
#include "cchapeiro.h"

typedef chapeiro::bitboard bitboard;

#include <stdio.h>
void precomputeData();
void MagicGenerator(int maxBitsRook, int maxBitsBishop, FILE* out);

namespace filled{
 	extern const bitboard normal[64];//, a1h8[64], h1a8[64], vertical[64];
	extern const bitboard rank[8], file[8];
}

namespace notFilled{
	extern const bitboard normal[64];//, a1h8[64], h1a8[64], vertical[64];
	extern const bitboard rank[8], file[8];
}

extern const bitboard KnightMoves[64];
extern const bitboard KingMoves[64];
extern const bitboard magic;
extern const unsigned int magictable[64];
extern const bitboard RookMagic[64];
extern const bitboard BishopMagic[64];
extern const bitboard RookMask[64];
extern const bitboard BishopMask[64];
#ifndef fixedShift
extern const int RookShift[64];
extern const int BishopShift[64];
#endif
extern const bitboard RookAttacks[64][4096];
extern const bitboard BishopAttacks[64][512];
#endif /* MAGICSANDPRECOMPUTEDDATA_H_ */
