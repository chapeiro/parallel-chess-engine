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
#define KNIGHT 2
#define BISHOP 4
#define ROOK 6
#define QUEEN 8
#define KING 10
//#define all_w 12
//#define all_b 13
#define PIECESMAX 12
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
const int white = 0;
const int black = 1;
//castling precomputed
const bitboard WhiteKingSideC = 0x0000000000000001ull;
const bitboard WhiteQueenSideC = 0x0000000000000080ull;
const bitboard BlackKingSideC = 0x0100000000000000ull;
const bitboard BlackQueenSideC = 0x8000000000000000ull;
const bitboard castlingsmagic = 0x8100000000000081ull;
const bitboard allcastlingrights = 0x8100000000000081ull;
const bitboard castlingrights[2] = {0x0000000000000081ull, 0x8100000000000000ull};
const bitboard deactcastlingrights[2] = {0xFFFFFFFFFFFFFF7Eull, 0x7EFFFFFFFFFFFFFFull};

const bitboard WhiteKingSideCSpace = 0x0000000000000006ull;
const bitboard WhiteQueenSideCSpace = 0x0000000000000070ull;
const bitboard BlackKingSideCSpace = 0x0600000000000000ull;
const bitboard BlackQueenSideCSpace = 0x7000000000000000ull;

const bitboard WhiteKingSideDest = 0x0000000000000002ull;
const bitboard WhiteQueenSideDest = 0x0000000000000020ull;
const bitboard BlackKingSideDest = 0x0200000000000000ull;
const bitboard BlackQueenSideDest = 0x2000000000000000ull;

const bitboard WKSCPassing = 0x0000000000000004ull;
const bitboard WQSCPassing = 0x0000000000000010ull;
const bitboard BKSCPassing = 0x0400000000000000ull;
const bitboard BQSCPassing = 0x1000000000000000ull;

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
		int halfmoves;
		int fullmoves;
		int playing;
		bitboard castling;
		Zobrist history[256];
		int lastHistoryEntry;
		//for debug
		void printbb(bitboard);

		//
		inline void togglePlaying(){
			playing ^= 1;
			zobr ^= zobrist::blackKey;
		}
		void deactivateCastlingRights();
		inline void updatePieces(int, int);
		void capture(int);
		int movePawnsByAttOrProm(int, const bitboard &);
		int movePawnsForward(int, const bitboard &);
		void addToHistory(Zobrist);
		void removeLastHistoryEntry();
		bool notAttacked(const bitboard &);
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

		void continueCapturesPerft(const bitboard &, const int &, const int &,
				const int* , const bitboard* , const bitboard* ,
				bitboard* &, bitboard* &, const int &, int &);
		void continueNormalMPerft(const bitboard &, const int &,
				const int*, const bitboard*, const bitboard*,
				bitboard* &, const int &, int &);

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
