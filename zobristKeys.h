/*
 * zobristKeys.h
 *
 *  Created on: 29 Ìבס 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef ZOBRISTKEYS_H_
#define ZOBRISTKEYS_H_
#include "cchapeiro.h"

typedef chapeiro::zobrist key;

namespace zobrist {
	extern const key keys[64][12];
	extern const key enPassant[8];
	extern const key blackKey;
	extern const key White_King_Castling, White_Queen_Castling;
	extern const key Black_King_Castling, Black_Queen_Castling;
}

void generateZobristKeys();

#endif /* ZOBRISTKEYS_H_ */
