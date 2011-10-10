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
	static const int MAT = 1048576; //1 << 20
};

const int inf = Value::MAT + 1;
#endif /* VALUES_H_ */
