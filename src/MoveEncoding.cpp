/*
 * MoveEncoding.cpp
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#include "MoveEncoding.hpp"

chapeiro::move getNullMove(){
	chapeiro::move n;
	n.fromX = -1;
	return n;
}

chapeiro::move chapeiro::convertUCImove(char m[6]){
	if (m[0]=='0'/**0000**/) return getNullMove();
	chapeiro::move ret;
	ret.fromX = m[0]-'a';
	ret.fromY = m[1]-'1';
	ret.toX = m[2]-'a';
	ret.toY = m[3]-'1';
	if (m[4]!='\0'){
		ret.promoteTo = m[4]-'a'+'A';
	} else {
		ret.promoteTo = '-';
	}
	return ret;
}

bool chapeiro::moveIsNull(chapeiro::move m){
	return m.fromX==-1;
}
