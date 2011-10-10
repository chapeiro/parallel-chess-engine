/*
 * Board.h
 *
 * General Board Public Interface
 *
 *  Created on: 29 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "cchapeiro.h"
#include "zobristKeys.h"
#include "MagicsAndPrecomputedData.h"
#include <string>

//square format definitions
#define index(x, y) (((y) << 3) + 7-(x))
#define file(index) (7-((index)&7))
#define rank(index) ((index) >> 3)

typedef chapeiro::bitboard bitboard;
typedef chapeiro::zobrist Zobrist;

#define MaskFrom 63
#define MaskTo 63
#define MaskEP 63
#define MaskKingCastling 1
#define MaskQueenCastling 1
#define MaskSpecial 3
#define MaskPiece 7
#define MaskPromotion 7
#define MaskCapturedPiece 7

#define ShiftTo 6
#define ShiftColor 12
#define ShiftEP 13
#define ShiftKingCastling 19
#define ShiftQueenCastling 20
#define ShiftSpecial 21
#define ShiftPiece 23
#define ShiftPromotion 26
#define ShiftCapturedPiece 29

//Pieces array & indexes definitions
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
//#define all_w 12
//#define all_b 13
#define PIECESMAX 12
#define BlackPiecesOffset 6
#define WRONG_PIECE -10

const bitboard lastRank_w = filled::rank[7];
const bitboard lastRank_b = filled::rank[0];
const bitboard notfile0 = notFilled::file[0];
const bitboard notfile7 = notFilled::file[7];
const bitboard notlastRank_w = ~filled::rank[7];
const bitboard notlastRank_b = ~filled::rank[0];
const bitboard pstartRank_w = filled::rank[1];
const bitboard pstartRank_b = filled::rank[6];
//color definitions
#define white true
#define black false

#define whiteInt 0
#define blackInt BlackPiecesOffset

#include <stdio.h>
void precomputeData();
void MagicGenerator(int maxBitsRook, int maxBitsBishop, FILE* out);

inline int square(bitboard &b){
	return magictable[(b*magic) >> 58];
}

class Board {
	private:
		//Self
		Zobrist zobr;
		bitboard Pieces[PIECESMAX];
		bitboard White_Pieces, Black_Pieces;
		bitboard enPassant;
		int playingInt;
		int halfmoves;
		int fullmoves;
		bool playing;
		bool WhiteKingSideCastling, WhiteQueenSideCastling;
		bool BlackKingSideCastling, BlackQueenSideCastling;
		Zobrist history[256];
		int lastHistoryEntry;
		//for debug
		void printbb(bitboard);

		//
		inline void togglePlaying(){
			playing = !playing;
			playingInt ^= BlackPiecesOffset;
			zobr ^= zobrist::blackKey;
		}
		void deactivateCastlingRights();
		inline void updatePieces(int, int);
		void capture(int);
		int movePawnsByAttOrProm(int, const bitboard &);
		int movePawnsForward(int, const bitboard &);
		void addToHistory(Zobrist);
		void removeLastHistoryEntry();
		bool validPosition();


		//protocols & definitions
		inline int getPieceIndex(char);
		inline bool isPieceBlack(int);
		static int getWhitePieceIndex(char);

		bitboard bishopAttacks(bitboard occ, const int &sq);
		bitboard queenAttacks(bitboard occ, const int &sq);
		bitboard rookAttacks(bitboard occ, const int &sq);

		bitboard bishopMovesTo(bitboard occ, const int &sq, bitboard &notFriendly);
		bitboard queenMovesTo(bitboard occ, const int &sq, bitboard &notFriendly);
		bitboard rookMovesTo(bitboard occ, const int &sq, bitboard &notFriendly);

		//pawns
		void toggleCaptureWhite(int, int, bitboard, bitboard, int, int);
		int makeWhitePawnsAttack(bitboard, int, int, int);
		int makeWhitePawnsAttackbs(bitboard, bitboard, int, int);
		void toggleCaptureBlack(int, int, bitboard, bitboard, int, int);
		int makeBlackPawnsAttack(bitboard, int, int, int);
		int makeBlackPawnsAttackbs(bitboard, bitboard, int, int);
		int makeWhitePawnsPAttack(bitboard, int, int, int);
		int makeBlackPawnsPAttack(bitboard, int, int, int);

	public:
		Board(char[], char, char[], int, int, int, int);
		void print();
		std::string getFEN();
		void make(move);
		int perft(int depth);
		static move getNullMove();
		static bool moveIsNull(move m);
		static short convertPromotion(char);
		static void newGame();
};

#endif /* BOARD_H_ */
