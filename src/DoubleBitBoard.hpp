/*
 * DoubleBitBoard.h
 *
 *  Created on: 2 Sep 2012
 *      Author: Periklis
 */

#ifndef DOUBLEBITBOARD_H_
#define DOUBLEBITBOARD_H_

class DBitBoard{
	union {
		bitboard bb[2];
		__m128i db;
	} data;

	DBitBoard(bitboard bb1 = 0, bitboard bb2 = 0) : data({bb1, bb2}){}

	inline DBitBoard & operator=(const DBitBoard &a){
		data.db = a.data.db;
		return *this;
	}

	inline DBitBoard & operator^=(const DBitBoard &a){
		data.db = _mm_xor_si128(data.db, a.data.db);
		return *this;
	}

	inline DBitBoard & operator&=(const DBitBoard &a){
		data.db = _mm_and_si128(data.db, a.data.db);
		return *this;
	}

	inline DBitBoard & operator|=(const DBitBoard &a){
		data.db = _mm_or_si128(data.db, a.data.db);
		return *this;
	}

	inline DBitBoard & operator<<=(const DBitBoard &a){
		data.db = _mm_sll_epi64(data.db, a.data.db);
		return *this;
	}

	inline DBitBoard & operator<<=(const int &a){
		data.db = _mm_slli_epi64(data.db, a);
		return *this;
	}

	inline DBitBoard & operator>>=(const DBitBoard &a){
		data.db = _mm_srl_epi64(data.db, a.data.db);
		return *this;
	}

	inline DBitBoard & operator>>=(const int &a){
		data.db = _mm_srli_epi64(data.db, a);
		return *this;
	}

};

#endif /* DOUBLEBITBOARD_H_ */
