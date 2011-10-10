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
#include "Values.h"
//#define NDEBUG
#include <assert.h>
#include <exception>
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
const bitboard dfRank_w = filled::rank[3];
const bitboard dfRank_b = filled::rank[4];
const bitboard pstartRank_w = filled::rank[1];
const bitboard pstartRank_b = filled::rank[6];
//color definitions
const int white = 0;
const int black = 1;
const int colormask = 1;

struct KingException : public std::exception {
	bool color;
	int number;
	KingException(bool k, int num) : color(k), number(num){}
	const char* what() const throw() {
		if (color == white){
			return "Invalid number of White Kings.";
		} else {
			return "Invalid number of Black Kings.";
		}
	}
};

struct MalformedFEN : public std::exception {
	MalformedFEN(){}
	const char* what() const throw() { return "Malformed FEN string."; }
};

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
		int pieceScore;

	public:
		//for Perft
		U64 horizonNodes;
		U64 nodes;
		int dividedepth;
#ifdef WIN32
		HANDLE child_input_write;
		HANDLE child_output_read;
#endif
		std::string pre;
		static int hashSize;

	public :
		//Construction
		static Board* createBoard(const char FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		static void initialize(){}
		Board(char fenBoard[] = (char*) "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", char fenPlaying = 'w', char fenCastling[] = (char*) "KQkq", int fenEnPX = -1, int fenEnPY = -1, int fenHC = 0, int fenFM = 1);

		void make(move m);

		//for debug
		std::string getFEN();
		void printbb(bitboard);
		void print();
		U64 perft(int depth);
		int test(int depth);

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
		template<int color> int getEvaluation();
		template<int color> void deactivateCastlingRights();
		void togglePlaying();

		template<int color> bool validPosition();
		template<int color> bool validPosition(const bitboard &occ);
		template<int color> bool notAttacked(bitboard target);
		template<int color> bool notAttacked(bitboard target, bitboard occ);
		template<int color> bitboard kingIsAttackedBy();

		template<int color> bool stalemate();

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

template<int color> bitboard Board::kingIsAttackedBy(){
	int sq = square(Pieces[KING | color]);
	bitboard occ = Pieces[CPIECES | white];
	occ |= Pieces[CPIECES | black];
	bitboard attackers = KnightMoves[sq];
	attackers &= Pieces[KNIGHT | (color^1)];
	attackers |= rookAttacks(occ, sq) & (Pieces[ROOK | (color^1)] | Pieces[QUEEN | (color^1)]);
	attackers |= bishopAttacks(occ, sq) & (Pieces[BISHOP | (color^1)] | Pieces[QUEEN | (color^1)]);
	if (color == black){
		attackers |= (((Pieces[KING | color] >> 7) & notfile7) | ((Pieces[KING | color] >> 9) & notfile0)) & Pieces[PAWN | white];
	} else {
		attackers |= (((Pieces[KING | color] << 9) & notfile7) | ((Pieces[KING | color] << 7) & notfile0)) & Pieces[PAWN | black];
	}
	return attackers;
}

/**
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<int color> inline bool Board::notAttacked(bitboard target){
	bitboard occ = Pieces[CPIECES | white] | Pieces[CPIECES | black];
	return notAttacked<color>(target, occ);
}
/**
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<int color> inline bool Board::notAttacked(bitboard target, bitboard occ){
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
	bitboard att = Pieces[BISHOP | color] | Pieces[QUEEN | color];
	if ((att & bishopAttacks(occ, sq)) != 0) return false;
	att = Pieces[ROOK | color] | Pieces[QUEEN | color];
	if ((att & rookAttacks(occ, sq)) != 0) return false;
	return true;
}

template<int color> inline bool Board::validPosition(const bitboard &occ) {
	return notAttacked<color^1>(Pieces[KING | color], occ);
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
	}
}

template<SearchMode mode, int color> int Board::search(int alpha, int beta, int depth){
	++nodes;
	if (depth == 0) {
		++horizonNodes;
		if (mode == Perft) {
			if (dividedepth==0) {
				int oldplaying = playing;
				playing = color;
				std::cout << pre << getFEN() << '\n';
				playing = oldplaying;
			}
			return beta;
		}
		return quieSearch<color>(mode==ZW?beta-1:alpha, beta);
	}
	bitboard checkedBy = kingIsAttackedBy<color>();
	//TODO special move generator if in check if (checkedBy == bitboard(0)){
	U64 stNodes (nodes);
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
		pieceScore -= Value::piece[captured];
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
					pieceScore -= Value::piece[PAWN | color];
					for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
						pieceScore += Value::piece[prom];
						score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
						Pieces[prom] ^= to;
						zobr ^= zobrist::keys[toSq][prom];
						pieceScore -= Value::piece[prom];
						if( score >= beta ) {
							removeLastHistoryEntry();
							Pieces[captured] ^= to;
							zobr ^= zobrist::keys[toSq][captured];
							zobr ^= zobrist::keys[toSq-diff][PAWN | color];
							Pieces[PAWN | color] ^= from;
							Pieces[CPIECES | color] ^= tf;
							Pieces[CPIECES | (color^1)] ^= to;
							pieceScore += Value::piece[PAWN | color];
							pieceScore += Value::piece[captured];
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
					pieceScore += Value::piece[PAWN | color];
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
		pieceScore += Value::piece[captured];
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
		pieceScore -= Value::piece[captured];
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
						pieceScore += Value::piece[captured];

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
		pieceScore += Value::piece[captured];
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
				pieceScore -= Value::piece[PAWN | (color ^ 1)];
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				pieceScore += Value::piece[PAWN | (color ^ 1)];
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
			pieceScore -= Value::piece[PAWN | color];
			for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
				pieceScore += Value::piece[prom];
				score = searchDeeper<mode, color^1>(alpha, beta, depth, pvFound);
				pieceScore -= Value::piece[prom];
				Pieces[prom] ^= to;
				zobr ^= zobrist::keys[toSq][prom];
				if( score >= beta ) {
					pieceScore += Value::piece[PAWN | color];
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
			pieceScore += Value::piece[PAWN | color];
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
			pieceScore -= Value::piece[captured];
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
							pieceScore += Value::piece[captured];

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
			pieceScore += Value::piece[captured];
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
			pieceScore -= Value::piece[captured];
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
							pieceScore += Value::piece[captured];

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
							pieceScore += Value::piece[captured];

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
							pieceScore += Value::piece[captured];

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
						pieceScore += Value::piece[captured];

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
			pieceScore += Value::piece[captured];
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
#ifdef WIN32color
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
	if (stNodes == nodes) {
		if (checkedBy == 0) return 0; //PAT
		return -Value::MAT; //MATed
	}
	return alpha;
	//TODO special move generator when in check
	/**} else {

		if (checkedBy & (checkedBy - 1) == 0){
			//Single Check
		toggle = zobrist::keys[toSq][PAWN | color];
		toggle ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
		} else {
			//Double Check
		}
	}**/
}

template<int color> int Board::quieSearch(int alpha, int beta){
	//TODO quieSearch
	if (color == white){
		return getEvaluation<color>();
	} else {
		return -getEvaluation<color>();
	}
}

template<int color> bool Board::stalemate(){
	bitboard occ = Pieces[CPIECES | white];
	occ |= Pieces[CPIECES | black];
	bitboard empty = ~occ;
	bitboard moving = empty;
	bool res = false;
	if (color==white){
		moving &= Pieces[PAWN | color] << 8;
	} else {
		moving &= Pieces[PAWN | color] >> 8;
	}
	bitboard moving2 = moving;
	bitboard to, tf;
	while (moving != 0){
		to = moving & -moving;
		if (color == white){
			tf = to | (to >> 8);
		} else {
			tf = to | (to << 8);
		}
		if (validPosition<color>(occ ^ tf)) return false;
		moving &= moving - 1;
	}
	if (color==white){
		moving2 <<= 8;
		moving2 &= dfRank_w;
	} else {
		moving2 >>= 8;
		moving2 &= dfRank_b;
	}
	moving2 &= empty;
	while (moving2 != 0){
		to = moving2 & -moving2;
		if (color == white){
			tf = to | (to >> 16);
		} else {
			tf = to | (to << 16);
		}
		if (validPosition<color>(occ ^ tf)) return false;
		moving2 &= moving2 - 1;
	}
	bitboard from = Pieces[KING | color];
	int fromSq = square(from);
	bitboard att = KingMoves[fromSq];
	moving = empty & att;
	occ ^= from;
	while (moving != 0){
		to = moving & -moving;
		if (notAttacked<color^1>(to, occ^to)) {
			occ ^= from;
			return false;
		}
		moving &= moving - 1;
	}
	for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
		moving = Pieces[captured] & att;
		while (moving != 0){
			to = moving & -moving;
			Pieces[captured] ^= to;
			res = notAttacked<color^1>(to, occ);
			Pieces[captured] ^= to;
			if (res) {
				occ ^= from;
				return false;
			}
			moving &= moving - 1;
		}
	}
	occ ^= from;
	bitboard temp = Pieces[KNIGHT | color];
	while (temp != 0){
		from = temp & -temp;
		fromSq = square(from);
		att = KnightMoves[fromSq];
		moving = empty & att;
		while (moving != 0){
			to = moving & -moving;
			tf = to | from;
			if (validPosition<color>(occ ^ tf)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from);
				Pieces[captured] ^= to;
				if (res) return false;
				moving &= moving - 1;
			}
		}
	}
	temp = Pieces[BISHOP | color];
	while (temp != 0){
		from = temp & -temp;
		fromSq = square(from);
		att = bishopAttacks(occ, fromSq);
		moving = empty & att;
		while (moving != 0){
			to = moving & -moving;
			tf = to | from;
			if (validPosition<color>(occ ^ tf)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from);
				Pieces[captured] ^= to;
				if (res) return false;
				moving &= moving - 1;
			}
		}
	}
	temp = Pieces[ROOK | color];
	while (temp != 0){
		from = temp & -temp;
		fromSq = square(from);
		att = rookAttacks(occ, fromSq);
		moving = empty & att;
		while (moving != 0){
			to = moving & -moving;
			tf = to | from;
			if (validPosition<color>(occ ^ tf)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from);
				Pieces[captured] ^= to;
				if (res) return false;
				moving &= moving - 1;
			}
		}
	}
	temp = Pieces[QUEEN | color];
	while (temp != 0){
		from = temp & -temp;
		fromSq = square(from);
		att = queenAttacks(occ, fromSq);
		moving = empty & att;
		while (moving != 0){
			to = moving & -moving;
			tf = to | from;
			if (validPosition<color>(occ ^ tf)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from);
				Pieces[captured] ^= to;
				if (res) return false;
				moving &= moving - 1;
			}
		}
	}
	bitboard attacking[2], attc;
	if (color==white){
		attacking[0] = notfile0 & (Pieces[PAWN | color] << 7);
		attacking[1] = notfile7 & (Pieces[PAWN | color] << 9);
	} else {
		attacking[0] = notfile0 & (Pieces[PAWN | color] >> 9);
		attacking[1] = notfile7 & (Pieces[PAWN | color] >> 7);
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
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from);
				Pieces[captured] ^= to;
				if (res) return false;
				attc &= attc - 1;
			}
		}
	}
	if (enPassant!=0){
		if (color == white){
			moving = (enPassant >> 9) | (enPassant >> 7);
		} else {
			moving = (enPassant << 9) | (enPassant << 7);
		}
		bitboard cpt;
		moving &= Pieces[PAWN | color];
		while (moving != 0){
			from = moving & -moving;
			cpt = (color == white) ? (enPassant >> 8) : (enPassant << 8);
			Pieces[PAWN | (color ^ 1)] ^= cpt;
			res = validPosition<color>(occ ^ cpt ^ from ^ enPassant);
			Pieces[PAWN | (color ^ 1)] ^= cpt;
			if (res) return false;
			attc &= attc - 1;
		}
	}
	//If castling was available, King would had a normal move as well!
	return true;
}

template<int color> int Board::getEvaluation(){
	if (stalemate<color>()){
		if (validPosition<color>()) return 0; //stalemate
		if (color == white){
			//White mated
			return -Value::MAT;
		}
		//Black mated
		return Value::MAT;
	}
	return pieceScore;
}
#endif /* BOARD_H_ */
