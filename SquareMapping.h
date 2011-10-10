/*
 * SquareMapping.h
 *
 *  Created on: 2 Αυγ 2011
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

inline int square(const bitboard &b){
	return magictable[(b*magic) >> 58];
}

#endif /* SQUAREMAPPING_H_ */
