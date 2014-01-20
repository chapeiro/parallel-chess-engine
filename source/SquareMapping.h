/*
 * SquareMapping.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef SQUAREMAPPING_H_
#define SQUAREMAPPING_H_
#include "MagicsAndPrecomputedData.h"

inline int index(const int &x, const int &y){
	return (y << 3) | (7 ^ x);
}

inline int file(const int &index){
	return 7 ^ (index & 7);
}

inline int rank(const int &index){
	return index >> 3;
}

#ifdef _MSC_VER
    #include <intrin.h>
	inline void square(unsigned long int* index, bitboard bb){
		_BitScanForward64(index, bb);
	}
#else
	inline void square(unsigned long int* index, bitboard bb){
		*index = __builtin_ctzll (bb);/**/
		//return magictable[(b*magic) >> 58];
	}
#endif

inline void printbb(bitboard bb){
	int i;
	for (int y = 7 ; y >= 0 ; --y){
		std::cerr << ndbgline;
		for (int x = 0 ; x < 8 ; ++x){
			i = index(x, y);
			std::cerr << ((bb >> i) & 1);
		}
		std::cerr << '\n';
	}
}
#endif /* SQUAREMAPPING_H_ */
