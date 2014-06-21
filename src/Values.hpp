/*
 * Values.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */
#include <cmath>

#ifndef VALUES_HPP_
#define VALUES_HPP_

namespace Value{
	constexpr int MAT = 1 << 19;
	extern const int piece[12];
	extern const int knightSq[64];
	extern const int kingSq[64];
	extern const int WpawnSq[64];
	extern const int BpawnSq[64];
	constexpr int rookOnOpenFile = 50;
};

#define mated(x) ((x) <= (100-Value::MAT))
#define isMat(x) ((abs(x)) >= (Value::MAT-100))
constexpr int inf = Value::MAT << 2;
#endif /* VALUES_HPP_ */
