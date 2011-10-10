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
#include "SquareMapping.h"
#include "MoveEncoding.h"
#include <string>
//#define NDEBUG
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif

typedef chapeiro::bitboard bitboard;
typedef chapeiro::zobrist Zobrist;


const std::string PiecesName[] = {"Pawns", "Knights", "Bishops",
							 "Rooks", "Queens", "Kings"};

//Pieces array & indexes definitions
enum Piece {
	PAWN = 0,
	KNIGHT = 2,
	BISHOP = 4,
	ROOK = 6,
	QUEEN = 8,
	KING = 10,
	CPIECES = 14
};

const int PIECEMASK = 14;
const int LASTPIECE = 12;
const int PIECESMAX = 16;
const int WRONG_PIECE = -10;

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
const int colormask = 1;
enum SearchMode {
	PV,
	ZW,
	Perft
};

//castling precomputed
template<int color> class castlingc {
public :
	static const bitboard KingSide = (color == white) ? 0x0000000000000001ull : 0x0100000000000000ull;
	static const bitboard QueenSide = (color == white) ? 0x0000000000000080ull : 0x8000000000000000ull;
	static const bitboard KingSideSpace = (color == white) ? 0x0000000000000006ull : 0x0600000000000000ull;
	static const bitboard QueenSideSpace = (color == white) ? 0x0000000000000070ull : 0x7000000000000000ull;
	static const bitboard KSCPassing = (color == white) ? 0x0000000000000004ull : 0x0400000000000000ull;
	static const bitboard QSCPassing = (color == white) ? 0x0000000000000010ull : 0x1000000000000000ull;
	static const bitboard KSCKT = (color == white) ? 0x000000000000000Aull : 0x0A00000000000000ull;
	static const bitboard QSCKT = (color == white) ? 0x0000000000000028ull : 0x2800000000000000ull;
	static const bitboard KSCRT = (color == white) ? 0x0000000000000005ull : 0x0500000000000000ull;
	static const bitboard QSCRT = (color == white) ? 0x0000000000000090ull : 0x9000000000000000ull;
	static const bitboard KSCFT = (color == white) ? 0x000000000000000Full : 0x0F00000000000000ull;
	static const bitboard QSCFT = (color == white) ? 0x00000000000000B8ull : 0xB800000000000000ull;
	static const bitboard deactrights = (color==white) ? 0xFFFFFFFFFFFFFF7Eull : 0x7EFFFFFFFFFFFFFFull;
};
const bitboard castlingsmagic = 0x8100000000000081ull;
const bitboard allcastlingrights = 0x8100000000000081ull;
const bitboard castlingrights[2] = {0x0000000000000081ull, 0x8100000000000000ull};

class Board {
	private:
		//State
		bitboard Pieces[PIECESMAX];
		Zobrist zobr;
		bitboard enPassant;
		int halfmoves;
		int fullmoves;
		int playing;
		bitboard castling;
		//Memory
		Zobrist history[256];
		int lastHistoryEntry;
		//for Perft
		U64 horizonNodes;

	public:
		int dividedepth;
#ifdef WIN32
		HANDLE child_input_write;
		HANDLE child_output_read;
#endif
		std::string pre;

	public :
		//Construction
		Board(char fenBoard[] = (char*) "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", char fenPlaying = 'w', char fenCastling[] = (char*) "KQkq", int fenEnPX = -1, int fenEnPY = -1, int fenHC = 0, int fenFM = 1);

		void make(move m);

		//for debug
		std::string getFEN();
		void printbb(bitboard);
		void print();
		U64 perft(int depth);

	private :
		/**
		 * if p is a char representing a white piece,
		 * piece's index at Pieces array is returned.
		 * Else <code>WRONG_PIECE</code> is returned.
		 */
		int getWhitePieceIndex(char p);
		/**
		 * if p is a char representing a piece,
		 * piece's index at Pieces array is returned.
		 * else <code>WRONG_PIECE | white</code> or
		 * <code>WRONG_PIECE | black</code> is returned.
		 */
		int getPieceIndex(char p);

		void updatePieces(int sq, int ind);
		void capture(int to);

		void addToHistory(Zobrist position);
		void removeLastHistoryEntry();

		template<int color> void deactivateCastlingRights();
		void togglePlaying();

		template<int color> bool validPosition();
		template<int color> bool notAttacked(const bitboard target);

		bitboard bishopAttacks(bitboard occ, const int &sq);
		bitboard rookAttacks(bitboard occ, const int &sq);
		bitboard queenAttacks(bitboard occ, const int &sq);

		template<SearchMode mode, int color> int search(int alpha, int beta, int depth);
		template<SearchMode mode, int color> int searchDeeper(const int &alpha, const int &beta, const int &depth, const int &pvFound);
		template<int color> int quieSearch(int alpha, int beta);
};

inline int Board::getPieceIndex(char p){
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A') | black;
	return getWhitePieceIndex(p) | white;
}

inline void Board::updatePieces(int sq, int ind){
	Pieces[ind] ^= filled::normal[sq];
	zobr ^= zobrist::keys[sq][ind];
	Pieces[CPIECES | ind] ^= filled::normal[sq];
}

inline void Board::addToHistory(Zobrist position){
	history[++lastHistoryEntry] = position;
}

inline void Board::removeLastHistoryEntry(){
	--lastHistoryEntry;
}

template<int color> inline void Board::deactivateCastlingRights(){
	bitboard oldc = castling;
	castling &= castlingc<color>::deactrights;
	zobr ^= zobrist::castling[((castling^oldc)*castlingsmagic)>>59];
}

inline void Board::togglePlaying(){
	playing ^= 1;
	zobr ^= zobrist::blackKey;
}

/**
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<int color> bool Board::notAttacked(const bitboard target){
	assert((target & (target-1))==0);
	if (color == black){
		if ( ( (Pieces[PAWN | black] >> 7) & target & notfile7) != 0) return false;
		if ( ( (Pieces[PAWN | black] >> 9) & target & notfile0) != 0) return false;
	} else {
		if ( ( (Pieces[PAWN | white] << 7) & target & notfile0) != 0) return false;
		if ( ( (Pieces[PAWN | white] << 9) & target & notfile7) != 0) return false;
	}
	int sq = square(target);
	if ((Pieces[KNIGHT | color] & KnightMoves[sq])!=0) return false;
	if ((Pieces[KING | color] & KingMoves[sq])!=0) return false;
	bitboard occ = Pieces[CPIECES | white] | Pieces[CPIECES | black];
	bitboard att = rookAttacks(occ, sq);
	if ((att & Pieces[CPIECES | color]) != 0 && ((att & Pieces[ROOK | color]) != 0 || (att & Pieces[QUEEN | color]) != 0)) return false;
	att = bishopAttacks(occ, sq);
	if ((att & Pieces[CPIECES | color]) != 0 && ((att & Pieces[BISHOP | color]) != 0 || (att & Pieces[QUEEN | color]) != 0)) return false;
	return true;
}

template<int color> inline bool Board::validPosition() {
	return notAttacked<color^1>(Pieces[KING | color]);
}

template<SearchMode mode, int color> inline int Board::searchDeeper(const int &alpha, const int &beta, const int &depth, const int &pvFound){
	if (mode == PV){
		if (pvFound) {
			int score = -search<ZW, color>(-1-alpha, -alpha, depth - 1);
			if ( score > alpha ) score = -search<PV, color>(-beta, -alpha, depth - 1);
			return score;
		} else {
			return -search<PV, color>(-beta, -alpha, depth - 1);
		}
	} else if (mode == ZW){
		return -search<ZW, color>(-beta, -alpha, depth - 1);
	} else {
		return -search<Perft, color>(-beta, -alpha, depth - 1);
		/**if (pvFound) {
			int score = -search<ZW, color>(-1-alpha, -alpha, depth - 1);
			if ( score > alpha ) score = -search<Perft, color>(-beta, -alpha, depth - 1);
			return score;
		} else {
			return -search<Perft, color>(-beta, -alpha, depth - 1);
		}**/
	}
}

template<SearchMode mode, int color> int Board::search(int alpha, int beta, int depth){
	if (depth == 0) {
		if (mode == Perft) {
			if (dividedepth==0) {
				int oldplaying = playing;
				playing = color;
				std::cout << pre << getFEN() << '\n';
				playing = oldplaying;
			}
			++horizonNodes;
			return beta;
		}
		return quieSearch<color>(mode==ZW?beta-1:alpha, beta);
	}
	U64 stHorNodes (horizonNodes);
	int score;
	bool pvFound = false;

	int oldhm (halfmoves);
	if (color==black) ++fullmoves;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square(enPassant)];
	zobr ^= zobrist::blackKey;
	bitboard tmpEnPassant (enPassant);

	halfmoves = 0;
	enPassant = bitboard(0);

	bitboard attacking[2], attc, to, from, tf, toSq;

	if (color==white){
		attacking[0] = notfile0 & lastRank_w & (Pieces[PAWN | color] << 7);
		attacking[1] = notfile7 & lastRank_w & (Pieces[PAWN | color] << 9);
	} else {
		attacking[0] = notfile0 & lastRank_b & (Pieces[PAWN | color] >> 9);
		attacking[1] = notfile7 & lastRank_b & (Pieces[PAWN | color] >> 7);
	}
	for (int captured = QUEEN | (color^1); captured >= 0 ; captured-=2){
		for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
			attc = attacking[at] & Pieces[captured];
			while (attc!=0){
				to = attc & -attc;
				if (color == white){
					from = to >> diff;
				} else {
					from = to << -diff;
				}
				tf = to | from;
				toSq = square(to);
				Pieces[captured] ^= to;
				Pieces[PAWN | color] ^= from;
				Pieces[QUEEN | color] ^= to;
				zobr ^= zobrist::keys[toSq][captured];
				zobr ^= zobrist::keys[toSq][QUEEN | color];
				zobr ^= zobrist::keys[toSq-diff][PAWN | color];
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color^1)] ^= to;
				addToHistory(zobr);
				if (validPosition<color>()) {
					for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						Pieces[prom] ^= to;
						zobr ^= zobrist::keys[toSq][prom];
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							zobr ^= zobrist::keys[toSq][captured];
							zobr ^= zobrist::keys[toSq-diff][PAWN | color];
							Pieces[PAWN | color] ^= from;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color^1)] ^= to;
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
							if (color==black) --fullmoves;
							return beta;	// fail-hard beta-cutoff
						}
						if( mode == PV && score > alpha ) {
							alpha = score;
							pvFound = true;
						}
						if (prom > (PAWN | colormask) + 2){
							Pieces[prom - 2] ^= to;
							zobr ^= zobrist::keys[toSq][prom - 2];
						}
					}
				} else {
					Pieces[QUEEN | color] ^= to;
					zobr ^= zobrist::keys[toSq][QUEEN | color];
				}
				removeLastHistoryEntry();
				Pieces[captured] ^= to;
				zobr ^= zobrist::keys[toSq][captured];
				zobr ^= zobrist::keys[toSq-diff][PAWN | color];
				Pieces[PAWN | color] ^= from;
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color^1)] ^= to;
				attc &= attc - 1;
			}
		}
	}
	if (color==white){
		attacking[0] = notfile0 & notlastRank_w & (Pieces[PAWN | color] << 7);
		attacking[1] = notfile7 & notlastRank_w & (Pieces[PAWN | color] << 9);
	} else {
		attacking[0] = notfile0 & notlastRank_b & (Pieces[PAWN | color] >> 9);
		attacking[1] = notfile7 & notlastRank_b & (Pieces[PAWN | color] >> 7);
	}
	Zobrist toggle;
	for (int captured = QUEEN | (color^1); captured >= 0 ; captured-=2){
		for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
			attc = attacking[at] & Pieces[captured];
			while (attc!=0){
				to = attc & -attc;
				if (color == white){
					from = to >> diff;
				} else {
					from = to << -diff;
				}
				tf = to | from;
				toSq = square(to);
				toggle = zobrist::keys[toSq][PAWN | color];
				toggle ^= zobrist::keys[toSq-diff][PAWN | color];
				toggle ^= zobrist::keys[toSq][captured];
				Pieces[captured] ^= to;
				Pieces[PAWN | color] ^= tf;
				zobr ^= toggle;
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color^1)] ^= to;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[captured] ^= to;
						Pieces[PAWN | color] ^= tf;
						zobr ^= toggle;
						Pieces[CPIECES | color] ^= tf;
						Pieces[CPIECES | (color^1)] ^= to;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[captured] ^= to;
				Pieces[PAWN | color] ^= tf;
				zobr ^= toggle;
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color^1)] ^= to;
				attc &= attc - 1;
			}
		}
	}
	for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
		if ((attacking[at] & tmpEnPassant) != 0){
			toSq = square(tmpEnPassant);
			tf = tmpEnPassant;
			bitboard cp;
			if (color == white){
				tf |= tmpEnPassant >> diff;
				cp = tmpEnPassant >> 8;
			} else {
				tf |= tmpEnPassant << -diff;
				cp = tmpEnPassant << 8;
			}
			toggle = zobrist::keys[toSq][PAWN | color];
			toggle ^= zobrist::keys[toSq-diff][PAWN | color];
			toggle ^= zobrist::keys[toSq+(color==white)?-8:8][PAWN | (color ^ 1)];
			Pieces[PAWN | color] ^= tf;
			Pieces[PAWN | (color^1)] ^= cp;
			zobr ^= toggle;
			Pieces[CPIECES | color] ^= tf;
			Pieces[CPIECES | (color^1)] ^= cp;
			addToHistory(zobr);
			if (validPosition<color>()){
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				if( score >= beta ) {
					removeLastHistoryEntry();
					Pieces[PAWN | color] ^= tf;
					Pieces[PAWN | (color^1)] ^= cp;
					zobr ^= toggle;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color^1)] ^= cp;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
					if (color==black) --fullmoves;
					return beta;	// fail-hard beta-cutoff
				}
				if( mode == PV && score > alpha ) {
					alpha = score;
					pvFound = true;
				}
			}
			removeLastHistoryEntry();
			Pieces[PAWN | color] ^= tf;
			Pieces[PAWN | (color^1)] ^= cp;
			zobr ^= toggle;
			Pieces[CPIECES | color] ^= tf;
			Pieces[CPIECES | (color^1)] ^= cp;
		}
	}
	bitboard All_Pieces = Pieces[CPIECES | white];
	All_Pieces |= Pieces[CPIECES | black];
	bitboard empty = ~All_Pieces;
	bitboard moving = empty;
	if (color == white){
		moving &= lastRank_w & Pieces[PAWN | color] << 8;
	} else {
		moving &= lastRank_b & Pieces[PAWN | color] >> 8;
	}
	while (moving != 0){
		to = moving & -moving;
		if (color == white){
			from = to >> 8;
		} else {
			from = to << 8;
		}
		tf = to | from;
		toSq = square(to);
		Pieces[PAWN | color] ^= from;
		Pieces[QUEEN | color] ^= to;
		zobr ^= zobrist::keys[toSq][QUEEN | color];
		zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
		Pieces[CPIECES | color] ^= tf;
		addToHistory(zobr);
		if (validPosition<color>()) {
			for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				Pieces[prom] ^= to;
				zobr ^= zobrist::keys[toSq][prom];
				if( score >= beta ) {
					removeLastHistoryEntry();
					Pieces[PAWN | color] ^= from;
					zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
					Pieces[CPIECES | color] ^= tf;
					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
					if (color==black) --fullmoves;
					return beta;	// fail-hard beta-cutoff
				}
				if( mode == PV && score > alpha ) {
					alpha = score;
					pvFound = true;
				}
				if (prom > (PAWN | colormask) + 2){
					Pieces[prom - 2] ^= to;
					zobr ^= zobrist::keys[toSq][prom - 2];
				}
			}
		} else {
			Pieces[QUEEN | color] ^= to;
			zobr ^= zobrist::keys[toSq][QUEEN | color];
		}
		removeLastHistoryEntry();
		Pieces[PAWN | color] ^= from;
		zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
		Pieces[CPIECES | color] ^= tf;
		moving &= moving - 1;
	}
	bitboard attack[64], frombb[64];
	int piecet[64], fromSq[64], n(0);
	bitboard tmp = Pieces[KNIGHT | color];
	while (tmp != 0){
		frombb[n] = tmp & -tmp;
		fromSq[n] = square(frombb[n]);
		attack[n] = KnightMoves[fromSq[n]];
		piecet[n] = KNIGHT | color;
		++n;
		tmp &= tmp - 1;
	}
	tmp = Pieces[BISHOP | color];
	while (tmp != 0){
		frombb[n] = tmp & -tmp;
		fromSq[n] = square(frombb[n]);
		attack[n] = bishopAttacks(All_Pieces, fromSq[n]);
		piecet[n] = BISHOP | color;
		++n;
		tmp &= tmp - 1;
	}
	int firstRook = n;
	tmp = Pieces[ROOK | color];
	while (tmp != 0){
		frombb[n] = tmp & -tmp;
		fromSq[n] = square(frombb[n]);
		attack[n] = rookAttacks(All_Pieces, fromSq[n]);
		piecet[n] = ROOK | color;
		++n;
		tmp &= tmp - 1;
	}
	int firstQueen = n;
	tmp = Pieces[QUEEN | color];
	while (tmp != 0){
		frombb[n] = tmp & -tmp;
		fromSq[n] = square(frombb[n]);
		attack[n] = queenAttacks(All_Pieces, fromSq[n]);
		piecet[n] = QUEEN | color;
		++n;
		tmp &= tmp - 1;
	}
	frombb[n] = Pieces[KING | color] & -Pieces[KING | color];
	fromSq[n] = square(frombb[n]);
	attack[n] = KingMoves[fromSq[n]];
	piecet[n] = KING | color;
	//n : position of last bitboard generated

	bitboard cpt;
	if ((castling & castlingrights[color]) == 0){
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			cpt = Pieces[captured];
			for (int i = 0 ; i <= n ; ++i) {
				tmp = cpt & attack[i];
				while (tmp != 0){
					to = tmp & -tmp;
					toSq = square(to);
					tf = to | frombb[i];
					toggle = zobrist::keys[toSq][captured];
					toggle ^= zobrist::keys[toSq][piecet[i]];
					toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
					Pieces[captured] ^= to;
					Pieces[piecet[i]] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					addToHistory(zobr);
					if (validPosition<color>()){
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							Pieces[piecet[i]] ^= tf;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color ^ 1)] ^= to;
							zobr ^= toggle;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
							if (color==black) --fullmoves;
							return beta;	// fail-hard beta-cutoff
						}
						if( mode == PV && score > alpha ) {
							alpha = score;
							pvFound = true;
						}
					}
					removeLastHistoryEntry();
					Pieces[captured] ^= to;
					Pieces[piecet[i]] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					tmp &= tmp - 1;
				}
			}
		}

		halfmoves = oldhm + 1;
		//normal moves of non-Pawns
		for (int i = 0 ; i <= n ; ++i) {
			tmp = attack[i] & empty;
			while (tmp != 0){
				to = tmp & -tmp;
				toSq = square(to);
				tf = to | frombb[i];
				toggle = zobrist::keys[toSq][piecet[i]];
				toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
				Pieces[piecet[i]] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[piecet[i]] ^= tf;
						Pieces[CPIECES | color] ^= tf;
						zobr ^= toggle;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[piecet[i]] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				tmp &= tmp-1;
			}
		}
	} else {
		bitboard oldcastling = castling;
		key ct = zobrist::castling[(castling*castlingsmagic)>>59];
		key ct2;
		for (int captured = QUEEN | (color ^ 1); captured >= 0 ; captured -= 2){
			cpt = Pieces[captured];
			int i = 0;
			for (; i < firstRook ; ++i) {
				tmp = cpt & attack[i];
				while (tmp != 0){
					to = tmp & -tmp;
					toSq = square(to);
					tf = to | frombb[i];
					toggle = zobrist::keys[toSq][captured];
					toggle ^= zobrist::keys[toSq][piecet[i]];
					toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
					Pieces[captured] ^= to;
					Pieces[piecet[i]] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					addToHistory(zobr);
					if (validPosition<color>()){
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							Pieces[piecet[i]] ^= tf;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color ^ 1)] ^= to;
							zobr ^= toggle;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
							if (color==black) --fullmoves;
							return beta;	// fail-hard beta-cutoff
						}
						if( mode == PV && score > alpha ) {
							alpha = score;
							pvFound = true;
						}
					}
					removeLastHistoryEntry();
					Pieces[captured] ^= to;
					Pieces[piecet[i]] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					tmp &= tmp - 1;
				}
			}
			zobr ^= ct;
			for ( ; i < firstQueen ; ++i){
				castling &= ~frombb[i];
				ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
				zobr ^= ct2;
				tmp = cpt & attack[i];
				while (tmp != 0){
					to = tmp & -tmp;
					toSq = square(to);
					tf = to | frombb[i];
					toggle = zobrist::keys[toSq][captured];
					toggle ^= zobrist::keys[toSq][ROOK | color];
					toggle ^= zobrist::keys[fromSq[i]][ROOK | color];
					Pieces[captured] ^= to;
					Pieces[ROOK | color] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					addToHistory(zobr);
					if (validPosition<color>()){
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							Pieces[ROOK | color] ^= tf;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color ^ 1)] ^= to;
							zobr ^= toggle;

							zobr ^= ct2;
							zobr ^= ct;
							castling = oldcastling;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
							if (color==black) --fullmoves;
							return beta;	// fail-hard beta-cutoff
						}
						if( mode == PV && score > alpha ) {
							alpha = score;
							pvFound = true;
						}
					}
					removeLastHistoryEntry();
					Pieces[captured] ^= to;
					Pieces[ROOK | color] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					tmp &= tmp - 1;
				}
				zobr ^= ct2;
				castling = oldcastling;
			}
			zobr ^= ct;
			for (; i < n ; ++i) {
				tmp = cpt & attack[i];
				while (tmp != 0){
					to = tmp & -tmp;
					toSq = square(to);
					tf = to | frombb[i];
					toggle = zobrist::keys[toSq][captured];
					toggle ^= zobrist::keys[toSq][QUEEN | color];
					toggle ^= zobrist::keys[fromSq[i]][QUEEN | color];
					Pieces[captured] ^= to;
					Pieces[QUEEN | color] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					addToHistory(zobr);
					if (validPosition<color>()){
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							Pieces[QUEEN | color] ^= tf;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color ^ 1)] ^= to;
							zobr ^= toggle;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
							if (color==black) --fullmoves;
							return beta;	// fail-hard beta-cutoff
						}
						if( mode == PV && score > alpha ) {
							alpha = score;
							pvFound = true;
						}
					}
					removeLastHistoryEntry();
					Pieces[captured] ^= to;
					Pieces[QUEEN | color] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					Pieces[CPIECES | (color ^ 1)] ^= to;
					zobr ^= toggle;
					tmp &= tmp - 1;
				}
			}
			castling &= castlingc<color>::deactrights;
			ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
			zobr ^= ct;
			zobr ^= ct2;
			tmp = cpt & attack[i];
			while (tmp != 0){
				to = tmp & -tmp;
				toSq = square(to);
				tf = to | frombb[i];
				toggle = zobrist::keys[toSq][captured];
				toggle ^= zobrist::keys[toSq][KING | color];
				toggle ^= zobrist::keys[fromSq[i]][KING | color];
				Pieces[captured] ^= to;
				Pieces[KING | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color ^ 1)] ^= to;
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[captured] ^= to;
						Pieces[KING | color] ^= tf;
						Pieces[CPIECES | color] ^= tf;
						Pieces[CPIECES | (color ^ 1)] ^= to;
						zobr ^= toggle;
						castling = oldcastling;
						zobr ^= ct;
						zobr ^= ct2;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[captured] ^= to;
				Pieces[KING | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				Pieces[CPIECES | (color ^ 1)] ^= to;
				zobr ^= toggle;
				tmp &= tmp - 1;
			}
			castling = oldcastling;
			zobr ^= ct;
			zobr ^= ct2;
		}
		halfmoves = oldhm + 1;
		if ((castling & (castlingc<color>::KingSide) & Pieces[ROOK | color])!=0 && (castlingc<color>::KingSideSpace & All_Pieces)==0 && notAttacked<color^1>(castlingc<color>::KSCPassing) && validPosition<color>()){
			castling &= castlingc<color>::deactrights;
			key toggle = zobrist::keys[0][ROOK | color];
			toggle ^= zobrist::keys[2][ROOK | color];
			toggle ^= ct;
			toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
			toggle ^= zobrist::keys[3][KING | color];
			toggle ^= zobrist::keys[1][KING | color];
			Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
			Pieces[KING | color] ^= castlingc<color>::KSCKT;
			Pieces[CPIECES | color] ^= castlingc<color>::KSCFT;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition<color>()){
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				if( score >= beta ) {
					removeLastHistoryEntry();
					Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
					Pieces[KING | color] ^= castlingc<color>::KSCKT;
					Pieces[CPIECES | color] ^= castlingc<color>::KSCFT;
					zobr ^= toggle;
					castling = oldcastling;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
					if (color==black) --fullmoves;
					return beta;	// fail-hard beta-cutoff
				}
				if( mode == PV && score > alpha ) {
					alpha = score;
					pvFound = true;
				}
			}
			removeLastHistoryEntry();
			Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
			Pieces[KING | color] ^= castlingc<color>::KSCKT;
			Pieces[CPIECES | color] ^= castlingc<color>::KSCFT;
			zobr ^= toggle;
			castling = oldcastling;
		}
		if ((castling & (castlingc<color>::QueenSide) & Pieces[ROOK | color])!=0 && (castlingc<color>::QueenSideSpace & All_Pieces)==0 && notAttacked<color^1>(castlingc<color>::QSCPassing) && validPosition<color>()){
			castling &= castlingc<color>::deactrights;
			key toggle = zobrist::keys[7][ROOK | color];
			toggle ^= zobrist::keys[4][ROOK | color];
			toggle ^= ct;
			toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
			toggle ^= zobrist::keys[3][KING | color];
			toggle ^= zobrist::keys[5][KING | color];
			Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
			Pieces[KING | color] ^= castlingc<color>::QSCKT;
			Pieces[CPIECES | color] ^= castlingc<color>::QSCFT;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition<color>()){
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				if( score >= beta ) {
					removeLastHistoryEntry();
					Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
					Pieces[KING | color] ^= castlingc<color>::QSCKT;
					Pieces[CPIECES | color] ^= castlingc<color>::QSCFT;
					zobr ^= toggle;
					castling = oldcastling;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
					if (color==black) --fullmoves;
					return beta;	// fail-hard beta-cutoff
				}
				if( mode == PV && score > alpha ) {
					alpha = score;
					pvFound = true;
				}
			}
			removeLastHistoryEntry();
			Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
			Pieces[KING | color] ^= castlingc<color>::QSCKT;
			Pieces[CPIECES | color] ^= castlingc<color>::QSCFT;
			zobr ^= toggle;
			castling = oldcastling;
		}

		int i = 0;
		for (; i < firstRook ; ++i) {
			tmp = attack[i] & empty;
			while (tmp != 0){
				to = tmp & -tmp;
				toSq = square(to);
				tf = to | frombb[i];
				toggle = zobrist::keys[toSq][piecet[i]];
				toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
				Pieces[piecet[i]] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[piecet[i]] ^= tf;
						Pieces[CPIECES | color] ^= tf;
						zobr ^= toggle;
						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[piecet[i]] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				tmp &= tmp-1;
			}
		}
		zobr ^= ct;
		for (; i < firstQueen ; ++i) {
			castling &= ~frombb[i];
			ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
			zobr ^= ct2;
			tmp = attack[i] & empty;
			while (tmp != 0){
				to = tmp & -tmp;
				toSq = square(to);
				tf = to | frombb[i];
				toggle = zobrist::keys[toSq][ROOK | color];
				toggle ^= zobrist::keys[fromSq[i]][ROOK | color];
				Pieces[ROOK | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[ROOK | color] ^= tf;
						Pieces[CPIECES | color] ^= tf;
						zobr ^= toggle;

						zobr ^= ct2;
						zobr ^= ct;
						castling = oldcastling;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[ROOK | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				tmp &= tmp-1;
			}
			zobr ^= ct2;
			castling = oldcastling;
		}
		zobr ^= ct;
		for (; i < n ; ++i) {
			tmp = attack[i] & empty;
			while (tmp != 0){
				to = tmp & -tmp;
				toSq = square(to);
				tf = to | frombb[i];
				toggle = zobrist::keys[toSq][QUEEN | color];
				toggle ^= zobrist::keys[fromSq[i]][QUEEN | color];
				Pieces[QUEEN | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition<color>()){
					score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
					if( score >= beta ) {
						removeLastHistoryEntry();
						Pieces[QUEEN | color] ^= tf;
						Pieces[CPIECES | color] ^= tf;
						zobr ^= toggle;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
						if (color==black) --fullmoves;
						return beta;	// fail-hard beta-cutoff
					}
					if( mode == PV && score > alpha ) {
						alpha = score;
						pvFound = true;
					}
				}
				removeLastHistoryEntry();
				Pieces[QUEEN | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;
				tmp &= tmp-1;
			}
		}
		castling &= castlingc<color>::deactrights;
		ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
		zobr ^= ct;
		zobr ^= ct2;
		tmp = attack[i] & empty;
		while (tmp != 0){
			to = tmp & -tmp;
			toSq = square(to);
			tf = to | frombb[i];
			toggle = zobrist::keys[toSq][KING | color];
			toggle ^= zobrist::keys[fromSq[i]][KING | color];
			Pieces[KING | color] ^= tf;
			Pieces[CPIECES | color] ^= tf;
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition<color>()){
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				if( score >= beta ) {
					removeLastHistoryEntry();
					Pieces[KING | color] ^= tf;
					Pieces[CPIECES | color] ^= tf;
					zobr ^= toggle;

					castling = oldcastling;
					zobr ^= ct;
					zobr ^= ct2;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
					if (color==black) --fullmoves;
					return beta;	// fail-hard beta-cutoff
				}
				if( mode == PV && score > alpha ) {
					alpha = score;
					pvFound = true;
				}
			}
			removeLastHistoryEntry();
			Pieces[KING | color] ^= tf;
			Pieces[CPIECES | color] ^= tf;
			zobr ^= toggle;
			tmp &= tmp - 1;
		}
		castling = oldcastling;
		zobr ^= ct;
		zobr ^= ct2;
	}
	halfmoves = 0;

	moving = empty;
	if (color == white){
		moving &= notlastRank_w & Pieces[PAWN | color] << 8;
	} else {
		moving &= notlastRank_b & Pieces[PAWN | color] >> 8;
	}
	while (moving != 0){
		to = moving & -moving;
		if (color == white){
			tf = to | (to >> 8);
		} else {
			tf = to | (to << 8);
		}
		toSq = square(to);
		toggle = zobrist::keys[toSq][PAWN | color];
		toggle ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
		Pieces[PAWN | color] ^= tf;
		Pieces[CPIECES | color] ^= tf;
		zobr ^= toggle;
		addToHistory(zobr);
		if (validPosition<color>()) {
			score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
			if( score >= beta ) {
				removeLastHistoryEntry();
				Pieces[PAWN | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;

				halfmoves = oldhm;
				zobr ^= zobrist::blackKey;
				enPassant = tmpEnPassant;
				if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
				if (color==black) --fullmoves;
				return beta;	// fail-hard beta-cutoff
			}
			if( mode == PV && score > alpha ) {
				alpha = score;
				pvFound = true;
			}
		}
		removeLastHistoryEntry();
		Pieces[PAWN | color] ^= tf;
		Pieces[CPIECES | color] ^= tf;
		zobr ^= toggle;
		moving &= moving - 1;
	}
	if (color == white){
		moving = ( ( ( ( Pieces[PAWN | color] & pstartRank_w ) << 8 ) & empty ) << 8 ) & empty;
	} else {
		moving = ( ( ( ( Pieces[PAWN | color] & pstartRank_b ) >> 8 ) & empty ) >> 8 ) & empty;
	}
	while (moving != 0){
		to = moving & -moving;
		if (color == white){
			tf = to | (to >> 16);
			enPassant = to >> 8;
		} else {
			tf = to | (to << 16);
			enPassant = to << 8;
		}
		toSq = square(to);
		toggle = zobrist::keys[toSq][PAWN | color];
		toggle ^= zobrist::keys[toSq+((color==white)?-16:16)][PAWN | color];
		toggle ^= zobrist::enPassant[7&square( enPassant )];
		Pieces[PAWN | color] ^= tf;
		Pieces[CPIECES | color] ^= tf;
		zobr ^= toggle;
		addToHistory(zobr);
		if (validPosition<color>()) {
			score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
			if( score >= beta ) {
				removeLastHistoryEntry();
				Pieces[PAWN | color] ^= tf;
				Pieces[CPIECES | color] ^= tf;
				zobr ^= toggle;

				halfmoves = oldhm;
				zobr ^= zobrist::blackKey;
				enPassant = tmpEnPassant;
				if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
				if (color==black) --fullmoves;
				return beta;	// fail-hard beta-cutoff
			}
			if( mode == PV && score > alpha ) {
				alpha = score;
				pvFound = true;
			}
		}
		removeLastHistoryEntry();
		Pieces[PAWN | color] ^= tf;
		Pieces[CPIECES | color] ^= tf;
		zobr ^= toggle;
		moving &= moving - 1;
	}

	halfmoves = oldhm;
	zobr ^= zobrist::blackKey;
	enPassant = tmpEnPassant;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
	if (color==black) --fullmoves;
#ifdef WIN32
	if (mode == Perft && depth == dividedepth) {
		U64 moves = horizonNodes;
		moves -= stHorNodes;
		int oldplaying = playing;
		playing = color;
		//std::cout << pre << getFEN() << '\t' << moves;
#ifdef WIN32
		DWORD bytes_read, bytes_written;
		CHAR buffer[4096];
		// Write a message to the child process
		WriteFile(child_input_write, ("setboard "+getFEN()+"\n").c_str(), ("setboard "+getFEN()+"\n").length(), &bytes_written, NULL);
		//ReadFile( child_output_read, buffer, sizeof(buffer), &bytes_read, NULL);
		WriteFile(child_input_write, "perft ", strlen("perft "), &bytes_written, NULL);
		char str[5];
		itoa(depth, str, 10);
		WriteFile(child_input_write, str, strlen(str), &bytes_written, NULL);
		WriteFile(child_input_write, "\n", strlen("\n"), &bytes_written, NULL);
		// Read the message from the child process
		ReadFile( child_output_read, buffer, sizeof(buffer), &bytes_read, NULL);
		buffer[bytes_read] = 0;
		unsigned int a = 0;
		sscanf(buffer, "Nodes: %d,", &a);
		if (a != moves) {
			std::cout << pre << getFEN() << '\t' << moves << "\tFailed!\t" << a << '\n';
			std::cout << "-----------------------------------------\n";
			std::string oldpre = pre;
			pre += "\t";
			dividedepth = depth-1;
			search<mode, color>(alpha, beta, depth);
			search<mode, color>(alpha, beta, depth);
			std::cout << "-----------------------------------------" << std::endl;
			pre = oldpre;
			dividedepth = depth;
		}/** else {
			std::cout << "\tOK\n";
		}**/
		playing = oldplaying;
#else
		std::cout << endl;
#endif
	}
#endif
	if (mode == Perft) return alpha + 1;
	return alpha;
}

template<int color> int Board::quieSearch(int alpha, int beta){
	return 0;
}
#endif /* BOARD_H_ */
