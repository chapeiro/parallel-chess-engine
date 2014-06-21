/*
 * MoveEncoding.h
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef MOVEENCODING_HPP_
#define MOVEENCODING_HPP_
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

#define NULL_MOVE (0)

#define TTMoveSize (18)
#define TTMove_EnPassantFlag (1 << (TTMoveSize-1))
#define TTMove_EnPassantPromFlag (1 << 5)
//EnPassantFlag|prom|--to--|-from-
//000000000000-|----|------|------
#define getTTMoveFormat(fromSq, toSq, prom_spec) ((fromSq) | ((toSq) << 6) | ((prom_spec) << 12))
#define getTTMove_From(move) ((move) & 0x3F)
#define getTTMove_To(move) (((move) >> 6) & 0x3F)
#define getTTMove_Prom_spec(move) (((move) >> 12) & 0x1F)
#define getTTMove_fromPS_P(ps) (ps & 0xF)

#endif /* MOVEENCODING_HPP_ */
