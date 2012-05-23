/*
 * Values.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef VALUES_H_
#define VALUES_H_

class Value{
public :
	static const int MAT = 1 << 20;
	static const int piece[12];
	static const int knightSq[64];
	static const int kingSq[64];
	static const int WpawnSq[64];
	static const int BpawnSq[64];
};

#define mated(x) ((x) <= (100-Value::MAT))
#define isMat(x) ((abs(x)) >= (Value::MAT-100))
const int inf = Value::MAT << 1;
#endif /* VALUES_H_ */
