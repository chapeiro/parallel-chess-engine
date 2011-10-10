/*
 * BitBoardPawns.cpp
 *
 *  Created on: 21 Ιουν 2011
 *      Author: User
 */

#include "Board.h"

using namespace std;

int Board::movePawnsForward(int depth, const bitboard &notAllPieces){
	int moves = 0;
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
		moved = (Pieces[PAWN+BlackPiecesOffset] >> 8) & notlastRank_b & notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to << 8);
			toSq = square(to);
			toggle = zobrist::keys[toSq][PAWN+BlackPiecesOffset] ^ zobrist::keys[toSq+8][PAWN+BlackPiecesOffset];
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		moved = ((((Pieces[PAWN+BlackPiecesOffset]&pstartRank_b)>>8)&notAllPieces)>>8)&notAllPieces;
		while (moved!=0){
			to = moved & -moved;
			tf = to | (to << 16);
			toSq = square(to);
			enPassant = to << 8;
			toggle = zobrist::keys[toSq][PAWN+BlackPiecesOffset] ^ zobrist::keys[toSq+8][PAWN+BlackPiecesOffset] ^ zobrist::enPassant[7&square( enPassant )];
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth - 1);
			removeLastHistoryEntry();
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Black_Pieces ^= tf;
			zobr ^= toggle;
			moved &= moved - 1;
		}
		enPassant = 0;
	}
	return moves;
}

int Board::movePawnsByAttOrProm(int depth, const bitboard &notAllPieces){
	int moves = 0;
	if (playing==white){
		bitboard attr, attl, prom;

		attr = notfile0 & lastRank_w & (Pieces[PAWN] << 7);
		//&enemy is tested later
		attl = notfile7 & lastRank_w & (Pieces[PAWN] << 9);
		for (int captured = QUEEN + BlackPiecesOffset; captured >= BlackPiecesOffset ; --captured){
			moves += makeWhitePawnsPAttack(attr, 7, depth, captured);
			moves += makeWhitePawnsPAttack(attl, 9, depth, captured);
		}

		attr = notfile0 & notlastRank_w & (Pieces[PAWN] << 7);
		//&enemy is tested later
		attl = notfile7 & notlastRank_w & (Pieces[PAWN] << 9);
		for (int i = QUEEN + BlackPiecesOffset; i >= BlackPiecesOffset ; --i){
			moves += makeWhitePawnsAttackbs(attr, attl, depth, i);
		}

		bitboard tf;
		int toSq;
		if ((attr & enPassant) != 0){
			toSq = square(enPassant);
			tf = (enPassant >> 7) | enPassant;
			bitboard cp = (enPassant >> 8);
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+BlackPiecesOffset] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN] ^
					zobrist::keys[toSq-7][PAWN] ^
					zobrist::keys[toSq-8][PAWN+BlackPiecesOffset];
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+BlackPiecesOffset] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
		}
		if ((attl & enPassant) != 0){
			toSq = square(enPassant);
			tf = (enPassant >> 9) | enPassant;
			bitboard cp = (enPassant >> 8);
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+BlackPiecesOffset] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN] ^
					zobrist::keys[toSq-9][PAWN] ^
					zobrist::keys[toSq-8][PAWN+BlackPiecesOffset];
			zobr ^= toggle;
			White_Pieces ^= tf;
			Black_Pieces ^= cp;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN] ^= tf;
			Pieces[PAWN+BlackPiecesOffset] ^= cp;
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
			togglePlaying();
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
			togglePlaying();
			prom &= prom - 1;
		}
	} else {
		bitboard attr, attl, prom;
		attr = notfile0 & lastRank_b & (Pieces[PAWN+BlackPiecesOffset] >> 9);
		//&enemy is tested later
		attl = notfile7 & lastRank_b & (Pieces[PAWN+BlackPiecesOffset] >> 7);
		for (int captured = QUEEN; captured >= 0 ; --captured){
			moves += makeBlackPawnsPAttack(attr, 9, depth, captured);
			moves += makeBlackPawnsPAttack(attl, 7, depth, captured);
		}
		attr = notfile0 & notlastRank_b & (Pieces[PAWN+BlackPiecesOffset] >> 9);
		attl = notfile7 & notlastRank_b & (Pieces[PAWN+BlackPiecesOffset] >> 7);
		for (int i = QUEEN; i >= 0 ; --i){
			moves += makeBlackPawnsAttackbs(attr, attl, depth, i);
		}
		bitboard tf;
		int toSq;
		if ((attr & enPassant) != 0){
			toSq = square(enPassant);
			tf = (enPassant << 9) | enPassant;
			bitboard cp = (enPassant << 8);
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Pieces[PAWN] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN+BlackPiecesOffset] ^
					zobrist::keys[toSq+9][PAWN+BlackPiecesOffset] ^
					zobrist::keys[toSq+8][PAWN];
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Pieces[PAWN] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
		}
		if ((attl & enPassant) != 0){
			toSq = square(enPassant);
			tf = (enPassant << 7) | enPassant;
			bitboard cp = (enPassant << 8);
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Pieces[PAWN] ^= cp;
			Zobrist toggle = zobrist::keys[toSq][PAWN+BlackPiecesOffset] ^
					zobrist::keys[toSq+7][PAWN+BlackPiecesOffset] ^
					zobrist::keys[toSq+8][PAWN];
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			removeLastHistoryEntry();
			Pieces[PAWN+BlackPiecesOffset] ^= tf;
			Pieces[PAWN] ^= cp;
			zobr ^= toggle;
			White_Pieces ^= cp;
			Black_Pieces ^= tf;
		}
		prom = (Pieces[PAWN+BlackPiecesOffset] >> 8) & lastRank_b & notAllPieces;
		bitboard to, from;
		while (prom!=0){
			to = prom & -prom;
			from = to << 8;
			tf = to | from;
			toSq = square(to);
			Pieces[PAWN+BlackPiecesOffset] ^= from;
			Pieces[QUEEN+BlackPiecesOffset] |= to;
			zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset] ^ zobrist::keys[toSq+8][PAWN+BlackPiecesOffset];
			Black_Pieces ^= tf;
			togglePlaying();
			addToHistory(zobr);
			if (validPosition()) {
				moves += perft(depth - 1);
				Pieces[QUEEN+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset];
				Pieces[ROOK+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK+BlackPiecesOffset];
				moves += perft(depth - 1);
				Pieces[ROOK+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][ROOK+BlackPiecesOffset];
				Pieces[BISHOP+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP+BlackPiecesOffset];
				moves += perft(depth - 1);
				Pieces[BISHOP+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][BISHOP+BlackPiecesOffset];
				Pieces[KNIGHT+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT+BlackPiecesOffset];
				moves += perft(depth - 1);
				Pieces[KNIGHT+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][KNIGHT+BlackPiecesOffset];
			} else {
				Pieces[QUEEN+BlackPiecesOffset] ^= to;
				zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset];
			}
			removeLastHistoryEntry();
			Pieces[PAWN+BlackPiecesOffset] ^= from;
			zobr ^= zobrist::keys[toSq+8][PAWN+BlackPiecesOffset];
			Black_Pieces ^= tf;
			togglePlaying();
			prom &= prom - 1;
		}
	}
	return moves;
}

int Board::makeWhitePawnsAttackbs(bitboard attackingR, bitboard attackingL, int depth, int attindex){
	return makeWhitePawnsAttack(attackingR, 7, depth, attindex) + makeWhitePawnsAttack(attackingL, 9, depth, attindex);
}

int Board::makeBlackPawnsAttackbs(bitboard attackingR, bitboard attackingL, int depth, int attindex){
	return makeBlackPawnsAttack(attackingR, 9, depth, attindex) + makeBlackPawnsAttack(attackingL, 7, depth, attindex);
}


int Board::makeWhitePawnsPAttack(bitboard attackingR, int sh, int depth, int captured){
	int moves = 0;
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
		togglePlaying();
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
		togglePlaying();
		attackingR &= attackingR - 1;
	}
	return moves;
}

int Board::makeBlackPawnsPAttack(bitboard attackingL, int sh, int depth, int captured){
	int moves = 0;
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
		Pieces[PAWN+BlackPiecesOffset] ^= from;
		Pieces[QUEEN+BlackPiecesOffset] |= to;
		zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset] ^ zobrist::keys[toSq+sh][PAWN+BlackPiecesOffset];
		Black_Pieces ^= tf;
		White_Pieces ^= to;
		togglePlaying();
		addToHistory( zobr);
		if (validPosition()) {
			moves += perft(depth - 1);
			Pieces[QUEEN+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset];
			Pieces[ROOK+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK+BlackPiecesOffset];
			moves += perft(depth - 1);
			Pieces[ROOK+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][ROOK+BlackPiecesOffset];
			Pieces[BISHOP+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP+BlackPiecesOffset];
			moves += perft(depth - 1);
			Pieces[BISHOP+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][BISHOP+BlackPiecesOffset];
			Pieces[KNIGHT+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT+BlackPiecesOffset];
			moves += perft(depth - 1);
			Pieces[KNIGHT+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][KNIGHT+BlackPiecesOffset];
		} else {
			Pieces[QUEEN+BlackPiecesOffset] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN+BlackPiecesOffset];
		}
		removeLastHistoryEntry();
		Pieces[captured] ^= to;
		zobr ^= zobrist::keys[toSq][captured];
		Pieces[PAWN+BlackPiecesOffset] ^= from;
		zobr ^= zobrist::keys[toSq+sh][PAWN+BlackPiecesOffset];
		Black_Pieces ^= tf;
		White_Pieces ^= to;
		togglePlaying();
		attackingL &= attackingL - 1;
	}
	return moves;
}


void Board::toggleCaptureWhite(int captured, int piece, bitboard to, bitboard tf, int fromSq, int toSq){
	Pieces[captured] ^= to;
	zobr ^= zobrist::keys[toSq][captured];
	Pieces[piece] ^= tf;
	zobr ^= zobrist::keys[toSq][piece] ^ zobrist::keys[fromSq][piece];
	White_Pieces ^= tf;
	Black_Pieces ^= to;
	togglePlaying();
}

void Board::toggleCaptureBlack(int captured, int piece, bitboard to, bitboard tf, int fromSq, int toSq){
	Pieces[captured] ^= to;
	zobr ^= zobrist::keys[toSq][captured];
	Pieces[piece] ^= tf;
	zobr ^= zobrist::keys[toSq][piece] ^ zobrist::keys[fromSq][piece];
	White_Pieces ^= to;
	Black_Pieces ^= tf;
	togglePlaying();
}

int Board::makeWhitePawnsAttack(bitboard attacking, int diff, int depth, int attindex){
	int toSq, count = 0;
	bitboard to, from, tf, cattacks;
	cattacks = attacking & Pieces[attindex];
	while (cattacks!=0){
		to = cattacks & -cattacks;
		from = to >> diff;
		tf = to | from;
		toSq = square(to);
		toggleCaptureWhite(attindex, PAWN, to, tf, toSq-diff, toSq);
		addToHistory(zobr);

		if (validPosition()) count += perft(depth-1);

		removeLastHistoryEntry();
		toggleCaptureWhite(attindex, PAWN, to, tf, toSq-diff, toSq);
		cattacks &= cattacks-1;
	}
	return count;
}

int Board::makeBlackPawnsAttack(bitboard attacking, int diff, int depth, int attindex){
	int toSq, count = 0;
	bitboard to, from, tf, cattacks;
	cattacks = attacking & Pieces[attindex];
	while (cattacks!=0){
		to = cattacks & -cattacks;
		from = to << diff;
		tf = to | from;
		toSq = square(to);
		toggleCaptureBlack(attindex, PAWN+BlackPiecesOffset, to, tf, toSq+diff, toSq);
		addToHistory(zobr);

		if (validPosition()) count += perft(depth-1);

		removeLastHistoryEntry();
		toggleCaptureBlack(attindex, PAWN+BlackPiecesOffset, to, tf, toSq+diff, toSq);
		cattacks &= cattacks-1;
	}
	return count;
}
