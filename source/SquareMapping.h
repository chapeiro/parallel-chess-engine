/*
 * SquareMapping.h
 *
 *  Created on: 2 Αυγ 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef SQUAREMAPPING_H_
#define SQUAREMAPPING_H_
#include "MagicsAndPrecomputedData.h"
#include <intrin.h>

inline int index(const int &x, const int &y){
	return (y << 3) | (7 ^ x);
}

inline int file(const int &index){
	return 7 ^ (index & 7);
}

inline int rank(const int &index){
	return index >> 3;
}

#if defined _MSC_VER
    #include <intrin.h>
	inline void square(unsigned long int* index, bitboard bb){
		_BitScanForward64(index, bb);
	}
#else
#error asdasdasd
	inline void square(unsigned long* index, unsigned long Mask){
		unsigned long int abfs;
		_BitScanForward64(&abfs, b);
		return abfs;/**/
		//return magictable[(b*magic) >> 58];
	}
#endif

inline void printbb(bitboard bb){
	int i;
	for (int y = 7 ; y >= 0 ; --y){
		std::cout << ndbgline;
		for (int x = 0 ; x < 8 ; ++x){
			i = index(x, y);
			std::cout << ((bb >> i) & 1);
		}
		std::cout << '\n';
	}
}
#endif /* SQUAREMAPPING_H_ */
