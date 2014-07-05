/*
 * SquareMapping.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef SQUAREMAPPING_HPP_
#define SQUAREMAPPING_HPP_
#include "MagicsAndPrecomputedData.hpp"
#include <assert.h>
#include <iostream>

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
	inline unsigned int square(bitboard bb){
		unsigned long int index;
		_BitScanForward64(&index, bb);
		return index;
	}
#else
	inline unsigned int square(unsigned long long int bb){
		return __builtin_ctzll(bb);
//		return magictable[(b*magic) >> 58];
	}
	inline unsigned int square(unsigned long int bb){
		return __builtin_ctzl(bb);
//		return magictable[(b*magic) >> 58];
	}
	inline unsigned int square(unsigned int bb){
		return __builtin_ctz(bb);
//		return magictable[(b*magic) >> 58];
	}
#endif

template<typename T>
inline T pop_lsb(T &bb){
	T tmp = bb & -bb;
	bb ^= tmp;
	return tmp;
}

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
#endif /* SQUAREMAPPING_HPP_ */
