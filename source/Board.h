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
#include <boost/thread/thread.hpp>
#include "TranspositionTable.h"

#ifdef DIVIDEPERFT
#include <windows.h>
#endif

typedef chapeiro::bitboard bitboard;
typedef chapeiro::zobrist Zobrist;
//typedef chapeiro::move move;


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

/**
const int PIECEMASK = 14;
const int LASTPIECE = 12;
const int PIECESMAX = 16;
const int WRONG_PIECE = -10;**/

#define PIECEMASK (14)
#define LASTPIECE (12)
#define PIECESMAX LASTPIECE
#define WRONG_PIECE (-10)

#define fd_rank(x) (0xFFull << ((x) << 3))
#define fd_file(x) (0x0101010101010101ull << (7^(x)))

/**
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
**/
const bitboard lastRank_w = fd_rank(7);//filled::rank[7];
const bitboard lastRank_b = fd_rank(0);//filled::rank[0];
const bitboard notfile0 = ~fd_file(0);//notFilled::file[0];
const bitboard notfile7 = ~fd_file(7);//notFilled::file[7];
const bitboard notlastRank_w = ~fd_rank(7);//~filled::rank[7];
const bitboard notlastRank_b = ~fd_rank(0);//~filled::rank[0];
const bitboard dfRank_w = fd_rank(3);//filled::rank[3];
const bitboard dfRank_b = fd_rank(4);//filled::rank[4];
const bitboard pstartRank_w = fd_rank(1);//filled::rank[1];
const bitboard pstartRank_b = fd_rank(6);//filled::rank[6];

//color definitions
/**
const int white = 0;
const int black = 1;
const int colormask = 1;
**/

#define white (0)
#define black (1)
#define colormask (1)

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
	PV = 0,
	ZW = 1,
	Perft = 2,
	QuiescencePV = 4,
	QuiescenceZW = 5

};

const int quiescenceMask = 4;
//castling precomputed
template<int color> class castlingc {
public :
	static const bitboard KingSide = (color == white) ? 0x0000000000000001ull : 0x0100000000000000ull;
	static const bitboard QueenSide = (color == white) ? 0x0000000000000080ull : 0x8000000000000000ull;
	static const bitboard KingSideSpace = (color == white) ? 0x0000000000000006ull : 0x0600000000000000ull;
	static const bitboard QueenSideSpace = (color == white) ? 0x0000000000000070ull : 0x7000000000000000ull;
	static const bitboard KSCPassing = (color == white) ? 0x0000000000000004ull : 0x0400000000000000ull;
	static const int KSCPassingSq = (color == white) ? 2 : 58;//square(KSCPassing);
	static const bitboard QSCPassing = (color == white) ? 0x0000000000000010ull : 0x1000000000000000ull;
	static const int QSCPassingSq = (color == white) ? 4 : 60;//square(QSCPassing);
	static const int kingSqBefore = (color == white) ? 3 : 59;
	static const int kingSqAfterKSC = (color == white) ? 1 : 57;
	static const int kingSqAfterQSC = (color == white) ? 5 : 61;
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

#define All_Pieces(x) ((((x)&colormask)==white) ? White_Pieces : Black_Pieces)

extern int rootDepth;
class Board {
	private:
		//State
		bitboard Pieces[PIECESMAX];
		bitboard White_Pieces, Black_Pieces;
		unsigned long int kingSq[colormask + 1];
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
		boost::thread *searchThread;

	public:
		//for Perft
		U64 horizonNodes;
		U64 nodes;
		U64 qNodes;
		int dividedepth;
#ifdef DIVIDEPERFT
		HANDLE child_input_write;
		HANDLE child_output_read;
#endif
		std::string pre;

	public :
		//Construction
		static Board* createBoard(const char FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		static void initialize(){}
		Board(Board * b);
		Board(char fenBoard[] = (char*) "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", char fenPlaying = 'w', char fenCastling[] = (char*) "KQkq", int fenEnPX = -1, int fenEnPY = -1, int fenHC = 0, int fenFM = 1);
		~Board(){ if (searchThread) searchThread->join(); }

		void make(chapeiro::move m);

		//for debug
		std::string getFEN();
		std::string getFEN(int playingl);
		void print();
		U64 perft(int depth);
		int test(int depth);
		void go(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch);
		void stop();

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
		template<int color> int getEvaluation(int depth);
		template<int color> void deactivateCastlingRights();
		void togglePlaying();
		void startSearch(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch);
		std::string Board::extractPV(int depth);

		template<int color> bool validPosition();
		template<int color> bool validPosition(const bitboard &occ);
		template<int color> bool validPositionNonChecked();
		template<int color> bool validPositionNonChecked(bitboard occ);
		template<int color> bool notAttacked(bitboard target, int targetSq);
		template<int color> bool notAttacked(bitboard target, bitboard occ, int targetSq);
		template<int color> bitboard kingIsAttackedBy();

		template<int color> bool stalemate();

		bitboard bishopAttacks(bitboard occ, const int &sq);
		bitboard rookAttacks(bitboard occ, const int &sq);
		bitboard queenAttacks(bitboard occ, const int &sq);
		template<int color> bitboard getChecker(bitboard occ, unsigned long int &sq);
		template<int color> void filterAttackBB(bitboard &occ, unsigned long int &sq, bitboard &attack);
		template<int color> bitboard getNPinnedPawns(bitboard occ);
		template<int color> int getMove(bitboard tf, int prom);
		char * moveToString(int move, char* m);

		template<SearchMode mode, int color> int search(int alpha, int beta, int depth);
		template<SearchMode mode, int color> void searchDeeper(const int &alpha, const int &beta, const int &depth, const int &pvFound, int &score);
		template<int color> int quieSearch(int alpha, int beta);
};

inline int Board::getPieceIndex(char p){
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A') | black;
	return getWhitePieceIndex(p) | white;
}

inline void Board::updatePieces(int sq, int ind){
	Pieces[ind] ^= filled::normal[sq];
	if ( (ind & ~colormask) == KING ) kingSq[ind & colormask] = sq;
	zobr ^= zobrist::keys[sq][ind];
	All_Pieces(ind) ^= filled::normal[sq];
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
	bitboard occ = All_Pieces(white);
	occ |= All_Pieces(black);
	bitboard attackers = KnightMoves[kingSq[color]];
	attackers &= Pieces[KNIGHT | (color^1)];
	attackers |= rookAttacks(occ, kingSq[color]) & (Pieces[ROOK | (color^1)] | Pieces[QUEEN | (color^1)]);
	attackers |= bishopAttacks(occ, kingSq[color]) & (Pieces[BISHOP | (color^1)] | Pieces[QUEEN | (color^1)]);
	if (color == black){
		attackers |= (((Pieces[KING | color] >> 7) & notfile7) | ((Pieces[KING | color] >> 9) & notfile0)) & Pieces[PAWN | white];
	} else {
		attackers |= (((Pieces[KING | color] << 9) & notfile7) | ((Pieces[KING | color] << 7) & notfile0)) & Pieces[PAWN | black];
	}
	return attackers;
}

/**
 * uses only Pieces[ [PAWN ... KING] | color ], target, All_Pieces([white, black])
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<int color> inline bool Board::notAttacked(bitboard target, int targetSq){
	bitboard occ = All_Pieces(white) | All_Pieces(black);
	return notAttacked<color>(target, occ, targetSq);
}

/**
 * uses only Pieces[ [PAWN ... KING] | color ], target, occ
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<int color> inline bool Board::notAttacked(bitboard target, bitboard occ, int targetSq){
	assert((target & (target-1))==0);
	//assert(square(target) == targetSq);
	if (color == black){
		if ( ( (Pieces[PAWN | black] >> 7) & target & notfile7) != 0) return false;
		if ( ( (Pieces[PAWN | black] >> 9) & target & notfile0) != 0) return false;
	} else {
		if ( ( (Pieces[PAWN | white] << 7) & target & notfile0) != 0) return false;
		if ( ( (Pieces[PAWN | white] << 9) & target & notfile7) != 0) return false;
	}
	if ((Pieces[KNIGHT | color] & KnightMoves[targetSq])!=0) return false;
	if ((Pieces[KING | color] & KingMoves[targetSq])!=0) return false;
	bitboard att = Pieces[BISHOP | color] | Pieces[QUEEN | color];
	if ((att & bishopAttacks(occ, targetSq)) != 0) return false;
	att = Pieces[ROOK | color] | Pieces[QUEEN | color];
	return ((att & rookAttacks(occ, targetSq)) == 0ull);
}

template<int color> inline bool Board::validPosition(const bitboard &occ) {
	assert((Pieces[KING | color] & (Pieces[KING | color]-1)) == 0);
	return notAttacked<color^1>(Pieces[KING | color], occ, kingSq[color]);
}

template<int color> inline bool Board::validPositionNonChecked(bitboard occ) {
	bitboard att = Pieces[BISHOP | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )];
	if ((att & bishopAttacks(occ, kingSq[color])) != 0) return false;
	att = Pieces[ROOK | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )];
	if ((att & rookAttacks(occ, kingSq[color])) != 0) return false;
	return true;
}

template<int color> inline bool Board::validPositionNonChecked() {
	return validPositionNonChecked<color>(All_Pieces(white) | All_Pieces(black));
}

/**
 * uses only :
 *  kingSq[color],
 * 	Pieces[ [PAWN ... KING] | color^1 ],
 * 	Pieces[KING | color],
 * 	All_Pieces(white),
 * 	All_Pieces(black)
 */
template<int color> inline bool Board::validPosition() {
	assert((Pieces[KING | color] & (Pieces[KING | color]-1)) == bitboard(0));
	return notAttacked<color^1>(Pieces[KING | color], kingSq[color]);
}

template<SearchMode mode, int color> inline void Board::searchDeeper(const int &alpha, const int &beta, const int &depth, const int &pvFound, int &score){
	addToHistory(zobr);
	if (mode >= quiescenceMask){
		score = -search<mode, color>(-beta, -alpha, depth - 1);
	} else if (mode == PV){
		if (pvFound) {
			score = -search<ZW, color>(-1-alpha, -alpha, depth - 1);
			if ( score > alpha ) score = -search<PV, color>(-beta, -alpha, depth - 1);
		} else {
			score = -search<PV, color>(-beta, -alpha, depth - 1);
		}
	} else if (mode == ZW){
		score = -search<ZW, color>(-beta, -alpha, depth - 1);
	} else {
		score = -search<Perft, color>(-beta, -alpha, depth - 1);
	}
	removeLastHistoryEntry();
}

template<SearchMode mode, int color> int Board::search(int alpha, int beta, int depth){
	//alpha - lower bound
	//beta - higher bound
	++nodes;
	if (mode >= quiescenceMask){
		++qNodes;
		int standPat = getEvaluation<color>(depth);
		if (color == black) standPat = -standPat;
		if (mated(standPat)) {
			//--nodes;
			return standPat;
		}
		if (standPat >= beta) {
			statistics(++betaCutOff);
			return beta; //fail-hard beta-cutoff Opponent will have a better answer
		}
		/**
		 * check if standPat can become the lowest score, as player can probably get it by
		 * playing at least a quiet move, if he can not get a better one by a non-quiet
		 */
		if (standPat > alpha) alpha = standPat;
	} else if (depth == 0) {
		++horizonNodes;
		if (mode & Perft) {
			if (dividedepth==0) std::cout << pre << getFEN(color) << '\n';
			return beta;
		}
		//return search<QuiescencePV, color>(alpha, beta, depth);
		return search<(SearchMode) (mode | quiescenceMask), color>(mode==ZW?beta-1:alpha, beta, depth);
	}
	//FIXME
	int ttResult = retrieveTTEntry<mode>(zobr, depth, alpha, beta);
	//if (ttResult == ttExactScoreReturned) return alpha;
	if (alpha >= beta) {
		statistics(++hashHitCutOff);
		return alpha;
	}
	//TODO play killer move
	//TODO add heuristics
	bitboard checkedBy = kingIsAttackedBy<color>();
	unsigned long int toSq, tmpSq, bestProm = 0;
	bitboard bestTF(0);


	int oldhm (halfmoves);
	//if (color==white) boost::this_thread::interruption_point();
	//FIXME betaCutOffs save this to TT
	if (color==white) if (boost::this_thread::interruption_requested()) return inf; //TODO Revision! does not seem such a good idea :(
	if (color==black) ++fullmoves;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
	bitboard tmpEnPassant (enPassant);
	enPassant = bitboard(0);
	halfmoves = 0;
	zobr ^= zobrist::blackKey;
	int startingAlpha = alpha;
	U64 stNodes (nodes);
	U64 stHorNodes (horizonNodes);
	int score;
	bool pvFound = false;
	bitboard to, from, tf;
	bitboard occ = All_Pieces(white);
	occ |= All_Pieces(black);
	Zobrist toggle;
	bitboard tmp = occ;
	if (checkedBy == bitboard(0)){
		bitboard nPinnedPawn = getNPinnedPawns<color>(occ);
		bitboard attacking[2] = {Pieces[PAWN | color], Pieces[PAWN | color]};
		int kingFL = 7 & kingSq[color];
		int kingRK = kingSq[color] >> 3;
		int kingMD = 7-kingFL+kingRK;
		int kingAD = kingFL+kingRK;
		kingFL ^= 7;

		if (color==white){
			attacking[0] &= nPinnedPawn | filled::antiDiag[kingAD];
			attacking[0] <<= 7;
			attacking[0] &= notfile0 & lastRank_w;
			attacking[1] &= nPinnedPawn | filled::mainDiag[kingMD];
			attacking[1] <<= 9;
			attacking[1] &= notfile7 & lastRank_w;
		} else {
			attacking[0] &= nPinnedPawn | filled::mainDiag[kingMD];
			attacking[0] >>= 9;
			attacking[0] &= notfile0 & lastRank_b;
			attacking[1] &= nPinnedPawn | filled::antiDiag[kingAD];
			attacking[1] >>= 7;
			attacking[1] &= notfile7 & lastRank_b;
		}
		pieceScore -= Value::piece[PAWN | color];
		for (int captured = QUEEN | (color^1); captured >= 0 ; captured-=2){
			pieceScore -= Value::piece[captured];
			for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
				tmp = attacking[at] & Pieces[captured];
				while (tmp){
					to = tmp & -tmp;
					from = to;
					if (color == white){
						from >>= diff;
					} else {
						from <<= -diff;
					}
					tf = to | from;
					Pieces[captured] ^= to;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					Pieces[PAWN | color] ^= from;
					square(&toSq, to);
					zobr ^= zobrist::keys[toSq][captured];
					zobr ^= zobrist::keys[toSq-diff][PAWN | color];
					for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
						Pieces[prom] ^= to;
						pieceScore += Value::piece[prom];
						zobr ^= zobrist::keys[toSq][prom];
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= zobrist::keys[toSq][prom];
						pieceScore -= Value::piece[prom];
						Pieces[prom] ^= to;
						if( score >= beta ) {
							Pieces[captured] ^= to;
							zobr ^= zobrist::keys[toSq][captured];
							zobr ^= zobrist::keys[toSq-diff][PAWN | color];
							Pieces[PAWN | color] ^= from;
							All_Pieces(color) ^= tf;
							All_Pieces(color ^ 1) ^= to;
							pieceScore += Value::piece[PAWN | color];
							pieceScore += Value::piece[captured];
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, prom), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!

							bestTF = tf;
							bestProm = prom;
						}
					}
					zobr ^= zobrist::keys[toSq][captured];
					zobr ^= zobrist::keys[toSq-diff][PAWN | color];
					Pieces[captured] ^= to;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					Pieces[PAWN | color] ^= from;
					tmp &= tmp - 1;
				}
			}
			pieceScore += Value::piece[captured];
		}
		pieceScore += Value::piece[PAWN | color];
		attacking[0] = attacking[1] = Pieces[PAWN | color];
		if (color==white){
			attacking[0] &= nPinnedPawn | filled::antiDiag[kingAD];
			attacking[0] <<= 7;
			attacking[0] &= notfile0 & notlastRank_w;
			attacking[1] &= nPinnedPawn | filled::mainDiag[kingMD];
			attacking[1] <<= 9;
			attacking[1] &= notfile7 & notlastRank_w;
		} else {
			attacking[0] &= nPinnedPawn | filled::mainDiag[kingMD];
			attacking[0] >>= 9;
			attacking[0] &= notfile0 & notlastRank_b;
			attacking[1] &= nPinnedPawn | filled::antiDiag[kingAD];
			attacking[1] >>= 7;
			attacking[1] &= notfile7 & notlastRank_b;
		}
		for (int captured = QUEEN | (color^1); captured >= 0 ; captured-=2){
			pieceScore -= Value::piece[captured];
			for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
				tmp = attacking[at] & Pieces[captured];
				while (tmp){
					to = tmp & -tmp;
					if (color == white){
						from = to >> diff;
					} else {
						from = to << -diff;
					}
					tf = to | from;
					square(&toSq, to);
					toggle = zobrist::keys[toSq][PAWN | color];
					toggle ^= zobrist::keys[toSq-diff][PAWN | color];
					toggle ^= zobrist::keys[toSq][captured];
					Pieces[PAWN | color] ^= tf;
					Pieces[captured] ^= to;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[PAWN | color] ^= tf;
					Pieces[captured] ^= to;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					if( score >= beta ) {
						pieceScore += Value::piece[captured];

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
						bestProm = PAWN | color;
					}
					tmp &= tmp - 1;
				}
			}
			pieceScore += Value::piece[captured];
		}
		for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
			if ((attacking[at] & tmpEnPassant) != 0){
				tf = tmpEnPassant;
				bitboard cp = tmpEnPassant;
				if (color == white){
					tf |= tmpEnPassant >> diff;
					cp >>= 8;
				} else {
					tf |= tmpEnPassant << -diff;
					cp <<= 8;
				}
				square(&toSq, tmpEnPassant);
				toggle = zobrist::keys[toSq][PAWN | color];
				toggle ^= zobrist::keys[toSq-diff][PAWN | color];
				toggle ^= zobrist::keys[toSq+(color==white)?-8:8][PAWN | (color ^ 1)];
				pieceScore -= Value::piece[PAWN | (color ^ 1)];
				Pieces[PAWN | color] ^= tf;
				Pieces[PAWN | (color^1)] ^= cp;
				All_Pieces(color) ^= tf;
				All_Pieces(color ^ 1) ^= cp;
				zobr ^= toggle;
				searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
				zobr ^= toggle;
				Pieces[PAWN | color] ^= tf;
				Pieces[PAWN | (color^1)] ^= cp;
				All_Pieces(color) ^= tf;
				All_Pieces(color ^ 1) ^= cp;
				pieceScore += Value::piece[PAWN | (color ^ 1)];
				if( score >= beta ) {
					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
					if (color==black) --fullmoves;
					addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color | TTMove_EnPassantPromFlag), beta);
					statistics(++betaCutOff);
					return beta;			// fail-hard beta-cutoff
				}
				pvFound = true;
				if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
					alpha = score;	//Better move found!
					bestTF = tf;
					bestProm = PAWN | color | TTMove_EnPassantPromFlag;
				}
			}
		}
		bitboard empty = ~occ;
		bitboard pawnsToForward = Pieces[PAWN | color];
		if (mode < quiescenceMask){
			pawnsToForward &= nPinnedPawn | filled::file[kingFL];
			tmp = pawnsToForward;
			if (color == white){
				tmp = lastRank_w & (tmp << 8);
			} else {
				tmp = lastRank_b & (tmp >> 8);
			}
			tmp &= empty;
			pieceScore -= Value::piece[PAWN | color];
			while (tmp){
				to = tmp & -tmp;
				from = to;
				if (color == white){
					from >>= 8;
				} else {
					from <<= 8;
				}
				tf = to | from;
				square(&toSq, to);
				All_Pieces(color) ^= tf;
				Pieces[PAWN | color] ^= from;
				zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
				for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
					Pieces[prom] ^= to;
					pieceScore += Value::piece[prom];
					zobr ^= zobrist::keys[toSq][prom];
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= zobrist::keys[toSq][prom];
					pieceScore -= Value::piece[prom];
					Pieces[prom] ^= to;
					if( score >= beta ) {
						pieceScore += Value::piece[PAWN | color];
						Pieces[PAWN | color] ^= from;
						All_Pieces(color) ^= tf;
						zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, prom), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
						bestProm = prom;
					}
				}
				zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
				Pieces[PAWN | color] ^= from;
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			pieceScore += Value::piece[PAWN | color];
		}
#ifdef HYPERPOSITION
#define SIZE 64
#else
#define SIZE 16
#endif
		bitboard attack[SIZE], frombb[SIZE], xRay;
		unsigned long int piecet[SIZE], fromSq[SIZE], n(0);
#undef SIZE
		int oldKSq = kingSq[color], dr;
		bitboard KAttack = KingMoves[kingSq[color]];
		tmp = Pieces[KNIGHT | color];
		while (tmp){
			frombb[n] = tmp & -tmp;
			square(fromSq+n, frombb[n]);
			dr = direction[kingSq[color]][fromSq[n]];
			//A pinned knight has no legal moves.
			if (dr == WRONG_PIECE || (rays[kingSq[color]][fromSq[n]] & occ) != 0 ||
					(((xRay = getChecker<color>(occ, fromSq[n])) & Pieces[QUEEN | (color ^ 1)]) == 0
							&& (xRay & Pieces[dr | (color ^ 1)]) == 0)) {
				attack[n] = KnightMoves[fromSq[n]];
				piecet[n] = KNIGHT | color;
				++n;
			}
			tmp &= tmp - 1;
		}
		//A pinned piece can only move between the pinner and the king
		tmp = Pieces[BISHOP | color];
		while (tmp){
			frombb[n] = tmp & -tmp;
			square(fromSq+n, frombb[n]);
			attack[n] = bishopAttacks(occ, fromSq[n]);
			piecet[n] = BISHOP | color;
			filterAttackBB<color>(occ, fromSq[n], attack[n]);
			++n;
			tmp &= tmp - 1;
		}
		unsigned long int firstRook = n;
		tmp = Pieces[ROOK | color];
		while (tmp){
			frombb[n] = tmp & -tmp;
			square(fromSq+n, frombb[n]);
			attack[n] = rookAttacks(occ, fromSq[n]);
			piecet[n] = ROOK | color;
			filterAttackBB<color>(occ, fromSq[n], attack[n]);
			++n;
			tmp &= tmp - 1;
		}
		unsigned long int firstQueen = n;
		tmp = Pieces[QUEEN | color];
		while (tmp){
			frombb[n] = tmp & -tmp;
			square(fromSq+n, frombb[n]);
			attack[n] = queenAttacks(occ, fromSq[n]);
			piecet[n] = QUEEN | color;
			filterAttackBB<color>(occ, fromSq[n], attack[n]);
			++n;
			tmp &= tmp - 1;
		}
		//frombb[n] = Pieces[KING | color];
		//fromSq[n] = kingSq[color];//square(frombb[n]);
		//attack[n] = KingMoves[kingSq[color]]; KAttack
		//n : position of last bitboard generated
		if ((castling & castlingrights[color]) == 0){
			for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
				pieceScore -= Value::piece[captured];
				for (unsigned long int i = 0 ; i < n ; ++i) {
					tmp = Pieces[captured] & attack[i];
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						Pieces[captured] ^= to;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						square(&toSq, to);
						toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						Pieces[piecet[i]] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[piecet[i]] ^= tf;
						Pieces[captured] ^= to;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						if( score >= beta ) {
							pieceScore += Value::piece[captured];
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				tmp = Pieces[captured] & KAttack;
				while (tmp){
					to = tmp & -tmp;
					square(kingSq+color, to);
					tf = to | Pieces[KING | color];
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					if (validPosition<color>()){
						toggle = zobrist::keys[kingSq[color]][captured];
						toggle ^= zobrist::keys[kingSq[color]][KING | color];
						toggle ^= zobrist::keys[oldKSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[captured] ^= to;
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;
							All_Pieces(color ^ 1) ^= to;
							pieceScore += Value::piece[captured];

							kingSq[color] = oldKSq;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					tmp &= tmp - 1;
				}
				kingSq[color] = oldKSq;
				pieceScore += Value::piece[captured];
			}

			halfmoves = oldhm + 1;
			if (mode < quiescenceMask){
				//normal moves of non-Pawns
				for (unsigned long int i = 0 ; i < n ; ++i) {
					tmp = attack[i] & empty;
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						All_Pieces(color) ^= tf;
						Pieces[piecet[i]] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						if( score >= beta ) {
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				tmp = KAttack & empty;
				while (tmp){
					to = tmp & -tmp;
					square(kingSq+color, to);
					tf = to | Pieces[KING | color];
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					if (validPosition<color>()){
						toggle = zobrist::keys[kingSq[color]][KING | color];
						toggle ^= zobrist::keys[oldKSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;

							kingSq[color] = oldKSq;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				kingSq[color] = oldKSq;
			}
		} else {
			bitboard oldcastling = castling;
			key ct = zobrist::castling[(castling*castlingsmagic)>>59];
			key ct2;
			unsigned long int i = 0;
			for (int captured = QUEEN | (color ^ 1); captured >= 0 ; captured -= 2){
				pieceScore -= Value::piece[captured];
				for (i = 0; i < firstRook ; ++i) {
					tmp = Pieces[captured] & attack[i];
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						zobr ^= toggle;
						Pieces[captured] ^= to;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[captured] ^= to;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						if( score >= beta ) {
							pieceScore += Value::piece[captured];

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				zobr ^= ct;
				for ( ; i < firstQueen ; ++i){
					castling &= ~frombb[i];
					ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
					zobr ^= ct2;
					tmp = Pieces[captured] & attack[i];
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][ROOK | color];
						toggle ^= zobrist::keys[fromSq[i]][ROOK | color];
						Pieces[captured] ^= to;
						Pieces[ROOK | color] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[captured] ^= to;
						Pieces[ROOK | color] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						if( score >= beta ) {
							zobr ^= ct2;
							zobr ^= ct;
							castling = oldcastling;
							pieceScore += Value::piece[captured];

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
					zobr ^= ct2;
					castling = oldcastling;
				}
				zobr ^= ct;
				for (; i < n ; ++i) {
					tmp = Pieces[captured] & attack[i];
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][QUEEN | color];
						toggle ^= zobrist::keys[fromSq[i]][QUEEN | color];
						Pieces[captured] ^= to;
						Pieces[QUEEN | color] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[captured] ^= to;
						Pieces[QUEEN | color] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						if( score >= beta ) {
							pieceScore += Value::piece[captured];
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				castling &= castlingc<color>::deactrights;
				ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
				zobr ^= ct;
				zobr ^= ct2;
				tmp = Pieces[captured] & KAttack;
				while (tmp){
					to = tmp & -tmp;
					square(kingSq+color, to);
					tf = to | Pieces[KING | color];
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					if (validPosition<color>()){
						toggle = zobrist::keys[kingSq[color]][captured];
						toggle ^= zobrist::keys[kingSq[color]][KING | color];
						toggle ^= zobrist::keys[oldKSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[captured] ^= to;
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;
							All_Pieces(color ^ 1) ^= to;
							castling = oldcastling;
							zobr ^= ct;
							zobr ^= ct2;
							pieceScore += Value::piece[captured];

							kingSq[color] = oldKSq;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					tmp &= tmp - 1;
				}
				kingSq[color] = oldKSq;
				castling = oldcastling;
				zobr ^= ct;
				zobr ^= ct2;
				pieceScore += Value::piece[captured];
			}
			halfmoves = oldhm + 1;
			if (mode < quiescenceMask){
				if ((castling & (castlingc<color>::KingSide) & Pieces[ROOK | color])!=0 && (castlingc<color>::KingSideSpace & occ)==0 && notAttacked<color^1>(castlingc<color>::KSCPassing, castlingc<color>::KSCPassingSq) && validPosition<color>()){
					Pieces[KING | color] ^= castlingc<color>::KSCKT;
					All_Pieces(color) ^= castlingc<color>::KSCFT;
					kingSq[color] = castlingc<color>::kingSqAfterKSC;
					if (validPosition<color>()){
						castling &= castlingc<color>::deactrights;
						toggle = zobrist::keys[0][ROOK | color];
						toggle ^= zobrist::keys[2][ROOK | color];
						toggle ^= zobrist::keys[3][KING | color];
						toggle ^= zobrist::keys[1][KING | color];
						toggle ^= ct;
						toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
						Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
						castling = oldcastling;
						if( score >= beta ) {
							Pieces[KING | color] ^= castlingc<color>::KSCKT;
							All_Pieces(color) ^= castlingc<color>::KSCFT;

							kingSq[color] = castlingc<color>::kingSqBefore;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(castlingc<color>::KSCKT, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = castlingc<color>::KSCKT;
						}
					}
					kingSq[color] = castlingc<color>::kingSqBefore;
					Pieces[KING | color] ^= castlingc<color>::KSCKT;
					All_Pieces(color) ^= castlingc<color>::KSCFT;
				}
				if ((castling & (castlingc<color>::QueenSide) & Pieces[ROOK | color])!=0 && (castlingc<color>::QueenSideSpace & occ)==0 && notAttacked<color^1>(castlingc<color>::QSCPassing, castlingc<color>::QSCPassingSq) && validPosition<color>()){
					Pieces[KING | color] ^= castlingc<color>::QSCKT;
					All_Pieces(color) ^= castlingc<color>::QSCFT;
					kingSq[color] = castlingc<color>::kingSqAfterQSC;
					if (validPosition<color>()){
						castling &= castlingc<color>::deactrights;
						toggle = zobrist::keys[7][ROOK | color];
						toggle ^= zobrist::keys[4][ROOK | color];
						toggle ^= zobrist::keys[3][KING | color];
						toggle ^= zobrist::keys[5][KING | color];
						toggle ^= ct;
						toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
						Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
						castling = oldcastling;
						if( score >= beta ) {
							Pieces[KING | color] ^= castlingc<color>::QSCKT;
							All_Pieces(color) ^= castlingc<color>::QSCFT;

							kingSq[color] = castlingc<color>::kingSqBefore;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(castlingc<color>::QSCKT, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = castlingc<color>::QSCKT;
						}
					}
					kingSq[color] = castlingc<color>::kingSqBefore;
					Pieces[KING | color] ^= castlingc<color>::QSCKT;
					All_Pieces(color) ^= castlingc<color>::QSCFT;
				}

				for (i = 0; i < firstRook ; ++i) {
					tmp = attack[i] & empty;
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						All_Pieces(color) ^= tf;
						Pieces[piecet[i]] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						if( score >= beta ) {
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				zobr ^= ct;
				for (; i < firstQueen ; ++i) {
					castling &= ~frombb[i];
					ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
					zobr ^= ct2;
					tmp = attack[i] & empty;
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][ROOK | color];
						toggle ^= zobrist::keys[fromSq[i]][ROOK | color];
						All_Pieces(color) ^= tf;
						Pieces[ROOK | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[ROOK | color] ^= tf;
						All_Pieces(color) ^= tf;
						if( score >= beta ) {
							zobr ^= ct2;
							zobr ^= ct;
							castling = oldcastling;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp-1;
					}
					zobr ^= ct2;
					castling = oldcastling;
				}
				zobr ^= ct;
				for (; i < n ; ++i) {
					tmp = attack[i] & empty;
					while (tmp){
						to = tmp & -tmp;
						tf = to | frombb[i];
						square(&toSq, to);
						toggle = zobrist::keys[toSq][QUEEN | color];
						toggle ^= zobrist::keys[fromSq[i]][QUEEN | color];
						All_Pieces(color) ^= tf;
						Pieces[QUEEN | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[QUEEN | color] ^= tf;
						All_Pieces(color) ^= tf;
						if( score >= beta ) {
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
						tmp &= tmp - 1;
					}
				}
				castling &= castlingc<color>::deactrights;
				ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
				zobr ^= ct;
				zobr ^= ct2;
				tmp = KAttack & empty;
				while (tmp){
					to = tmp & -tmp;
					square(kingSq+color, to);
					tf = to | Pieces[KING | color];
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					if (validPosition<color>()){
						toggle = zobrist::keys[kingSq[color]][KING | color];
						toggle ^= zobrist::keys[oldKSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;

							castling = oldcastling;
							zobr ^= ct;
							zobr ^= ct2;

							kingSq[color] = oldKSq;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				kingSq[color] = oldKSq;
				castling = oldcastling;
				zobr ^= ct;
				zobr ^= ct2;
			}
		}
		if (mode < quiescenceMask){
			halfmoves = 0;

			tmp = pawnsToForward;
			if (color == white){
				tmp <<= 8;
				tmp &= notlastRank_w;
			} else {
				tmp >>= 8;
				tmp &= notlastRank_b;
			}
			tmp &= empty;
			while (tmp){
				to = tmp & -tmp;
				if (color == white){
					tf = to | (to >> 8);
				} else {
					tf = to | (to << 8);
				}
				square(&toSq, to);
				toggle = zobrist::keys[toSq][PAWN | color];
				toggle ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
				All_Pieces(color) ^= tf;
				Pieces[PAWN | color] ^= tf;
				zobr ^= toggle;
				searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
				zobr ^= toggle;
				Pieces[PAWN | color] ^= tf;
				All_Pieces(color) ^= tf;
				if( score >= beta ) {
					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
					if (color==black) --fullmoves;
					addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
					statistics(++betaCutOff);
					return beta;			// fail-hard beta-cutoff
				}
				pvFound = true;
				if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
					alpha = score;	//Better move found!
					bestTF = tf;
					bestProm = PAWN | color;
				}
				tmp &= tmp - 1;
			}
			tmp = pawnsToForward;
			if (color == white){
				tmp = ( ( ( ( tmp & pstartRank_w ) << 8 ) & empty ) << 8 ) & empty;
			} else {
				tmp = ( ( ( ( tmp & pstartRank_b ) >> 8 ) & empty ) >> 8 ) & empty;
			}
			while (tmp != 0){
				to = tmp & -tmp;
				if (color == white){
					tf = to | (to >> 16);
					enPassant = to >> 8;
				} else {
					tf = to | (to << 16);
					enPassant = to << 8;
				}
				square(&toSq, to);
				square(&tmpSq, enPassant);
				toggle = zobrist::keys[toSq][PAWN | color];
				toggle ^= zobrist::keys[toSq+((color==white)?-16:16)][PAWN | color];
				toggle ^= zobrist::enPassant[7&tmpSq];
				All_Pieces(color) ^= tf;
				Pieces[PAWN | color] ^= tf;
				zobr ^= toggle;
				searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
				zobr ^= toggle;
				Pieces[PAWN | color] ^= tf;
				All_Pieces(color) ^= tf;
				if( score >= beta ) {
					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
					if (color==black) --fullmoves;
					addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
					statistics(++betaCutOff);
					return beta;			// fail-hard beta-cutoff
				}
				pvFound = true;
				if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
					alpha = score;	//Better move found!
					bestTF = tf;
					bestProm = PAWN | color;
				}
				tmp &= tmp - 1;
			}
			enPassant = bitboard(0);
		}
	} else {
		unsigned long int fromSq;
		if (( checkedBy & (checkedBy - 1) ) == bitboard(0)){
			//1) Capturing the attacking piece
			square(&toSq, checkedBy);
			bitboard att;
			int attacker = QUEEN | (color ^ 1);
			while ((Pieces[attacker] & checkedBy)==0) attacker -= 2;
			zobr ^= zobrist::keys[toSq][attacker];
			Pieces[attacker] ^= checkedBy;
			All_Pieces(color ^ 1) ^= checkedBy;
			pieceScore -= Value::piece[attacker];
			if ((checkedBy & (color==white?lastRank_w:lastRank_b)) == 0){
				for (int diff = (color==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
					if (color == white){
						att = (checkedBy >> diff);
					} else {
						att = (checkedBy << -diff);
					}
					att &= notFilled::file[f] & Pieces[PAWN | color];
					if (att != bitboard(0)){
						tf = checkedBy | (att & -att);
						All_Pieces(color) ^= tf;
						if (validPositionNonChecked<color>()){
							toggle = zobrist::keys[toSq][PAWN | color];
							toggle ^= zobrist::keys[toSq - diff][PAWN | color];
							Pieces[PAWN | color] ^= tf;
							zobr ^= toggle;
							searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
							zobr ^= toggle;
							Pieces[PAWN | color] ^= tf;
							if( score >= beta ) {
								zobr ^= zobrist::keys[toSq][attacker];
								Pieces[attacker] ^= checkedBy;
								All_Pieces(color ^ 1) ^= checkedBy;
								pieceScore += Value::piece[attacker];

								All_Pieces(color) ^= tf;

								halfmoves = oldhm;
								zobr ^= zobrist::blackKey;
								enPassant = tmpEnPassant;
								if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
								if (color==black) --fullmoves;
								addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
								statistics(++betaCutOff);
								return beta;			// fail-hard beta-cutoff
							}
							pvFound = true;
							if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
								alpha = score;	//Better move found!
								bestTF = tf;
								bestProm = PAWN | color;
							}
						}
						All_Pieces(color) ^= tf;
					}
				}
				if (((color==white)?(checkedBy<<8):(checkedBy>>8)) == tmpEnPassant){
					for (int diff = (color==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
						if (color == white){
							att = (tmpEnPassant >> diff);
						} else {
							att = (tmpEnPassant << -diff);
						}
						att &=  notFilled::file[f] & Pieces[PAWN | color];
						if (att != bitboard(0)){
							tf = tmpEnPassant | (att & -att);
							All_Pieces(color) ^= tf;
							if (validPositionNonChecked<color>()){
								unsigned long int toenpsq;
								square(&toenpsq, tmpEnPassant);
								toggle = zobrist::keys[toenpsq][PAWN | color];
								toggle ^= zobrist::keys[toenpsq - diff][PAWN | color];
								Pieces[PAWN | color] ^= tf;
								zobr ^= toggle;
								searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
								zobr ^= toggle;
								Pieces[PAWN | color] ^= tf;
								if( score >= beta ) {
									zobr ^= zobrist::keys[toSq][attacker];
									Pieces[attacker] ^= checkedBy;
									All_Pieces(color ^ 1) ^= checkedBy;
									pieceScore += Value::piece[attacker];

									All_Pieces(color) ^= tf;

									halfmoves = oldhm;
									zobr ^= zobrist::blackKey;
									enPassant = tmpEnPassant;
									if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
									if (color==black) --fullmoves;
									addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color | TTMove_EnPassantPromFlag), beta);
									statistics(++betaCutOff);
									return beta;			// fail-hard beta-cutoff
								}
								pvFound = true;
								if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
									alpha = score;	//Better move found!
									bestTF = tf;
									bestProm = PAWN | color | TTMove_EnPassantPromFlag;
								}
							}
							All_Pieces(color) ^= tf;
						}
					}
				}
			} else {
				for (int diff = (color==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
					if (color == white){
						att = (checkedBy >> diff);
					} else {
						att = (checkedBy << -diff);
					}
					att &=  notFilled::file[f] & Pieces[PAWN | color];
					if (att != bitboard(0)){
						from = att & -att;
						tf = checkedBy | from;
						All_Pieces(color) ^= tf;
						if (validPositionNonChecked<color>()){
							Pieces[PAWN | color] ^= from;
							pieceScore -= Value::piece[PAWN | color];
							zobr ^= zobrist::keys[toSq - diff][PAWN | color];
							for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
								Pieces[prom] ^= checkedBy;
								pieceScore += Value::piece[prom];
								zobr ^= zobrist::keys[toSq][prom];
								searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
								zobr ^= zobrist::keys[toSq][prom];
								pieceScore -= Value::piece[prom];
								Pieces[prom] ^= checkedBy;
								if( score >= beta ) {
									zobr ^= zobrist::keys[toSq - diff][PAWN | color];
									zobr ^= zobrist::keys[toSq][attacker];
									Pieces[attacker] ^= checkedBy;
									All_Pieces(color ^ 1) ^= checkedBy;
									pieceScore += Value::piece[attacker];

									pieceScore += Value::piece[PAWN | color];
									Pieces[PAWN | color] ^= from;
									All_Pieces(color) ^= tf;

									halfmoves = oldhm;
									zobr ^= zobrist::blackKey;
									enPassant = tmpEnPassant;
									if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
									if (color==black) --fullmoves;
									addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, prom), beta);
									statistics(++betaCutOff);
									return beta;			// fail-hard beta-cutoff
								}
								pvFound = true;
								if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
									alpha = score;	//Better move found!
									bestTF = tf;
									bestProm = prom;
								}
							}
							zobr ^= zobrist::keys[toSq - diff][PAWN | color];
							pieceScore += Value::piece[PAWN | color];
							Pieces[PAWN | color] ^= from;
						}
						All_Pieces(color) ^= tf;
					}
				}
			}
			tmp = Pieces[KNIGHT | color] & KnightMoves[toSq];
			while (tmp){
				from = tmp & -tmp;
				tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()){
					square(&tmpSq, from);
					toggle = zobrist::keys[tmpSq][KNIGHT | color];
					toggle ^= zobrist::keys[toSq][KNIGHT | color];
					Pieces[KNIGHT | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[KNIGHT | color] ^= tf;
					if( score >= beta ) {
						zobr ^= zobrist::keys[toSq][attacker];
						Pieces[attacker] ^= checkedBy;
						All_Pieces(color ^ 1) ^= checkedBy;
						pieceScore += Value::piece[attacker];

						All_Pieces(color) ^= tf;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
					}
				}
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			tmp = Pieces[BISHOP | color] & bishopAttacks(occ, toSq);
			while (tmp){
				from = tmp & -tmp;
				tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()){
					square(&tmpSq, from);
					toggle = zobrist::keys[tmpSq][BISHOP | color];
					toggle ^= zobrist::keys[toSq][BISHOP | color];
					Pieces[BISHOP | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[BISHOP | color] ^= tf;
					if( score >= beta ) {
						zobr ^= zobrist::keys[toSq][attacker];
						Pieces[attacker] ^= checkedBy;
						All_Pieces(color ^ 1) ^= checkedBy;
						pieceScore += Value::piece[attacker];

						All_Pieces(color) ^= tf;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
					}
				}
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			tmp = Pieces[ROOK | color] & rookAttacks(occ, toSq);
			bitboard oldcastling = castling;
			key ct = zobrist::castling[(castling*castlingsmagic)>>59];
			zobr ^= ct;
			while (tmp){
				from = tmp & -tmp;
				tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()){
					castling &= ~from;
					square(&tmpSq, from);
					toggle = zobrist::keys[tmpSq][ROOK | color];
					toggle ^= zobrist::keys[toSq][ROOK | color];
					toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
					Pieces[ROOK | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[ROOK | color] ^= tf;
					castling = oldcastling;
					if( score >= beta ) {
						zobr ^= zobrist::keys[toSq][attacker];
						Pieces[attacker] ^= checkedBy;
						All_Pieces(color ^ 1) ^= checkedBy;
						pieceScore += Value::piece[attacker];

						All_Pieces(color) ^= tf;
						zobr ^= ct;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
					}
				}
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			zobr ^= ct;
			tmp = Pieces[QUEEN | color] & queenAttacks(occ, toSq);
			while (tmp){
				from = tmp & -tmp;
				tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()){
					square(&tmpSq, from);
					toggle = zobrist::keys[tmpSq][QUEEN | color];
					toggle ^= zobrist::keys[toSq][QUEEN | color];
					Pieces[QUEEN | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[QUEEN | color] ^= tf;
					if( score >= beta ) {
						zobr ^= zobrist::keys[toSq][attacker];
						Pieces[attacker] ^= checkedBy;
						All_Pieces(color ^ 1) ^= checkedBy;
						pieceScore += Value::piece[attacker];

						All_Pieces(color) ^= tf;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
					}
				}
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			pieceScore += Value::piece[attacker];
			zobr ^= zobrist::keys[toSq][attacker];
			Pieces[attacker] ^= checkedBy;
			All_Pieces(color ^ 1) ^= checkedBy;
			//2) Block it if it is a ray piece
			//ray is a subset of empty
			square(&tmpSq, Pieces[KING | color]);
			bitboard ray = rays[tmpSq][toSq];
			bitboard tmpP;
			bitboard tmp2 = Pieces[PAWN | color];
			/** A position in which enPassant can block the ray of the attacking piece can not exist
			 * in a real game as there is no ray able to pass from the starting square of the double
			 * forwarded pawn, from the skipped square and from a square where the king may be.
			 * (en Passant is valid only when the double forwarded pawn is the piece checking the king
			 *  which is handled in another part of the search)
			 * Exapmles of positions where the en Passant can block the ray :
			 *
			 * 	WARNING : All this positions are not going to happen in a game, so for not wasting resources
			 * 			enPassant during check will be tried only if <code>HYPERPOSITION<code> is defined
			 * 			during compilation.
			 *
perft fen 1k4q1/8/8/4pP2/2K5/8/8/8 w - e6 0 2 results : D1 6; D2 145; D3 935; D4 22519; D5 134567; D6 3245867; D7 18792920; D8 454985672;
perft fen 1k6/8/1K5r/3PpP2/8/8/8/8 w - e6 0 2 results : D1 7; D2 110; D3 716; D4 11686; D5 82893; D6 1351742; D7 9944735; D8 160935499;
perft fen 8/8/8/8/3pPp2/1k5R/8/1K6 b - e3 0 1 results : D1 7; D2 110; D3 716; D4 11686; D5 82893; D6 1351742; D7 9944735; D8 160935499;
perft fen 8/8/8/2k5/4Pp2/8/8/1K4Q1 b - e3 0 2 results : D1 6; D2 145; D3 935; D4 22519; D5 134567; D6 3245867; D7 18792920; D8 454985672;
			 **/
#ifdef HYPERPOSITION
			if ((ray & tmpEnPassant) != 0){
				toSq = square(tmpEnPassant);
				bitboard attacker, cp = tmpEnPassant;
				if (color == white){
					cp >>= 8;
				} else {
					cp <<= 8;
				}
				for (int diff = ((color==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
					if (color == white){
						attacker = tmpEnPassant >> diff;
					} else {
						attacker = tmpEnPassant << -diff;
					}
					if ((attacker & Pieces[PAWN | color]) != 0){
						tf = tmpEnPassant | attacker;
						Pieces[PAWN | color] ^= tf;
						Pieces[PAWN | (color^1)] ^= cp;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= cp;
						if (validPositionNonChecked<color>()){
							toggle = zobrist::keys[toSq][PAWN | color];
							toggle ^= zobrist::keys[toSq-diff][PAWN | color];
							toggle ^= zobrist::keys[toSq+(color==white)?-8:8][PAWN | (color ^ 1)];
							zobr ^= toggle;
							pieceScore -= Value::piece[PAWN | (color ^ 1)];
							searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
							pieceScore += Value::piece[PAWN | (color ^ 1)];
							zobr ^= toggle;
							if( score >= beta ) {
								Pieces[PAWN | color] ^= tf;
								Pieces[PAWN | (color^1)] ^= cp;
								All_Pieces(color) ^= tf;
								All_Pieces(color ^ 1) ^= cp;

								halfmoves = oldhm;
								zobr ^= zobrist::blackKey;
								enPassant = tmpEnPassant;
								if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
								if (color==black) --fullmoves;
								addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color | TTMove_EnPassantPromFlag), beta);
								statistics(++betaCutOff);
								return beta;			// fail-hard beta-cutoff
							}
							pvFound = true;
							if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
								alpha = score;	//Better move found!
								bestTF = tf;
								bestProm = PAWN | color | TTMove_EnPassantPromFlag;
							}
						}
						Pieces[PAWN | color] ^= tf;
						Pieces[PAWN | (color^1)] ^= cp;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= cp;
					}
				}
			}
#endif
			tmp = ray;
			if (color == white){
				tmp2 <<= 8;
				tmp &= tmp2;
				tmpP = lastRank_w & tmp;
				tmp &= notlastRank_w;
				tmp2 &= ~occ;
				tmp2 &= pstartRank_w << 8;
				tmp2 <<= 8;
			} else {
				tmp2 >>= 8;
				tmp &= tmp2;
				tmpP = lastRank_b & tmp;
				tmp &= notlastRank_b;
				tmp2 &= ~occ;
				tmp2 &= pstartRank_b >> 8;
				tmp2 >>= 8;
			}
			tmp2 &= ray;
			while (tmp2 != 0){
				to = tmp2 & -tmp2;
				if (color == white){
					tf = to | (to >> 16);
					enPassant = to >> 8;
				} else {
					tf = to | (to << 16);
					enPassant = to << 8;
				}
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()) {
					square(&toSq, to);
					square(&tmpSq, enPassant);
					toggle = zobrist::keys[toSq][PAWN | color];
					toggle ^= zobrist::keys[toSq+((color==white)?-16:16)][PAWN | color];
					toggle ^= zobrist::enPassant[7&tmpSq];
					Pieces[PAWN | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[PAWN | color] ^= tf;
					if( score >= beta ) {
						All_Pieces(color) ^= tf;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
						bestProm = PAWN | color;
					}
				}
				All_Pieces(color) ^= tf;
				tmp2 &= tmp2 - 1;
			}
			enPassant = 0;
			while (tmpP != 0){
				to = tmpP & -tmpP;
				if (color == white){
					from = to >> 8;
				} else {
					from = to << 8;
				}
				tf = to | from;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()) {
					square(&toSq, to);
					Pieces[PAWN | color] ^= from;
					pieceScore -= Value::piece[PAWN | color];
					zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
					for (int prom = QUEEN | color; prom > (PAWN | colormask) ; prom -= 2){
						Pieces[prom] ^= to;
						pieceScore += Value::piece[prom];
						zobr ^= zobrist::keys[toSq][prom];
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= zobrist::keys[toSq][prom];
						pieceScore -= Value::piece[prom];
						Pieces[prom] ^= to;
						if( score >= beta ) {
							pieceScore += Value::piece[PAWN | color];
							Pieces[PAWN | color] ^= from;
							zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
							All_Pieces(color) ^= tf;
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, prom), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
							bestProm = prom;
						}
					}
					zobr ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
					pieceScore += Value::piece[PAWN | color];
					Pieces[PAWN | color] ^= from;
				}
				All_Pieces(color) ^= tf;
				tmpP &= tmpP - 1;
			}
			while (tmp != 0){
				to = tmp & -tmp;
				if (color == white){
					tf = to | (to >> 8);
				} else {
					tf = to | (to << 8);
				}
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>()) {
					square(&toSq, to);
					toggle = zobrist::keys[toSq][PAWN | color];
					toggle ^= zobrist::keys[toSq+((color==white)?-8:8)][PAWN | color];
					Pieces[PAWN | color] ^= tf;
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					Pieces[PAWN | color] ^= tf;
					if( score >= beta ) {
						All_Pieces(color) ^= tf;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, PAWN | color), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
						bestProm = PAWN | color;
					}
				}
				All_Pieces(color) ^= tf;
				tmp &= tmp - 1;
			}
			tmpP = Pieces[KNIGHT | color];
			while (tmpP != 0){
				from = tmpP & -tmpP;
				square(&fromSq, from);
				tmp = ray & KnightMoves[fromSq];
				while (tmp != 0){
					to = tmp & -tmp;
					tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>()){
						square(&toSq, to);
						toggle = zobrist::keys[toSq][KNIGHT | color];
						toggle ^= zobrist::keys[fromSq][KNIGHT | color];
						Pieces[KNIGHT | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[KNIGHT | color] ^= tf;
						if( score >= beta ) {
							All_Pieces(color) ^= tf;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				tmpP &= tmpP - 1;
			}
			tmpP = Pieces[BISHOP | color];
			while (tmpP != 0){
				from = tmpP & -tmpP;
				square(&fromSq, from);
				tmp = ray & bishopAttacks(occ, fromSq);
				while (tmp != 0){
					to = tmp & -tmp;
					tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>()){
						square(&toSq, to);
						toggle = zobrist::keys[toSq][BISHOP | color];
						toggle ^= zobrist::keys[fromSq][BISHOP | color];
						Pieces[BISHOP | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[BISHOP | color] ^= tf;
						if( score >= beta ) {
							All_Pieces(color) ^= tf;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				tmpP &= tmpP - 1;
			}
			tmpP = Pieces[ROOK | color];
			//Rooks in corners can not get into ray, so changing castling rights is useless
			//as rooks will never be in a position where they have castling right.
			while (tmpP != 0){
				from = tmpP & -tmpP;
				square(&fromSq, from);
				tmp = ray & rookAttacks(occ, fromSq);
				while (tmp != 0){
					to = tmp & -tmp;
					tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>()){
						square(&toSq, to);
						toggle = zobrist::keys[toSq][ROOK | color];
						toggle ^= zobrist::keys[fromSq][ROOK | color];
						Pieces[ROOK | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[ROOK | color] ^= tf;
						if( score >= beta ) {
							All_Pieces(color) ^= tf;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				tmpP &= tmpP - 1;
			}
			tmpP = Pieces[QUEEN | color];
			while (tmpP != 0){
				from = tmpP & -tmpP;
				square(&fromSq, from);
				tmp = ray & queenAttacks(occ, fromSq);
				while (tmp != 0){
					to = tmp & -tmp;
					tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>()){
						square(&toSq, to);
						toggle = zobrist::keys[toSq][QUEEN | color];
						toggle ^= zobrist::keys[fromSq][QUEEN | color];
						Pieces[QUEEN | color] ^= tf;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						Pieces[QUEEN | color] ^= tf;
						if( score >= beta ) {
							All_Pieces(color) ^= tf;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
							if (color==black) --fullmoves;
							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
							statistics(++betaCutOff);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!
							bestTF = tf;
						}
					}
					All_Pieces(color) ^= tf;
					tmp &= tmp - 1;
				}
				tmpP &= tmpP - 1;
			}
		}
		//3) Move the king
		halfmoves = oldhm + 1;
		from = Pieces[KING | color];
		square(&fromSq, from);
		bitboard mv = KingMoves[fromSq];
		bitboard tmp1 = mv;
		bitboard oldcastling = castling;
		key ct = zobrist::castling[(castling*castlingsmagic)>>59];
		castling &= castlingc<color>::deactrights;
		ct ^= zobrist::castling[(castling*castlingsmagic)>>59];
		zobr ^= ct;
		for (int attacker = QUEEN | (color ^ 1); attacker >= 0 ; attacker -= 2){
			tmp = Pieces[attacker] & tmp1;
			while (tmp != 0){
				to = tmp & -tmp;
				square(kingSq + color, to);
				tf = from | to;
				Pieces[attacker] ^= to;
				All_Pieces(color ^ 1) ^= to;
				Pieces[KING | color] ^= tf;
				All_Pieces(color) ^= tf;
				if (validPosition<color>()){
					toggle = zobrist::keys[fromSq][KING | color];
					toggle ^= zobrist::keys[kingSq[color]][KING | color];
					toggle ^= zobrist::keys[kingSq[color]][attacker];
					pieceScore -= Value::piece[attacker];
					zobr ^= toggle;
					searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
					zobr ^= toggle;
					pieceScore += Value::piece[attacker];
					if( score >= beta ) {
						Pieces[attacker] ^= to;
						All_Pieces(color ^ 1) ^= to;
						Pieces[KING | color] ^= tf;
						All_Pieces(color) ^= tf;

						castling = oldcastling;
						zobr ^= ct;

						kingSq[color] = fromSq;

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
						if (color==black) --fullmoves;
						addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
						statistics(++betaCutOff);
						return beta;			// fail-hard beta-cutoff
					}
					pvFound = true;
					if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
						alpha = score;	//Better move found!
						bestTF = tf;
					}
				}
				Pieces[KING | color] ^= tf;
				All_Pieces(color) ^= tf;
				Pieces[attacker] ^= to;
				All_Pieces(color ^ 1) ^= to;
				tmp &= tmp - 1;
			}
		}
		tmp = mv;
		tmp &= ~occ;
		while (tmp != 0){
			to = tmp & -tmp;
			tf = to | from;
			square(kingSq + color, to);
			Pieces[KING | color] ^= tf;
			All_Pieces(color) ^= tf;
			if (validPosition<color>()){
				toggle = zobrist::keys[kingSq[color]][KING | color];
				toggle ^= zobrist::keys[fromSq][KING | color];
				zobr ^= toggle;
				searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
				zobr ^= toggle;
				if( score >= beta ) {
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;

					castling = oldcastling;
					zobr ^= ct;

					kingSq[color] = fromSq;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
					if (color==black) --fullmoves;
					addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<color>(tf, 0), beta);
					statistics(++betaCutOff);
					return beta;			// fail-hard beta-cutoff
				}
				pvFound = true;
				if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
					alpha = score;	//Better move found!
					bestTF = tf;
				}
			}
			Pieces[KING | color] ^= tf;
			All_Pieces(color) ^= tf;
			tmp &= tmp - 1;
		}
		kingSq[color] = fromSq;
		castling = oldcastling;
		zobr ^= ct;
	}
	halfmoves = oldhm;
	zobr ^= zobrist::blackKey;
	enPassant = tmpEnPassant;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&(square( &toSq, enPassant ), toSq)];
	if (color==black) --fullmoves;
	if (mode == Perft && depth == dividedepth) {
		U64 moves = horizonNodes;
		moves -= stHorNodes;
#ifdef DIVIDEPERFT
		DWORD bytes_read, bytes_written;
		CHAR buffer[4096];
		// Write a message to the child process
		WriteFile(child_input_write, ("setboard "+getFEN(color)+"\n").c_str(), ("setboard "+getFEN(color)+"\n").length(), &bytes_written, NULL);
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
		sscanf(buffer, "Nodes: %u,", &a);
		if (a != moves) {
			std::cout << pre << getFEN(color) << '\t' << moves << "\tFailed!\t" << a << '\n';
			std::cout << "-----------------------------------------\n";
			std::string oldpre = pre;
			pre += "\t";
			dividedepth = depth-1;
			search<mode, color>(alpha, beta, depth);
			std::cout << "-----------------------------------------" << std::endl;
			pre = oldpre;
			dividedepth = depth;
		}
#else
		std::cout << pre << getFEN(color) << '\t' << moves << std::endl;
#endif
	}
	if (mode == Perft) return alpha + 1;
	if (!boost::this_thread::interruption_requested()){
		if (mode < quiescenceMask){
			if (stNodes == nodes){
				if (checkedBy == bitboard(0)) {
					alpha = 0;								//PAT
				} else {
					alpha = -Value::MAT+rootDepth-depth;	//MATed
				}
				addTTEntry<ExactScore>(zobr, depth, 0, alpha);//ExactScore
			} else if (alpha != startingAlpha) {
				assert(bestTF != bitboard(0));
				addTTEntry<AlphaCutoff>(zobr, depth, getMove<color>(bestTF, bestProm), alpha);
			} else {
				assert(bestTF == bitboard(0));//ExactScore
				addTTEntry<mode == ZW ? AlphaCutoff : ExactScore>(zobr, depth, (bestTF == bitboard(0)) ? ttResult : getMove<color>(bestTF, bestProm), alpha);
			}
		} else {
			addTTEntry<QSearchAlphaCutoff>(zobr, depth, (bestTF == bitboard(0)) ? ttResult : getMove<color>(bestTF, bestProm), alpha);
		}
	}
	return alpha;
}

template<int color> inline int Board::getMove(bitboard tf, int prom){
	unsigned long int fromSq, toSq;
	assert(tf != bitboard(0));
	assert(prom < (TTMove_EnPassantPromFlag << 1));
	assert((tf & All_Pieces(color)) != bitboard(0));
	assert((tf & (~All_Pieces(color))) != bitboard(0));
	square(&fromSq, tf & All_Pieces(color));
	square(&toSq, tf & (~All_Pieces(color)));
	assert(0 <= fromSq && fromSq < 64);
	assert(0 <= toSq && toSq < 64);
	if (tf & Pieces[PAWN | color]) return getTTMoveFormat(fromSq, toSq, prom);
	return getTTMoveFormat(fromSq, toSq, 0);
}

template<int color> bool Board::stalemate(){
	bitboard occ = All_Pieces(white);
	occ |= All_Pieces(black);
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
	unsigned long int fromSq, tsq;
	square(&fromSq, from);
	bitboard att = KingMoves[fromSq];
	moving = empty & att;
	occ ^= from;
	while (moving != 0){
		to = moving & -moving;
		square(&tsq, to);
		if (notAttacked<color^1>(to, occ^to, tsq)) {
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
			square(&tsq, to);
			res = notAttacked<color^1>(to, occ, tsq);
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
		square(&fromSq, from);
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
		temp &= temp - 1;
	}
	temp = Pieces[BISHOP | color];
	while (temp != 0){
		from = temp & -temp;
		square(&fromSq, from);
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
		temp &= temp - 1;
	}
	temp = Pieces[ROOK | color];
	while (temp != 0){
		from = temp & -temp;
		square(&fromSq, from);
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
		temp &= temp - 1;
	}
	temp = Pieces[QUEEN | color];
	while (temp != 0){
		from = temp & -temp;
		square(&fromSq, from);
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
		temp &= temp - 1;
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
			moving &= moving - 1;
		}
	}
	//If castling was available, King would had a normal move as well!
	return true;
}

template<int color> int Board::getEvaluation(int depth){
	if (stalemate<color>()){
		if (validPosition<color>()) return 0;						//stalemate
		if (color == white) return -Value::MAT+rootDepth-depth;	//White Mated
		return Value::MAT-rootDepth+depth;						//Black Mated
	}
	int score = pieceScore;
	score += Value::kingSq[kingSq[white]] - Value::kingSq[kingSq[black]];
	bitboard knights = Pieces[KNIGHT | white];
	while (knights) {
		unsigned long int sq;
		square(&sq, knights & -knights);
		score += Value::knightSq[sq];
		knights &= knights-1;
	}
	knights = Pieces[KNIGHT | black];
	while (knights) {
		unsigned long int sq;
		square(&sq, knights & -knights);
		score -= Value::knightSq[sq];
		knights &= knights-1;
	}
	bitboard pawns = Pieces[PAWN | white];
	//bitboard emptyFiles = pawns;
	while (pawns){
		unsigned long int sq;
		square(&sq, pawns & -pawns);
		score += Value::WpawnSq[sq];
		pawns &= pawns-1;
	}
	pawns = Pieces[PAWN | black];
	while (pawns){
		unsigned long int sq;
		square(&sq, pawns & -pawns);
		score -= Value::BpawnSq[sq];
		pawns &= pawns-1;
	}
	/**emptyFiles |= emptyFiles << 8;
	emptyFiles |= emptyFiles << 16;
	emptyFiles |= emptyFiles << 32;
	emptyFiles |= emptyFiles >> 8;
	emptyFiles |= emptyFiles >> 16;
	emptyFiles |= emptyFiles >> 32;
	emptyFiles = ~emptyFiles;**/

	return score;
}

inline bitboard Board::bishopAttacks(bitboard occ, const int &sq){
	occ &= BishopMask[sq];
	occ *= BishopMagic[sq];
#ifndef fixedShift
	occ >>= BishopShift[sq];
#else
	occ >>= 64-maxBishopBits;
#endif
	return BishopAttacks[sq][occ];
}

inline bitboard Board::rookAttacks(bitboard occ, const int &sq){
	occ &= RookMask[sq];
	occ *= RookMagic[sq];
#ifndef fixedShift
	occ >>= RookShift[sq];
#else
	occ >>= 64-maxRookBits;
#endif
	return RookAttacks[sq][occ];
}

inline bitboard Board::queenAttacks(bitboard occ, const int &sq){
	return rookAttacks(occ, sq) | bishopAttacks(occ, sq);
}

template<int color> inline void Board::filterAttackBB(bitboard &occ, unsigned long int &sq, bitboard &attack){
	int dr = direction[kingSq[color]][sq];
	bitboard ray = rays[kingSq[color]][sq];
	if (dr != WRONG_PIECE && (ray & occ) == bitboard(0)){
		bitboard xRay = XRayOFCMask[kingSq[color]][sq];
		bitboard ray2 = xRay;
		xRay &= occ;
		xRay *= XRayOFCMagic[kingSq[color]][sq];
		xRay >>= 64 - maxCheckAvoidanceShiftBits;
		xRay = XRayOFCChecker[kingSq[color]][sq][xRay];
		if ((xRay & Pieces[QUEEN | (color ^ 1)]) != 0 || (xRay & Pieces[dr | (color ^ 1)]) != 0) attack &= ray | ray2;
	}
}

template<int color> inline bitboard Board::getNPinnedPawns(bitboard occ){
	bitboard pinnedPawns = bitboard(0);
	bitboard tmp = rookAttacks(occ & ~rookAttacks(occ, kingSq[color]), kingSq[color]);
	tmp &= Pieces[ROOK | (color ^ 1)] | Pieces[QUEEN | (color ^ 1)];
	unsigned long int sq;
	while (tmp){
		square(&sq, tmp);
		pinnedPawns |= rays[kingSq[color]][sq];
		tmp &= tmp - 1;
	}
	tmp = bishopAttacks(occ & ~bishopAttacks(occ, kingSq[color]), kingSq[color]);
	tmp &= Pieces[BISHOP | (color ^ 1)] | Pieces[QUEEN | (color ^ 1)];
	while (tmp){
		square(&sq, tmp);
		pinnedPawns |= rays[kingSq[color]][sq];
		tmp &= tmp - 1;
	}
	pinnedPawns &= Pieces[PAWN | color];
	return (~pinnedPawns);
}

template<int color> inline bitboard Board::getChecker(bitboard occ, unsigned long int &sq){
	occ &= XRayOFCMask[kingSq[color]][sq];
	occ *= XRayOFCMagic[kingSq[color]][sq];
	occ >>= 64 - maxCheckAvoidanceShiftBits;
	return XRayOFCChecker[kingSq[color]][sq][occ];
}
#endif /* BOARD_H_ */
