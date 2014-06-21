/*
 * Utilities.h
 *
 *  Created on: 3 Sep 2012
 *      Author: Periklis
 */

#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_
#include "cchapeiro.hpp"

#ifdef _MSC_VER
    #include <intrin.h>
inline int popCount(chapeiro::bitboard a){
	return _mm_popcnt_u64(a);
}
#else
inline int popCount(chapeiro::bitboard a){
	return __builtin_popcountll(a);
}
#endif



#endif /* UTILITIES_HPP_ */
