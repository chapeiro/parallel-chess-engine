/*
 * MoveEncoding.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef MOVEENCODING_H_
#define MOVEENCODING_H_
class Mask {
public :
	static const int from = 63;
	static const int to = 63;
	static const int ep = 63;
	static const int kingCastling = 1;
	static const int queenCastling = 1;
	static const int special = 3;
	static const int piece = 7;
	static const int promotion = 7;
	static const int capturedPiece = 7;
};

class Shift {
public :
	static const int from = 6;
	static const int to = 12;
	static const int ep = 13;
	static const int kingCastling = 19;
	static const int queenCastling = 20;
	static const int special = 21;
	static const int piece = 23;
	static const int promotion = 26;
	static const int capturedPiece = 29;
};

namespace chapeiro {
	struct move{
		int fromX, fromY, toX, toY;
		char promoteTo;
		bool operator ==(move m){
			if (fromX == m.fromX && fromY == m.fromY && toX==m.toX && toY==m.toY && promoteTo==m.promoteTo){
				return true;
			}
			return false;
		}
	};

	move convertUCImove(char m[6]);
	bool moveIsNull(move m);
}

#endif /* MOVEENCODING_H_ */
