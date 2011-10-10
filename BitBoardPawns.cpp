/*
 * BitBoardPawns.cpp
 *
 *  Created on: 21 ���� 2011
 *      Author: Chrysogelos Periklis
 */

#include "Board.h"

using namespace std;

U64 Board::movePawnsForward(int depth, const bitboard &notAllPieces){
	U64 moves = 0;
	//playing has the opposite color
	if (playing!=white){
		bitboard moved, to, tf;
		int toSq;
		key toggle;
		moved = (Pieces[PAWN] << 8) & notlastRank_w & notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to >> 8);
			toSq = square(to);
			toggle = zobrist::keys[toSq][PAWN] ^ zobrist::keys[toSq-8][PAWN];
			Pieces[PAWN] ^= tf;
			White_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			White_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		moved = ((((Pieces[PAWN]&pstartRank_w)<<8)&notAllPieces)<<8)&notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to >> 16);
			toSq = square(to);
			enPassant = to >> 8;
			toggle = zobrist::keys[toSq][PAWN] ^ zobrist::keys[toSq-8][PAWN] ^ zobrist::enPassant[7&square( enPassant )];
			Pieces[PAWN] ^= tf;
			White_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			White_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		enPassant = 0;
	} else {
		bitboard moved, to, tf;
		int toSq;
		key toggle;
		moved = (Pieces[PAWN+black] >> 8) & notlastRank_b & notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to << 8);
			toSq = square(to);
			toggle = zobrist::keys[toSq][PAWN+black] ^ zobrist::keys[toSq+8][PAWN+black];
			Pieces[PAWN+black] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN+black] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		moved = ((((Pieces[PAWN+black]&pstartRank_b)>>8)&notAllPieces)>>8)&notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to << 16);
			toSq = square(to);
			enPassant = to << 8;
			toggle = zobrist::keys[toSq][PAWN+black] ^ zobrist::keys[toSq+8][PAWN+black] ^ zobrist::enPassant[7&square( enPassant )];
			Pieces[PAWN+black] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN+black] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		enPassant = 0;
	}
	return moves;
}

U64 Board::movePawnsByAttOrProm(int depth, const bitboard &notAllPieces, const bitboard &tmpEnPassant){
	U64 moves = 0;
	if (playing!=white){
		bitboard attr, attl, prom;

		attr = notfile0 & lastRank_w & (Pieces[PAWN] << 7);
		//&enemy is tested later
		attl = notfile7 & lastRank_w & (Pieces[PAWN] << 9);
		for (int captured = QUEEN + black; captured >= 0 ; captured-=2){
			moves += makeWhitePawnsPAttack(attr, 7, depth, captured);
			moves += makeWhitePawnsPAttack(attl, 9, depth, captured);
		}

		attr = notfile0 & notlastRank_w & (Pieces[PAWN] << 7);
		//&enemy is tested later
		attl = notfile7 & notlastRank_w & (Pieces[PAWN] << 9);
		for (int i = QUEEN + black; i >= 0 ; i-=2){
			moves += makeWhitePawnsAttackbs(attr, attl, depth, i);
		}

		bitboard tf;
		int toSq;
		if ((attr & tmpEnPassant) != 0){
			toSq = square(tmpEnPassant);
			tf = (tmpEnPassant >> 7) | tmpEnPassant;
			bitboard cp = (tmpEnPassant >> 8);
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+black] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN] ^
					zobrist::keys[toSq-7][PAWN] ^
					zobrist::keys[toSq-8][PAWN+black];
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+black] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
		}
		if ((attl & tmpEnPassant) != 0){
			toSq = square(tmpEnPassant);
			tf = (tmpEnPassant >> 9) | tmpEnPassant;
			bitboard cp = (tmpEnPassant >> 8);
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+black] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN] ^
					zobrist::keys[toSq-9][PAWN] ^
					zobrist::keys[toSq-8][PAWN+black];
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+black] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
		}
		bitboard to, from;
		prom = (Pieces[PAWN] << 8) & lastRank_w & notAllPieces;
		while (prom!=0){
			to = prom & -prom;
			from = to >> 8;
			tf = to | from;
			toSq = square(to);
			Pieces[PAWN] ^= from;
			Pieces[QUEEN] |= to;
			zobr ^= zobrist::keys[toSq][QUEEN] ^ zobrist::keys[toSq-8][PAWN];
			White_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) {
				moves += perft(depth - 1);
				Pieces[QUEEN] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN];
				Pieces[ROOK] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK];
				moves += perft(depth - 1);
				Pieces[ROOK] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK];
				Pieces[BISHOP] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP];
				moves += perft(depth - 1);
				Pieces[BISHOP] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP];
				Pieces[KNIGHT] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT];
				moves += perft(depth - 1);
				Pieces[KNIGHT] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT];
			} else {
				Pieces[QUEEN] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN];
			}
			removeLastHistoryEntry();
			Pieces[PAWN] ^= from;
			zobr ^= zobrist::keys[toSq-8][PAWN];
			White_Pieces ^= tf;
			prom &= prom - 1;
		}
	} else {
		bitboard attr, attl, prom;
		attr = notfile0 & lastRank_b & (Pieces[PAWN+black] >> 9);
		//&enemy is tested later
		attl = notfile7 & lastRank_b & (Pieces[PAWN+black] >> 7);
		for (int captured = QUEEN; captured >= 0 ; captured-=2){
			moves += makeBlackPawnsPAttack(attr, 9, depth, captured);
			moves += makeBlackPawnsPAttack(attl, 7, depth, captured);
		}
		attr = notfile0 & notlastRank_b & (Pieces[PAWN+black] >> 9);
		attl = notfile7 & notlastRank_b & (Pieces[PAWN+black] >> 7);
		for (int i = QUEEN; i >= 0 ; i-=2){
			moves += makeBlackPawnsAttackbs(attr, attl, depth, i);
		}
		bitboard tf;
		int toSq;
		if ((attr & tmpEnPassant) != 0){
			toSq = square(tmpEnPassant);
			tf = (tmpEnPassant << 9) | tmpEnPassant;
			bitboard cp = (tmpEnPassant << 8);
			Pieces[PAWN+black] ^= tf;
			Pieces[PAWN] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN+black] ^
					zobrist::keys[toSq+9][PAWN+black] ^
					zobrist::keys[toSq+8][PAWN];
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN+black] ^= tf;
			Pieces[PAWN] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
		}
		if ((attl & tmpEnPassant) != 0){
			toSq = square(tmpEnPassant);
			tf = (tmpEnPassant << 7) | tmpEnPassant;
			bitboard cp = (tmpEnPassant << 8);
			Pieces[PAWN+black] ^= tf;
			Pieces[PAWN] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN+black] ^
					zobrist::keys[toSq+7][PAWN+black] ^
					zobrist::keys[toSq+8][PAWN];
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN+black] ^= tf;
			Pieces[PAWN] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
		}
		prom = (Pieces[PAWN+black] >> 8) & lastRank_b & notAllPieces;
		bitboard to, from;
		while (prom!=0){
			to = prom & -prom;
			from = to << 8;
			tf = to | from;
			toSq = square(to);
			Pieces[PAWN+black] ^= from;
			Pieces[QUEEN+black] |= to;
			zobr ^= zobrist::keys[toSq][QUEEN+black] ^ zobrist::keys[toSq+8][PAWN+black];
			Black_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) {
				moves += perft(depth - 1);
				Pieces[QUEEN+black] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN+black];
				Pieces[ROOK+black] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK+black];
				moves += perft(depth - 1);
				Pieces[ROOK+black] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK+black];
				Pieces[BISHOP+black] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP+black];
				moves += perft(depth - 1);
				Pieces[BISHOP+black] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP+black];
				Pieces[KNIGHT+black] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT+black];
				moves += perft(depth - 1);
				Pieces[KNIGHT+black] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT+black];
			} else {
				Pieces[QUEEN+black] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN+black];
			}
			removeLastHistoryEntry();
			Pieces[PAWN+black] ^= from;
			zobr ^= zobrist::keys[toSq+8][PAWN+black];
			Black_Pieces ^= tf;
			prom &= prom - 1;
		}
	}
	return moves;
}

inline U64 Board::makeWhitePawnsAttackbs(bitboard attackingR, bitboard attackingL, int depth, int attindex){
	return makeWhitePawnsAttack(attackingR, 7, depth, attindex) + makeWhitePawnsAttack(attackingL, 9, depth, attindex);
}

inline U64 Board::makeBlackPawnsAttackbs(bitboard attackingR, bitboard attackingL, int depth, int attindex){
	return makeBlackPawnsAttack(attackingR, 9, depth, attindex) + makeBlackPawnsAttack(attackingL, 7, depth, attindex);
}


U64 Board::makeWhitePawnsPAttack(bitboard attackingR, int sh, int depth, int captured){
	U64 moves = 0;
	int toSq;
	bitboard to, from, tf;
	attackingR = attackingR & Pieces[captured];
	while (attackingR != 0) {
		to = attackingR & -attackingR;
		from = to >> sh;
		tf = to | from;
		toSq = square(to);
		Pieces[captured] ^= to;
		zobr ^= zobrist::keys[toSq][captured];
		Pieces[PAWN] ^= from;
		Pieces[QUEEN] |= to;
		zobr ^= zobrist::keys[toSq][QUEEN] ^ zobrist::keys[toSq-sh][PAWN];
		White_Pieces ^= tf;
		Black_Pieces ^= to;
		addToHistory( zobr);
		if (validPosition()) {
			moves += perft(depth - 1);
			Pieces[QUEEN] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN];
			Pieces[ROOK] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK];
			moves += perft(depth - 1);
			Pieces[ROOK] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK];
			Pieces[BISHOP] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP];
			moves += perft(depth - 1);
			Pieces[BISHOP] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP];
			Pieces[KNIGHT] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT];
			moves += perft(depth - 1);
			Pieces[KNIGHT] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT];
		} else {
			Pieces[QUEEN] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN];
		}
		removeLastHistoryEntry();
		Pieces[captured] ^= to;
		zobr ^= zobrist::keys[toSq][captured];
		Pieces[PAWN] ^= from;
		zobr ^= zobrist::keys[toSq-sh][PAWN];
		White_Pieces ^= tf;
		Black_Pieces ^= to;
		attackingR &= attackingR - 1;
	}
	return moves;
}

U64 Board::makeBlackPawnsPAttack(bitboard attackingL, int sh, int depth, int captured){
	U64 moves = 0;
	int toSq;
	bitboard to, from, tf;
	attackingL = attackingL & Pieces[captured];
	while (attackingL != 0) {
		to = attackingL & -attackingL;
		from = to << sh;
		tf = to | from;
		toSq = square(to);
		Pieces[captured] ^= to;
		zobr ^= zobrist::keys[toSq][captured];
		Pieces[PAWN+black] ^= from;
		Pieces[QUEEN+black] |= to;
		zobr ^= zobrist::keys[toSq][QUEEN+black] ^ zobrist::keys[toSq+sh][PAWN+black];
		Black_Pieces ^= tf;
		White_Pieces ^= to;
		addToHistory( zobr);
		if (validPosition()) {
			moves += perft(depth - 1);
			Pieces[QUEEN+black] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN+black];
			Pieces[ROOK+black] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK+black];
			moves += perft(depth - 1);
			Pieces[ROOK+black] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK+black];
			Pieces[BISHOP+black] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP+black];
			moves += perft(depth - 1);
			Pieces[BISHOP+black] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP+black];
			Pieces[KNIGHT+black] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT+black];
			moves += perft(depth - 1);
			Pieces[KNIGHT+black] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT+black];
		} else {
			Pieces[QUEEN+black] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN+black];
		}
		removeLastHistoryEntry();
		Pieces[captured] ^= to;
		zobr ^= zobrist::keys[toSq][captured];
		Pieces[PAWN+black] ^= from;
		zobr ^= zobrist::keys[toSq+sh][PAWN+black];
		Black_Pieces ^= tf;
		White_Pieces ^= to;
		attackingL &= attackingL - 1;
	}
	return moves;
}

inline U64 Board::makeWhitePawnsAttack(bitboard attacking, int diff, int depth, int attindex){
	int toSq;
	U64 count = 0;
	bitboard to, from, tf, cattacks;
	cattacks = attacking & Pieces[attindex];
	while (cattacks!=0){
		to = cattacks & -cattacks;

		from = to >> diff;
		tf = to | from;
		toSq = square(to);

		Pieces[attindex] ^= to;
		Pieces[PAWN] ^= tf;
		zobr ^= zobrist::keys[toSq][PAWN] ^ zobrist::keys[toSq-diff][PAWN] ^ zobrist::keys[toSq][attindex];
		White_Pieces ^= tf;
		Black_Pieces ^= to;
		addToHistory(zobr);

		if (validPosition()) count += perft(depth-1);

		removeLastHistoryEntry();
		Pieces[attindex] ^= to;
		Pieces[PAWN] ^= tf;
		zobr ^= zobrist::keys[toSq][PAWN] ^ zobrist::keys[toSq-diff][PAWN] ^ zobrist::keys[toSq][attindex];
		White_Pieces ^= tf;
		Black_Pieces ^= to;

		cattacks &= cattacks-1;
	}
	return count;
}

inline U64 Board::makeBlackPawnsAttack(bitboard attacking, int diff, int depth, int attindex){
	int toSq;
	U64 count = 0;
	bitboard to, from, tf, cattacks;
	cattacks = attacking & Pieces[attindex];
	while (cattacks!=0){
		to = cattacks & -cattacks;
		from = to << diff;
		tf = to | from;
		toSq = square(to);
		Pieces[attindex] ^= to;
		zobr ^= zobrist::keys[toSq][attindex];
		Pieces[PAWN+black] ^= tf;
		zobr ^= zobrist::keys[toSq][PAWN+black] ^ zobrist::keys[toSq+diff][PAWN+black];
		White_Pieces ^= to;
		Black_Pieces ^= tf;
		addToHistory(zobr);

		if (validPosition()) count += perft(depth-1);

		removeLastHistoryEntry();
		Pieces[attindex] ^= to;
		zobr ^= zobrist::keys[toSq][attindex];
		Pieces[PAWN+black] ^= tf;
		zobr ^= zobrist::keys[toSq][PAWN+black] ^ zobrist::keys[toSq+diff][PAWN+black];
		White_Pieces ^= to;
		Black_Pieces ^= tf;
		cattacks &= cattacks-1;
	}
	return count;
}
