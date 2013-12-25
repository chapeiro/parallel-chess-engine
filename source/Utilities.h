/*
 * Utilities.h
 *
 *  Created on: 3 Sep 2012
 *      Author: Periklis
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_
#include "cchapeiro.h"
#include <intrin.h>

inline int popCount(chapeiro::bitboard a){
	return _mm_popcnt_u64(a);
}



#endif /* UTILITIES_H_ */
