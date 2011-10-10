/*
 * MoveEncoding.cpp
 *
 *  Created on: 2 Αυγ 2011
 *      Author: Chrysogelos Periklis
 */

#include "MoveEncoding.h"

using namespace std;

move getNullMove(){
	move n;
	n.fromX = -1;
	return n;
}

move convertUCImove(string s){
	if (s.compare("0000")==0){
		return getNullMove();
	}
	move ret;
	ret.fromX = s[0]-'a';
	ret.fromY = s[1]-'1';
	ret.toX = s[2]-'a';
	ret.toY = s[3]-'1';
	if (s.length()==5){
		ret.promoteTo = s[4]-'a'+'A';
	} else {
		ret.promoteTo = '-';
	}
	return ret;
}

bool moveIsNull(move m){
	return m.fromX==-1;
}
