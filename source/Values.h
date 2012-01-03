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
};

const int inf = Value::MAT + 1;
#endif /* VALUES_H_ */
