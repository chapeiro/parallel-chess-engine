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
#include "Utilities.h"
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

#if defined _MSC_VER && _MSC_VER <= 1600
#include <boost/thread/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/microsec_time_clock.hpp"
#else
#include <thread>
#include <chrono>
#endif

#include "TranspositionTable.h"
#include <iomanip>

//#define NO_KILLER_MOVE

#ifdef DIVIDEPERFT
#include <windows.h>
#endif

#ifndef NDEBUG
#define ASSUME(x) assert(x)
#else
#ifdef _MSC_VER
#define ASSUME(x) __assume(x)
#else
#define ASSUME(x) ((void) 0)
#endif
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

#if defined _MSC_VER && _MSC_VER <= 1600
typedef boost::thread thread;
#define chrono boost::posix_time;
typedef boost::posix_time::ptime time_td;
typedef boost::posix_time::time_duration time_duration;
#else
typedef std::chrono::high_resolution_clock::time_point time_td;
typedef std::chrono::high_resolution_clock clock_ns;
typedef std::chrono::nanoseconds time_duration;
typedef std::thread thread;
#endif

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
		//unsigned long int kingSq[colormask + 1];
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
		thread *searchThread;
		bool interruption_requested;

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
		void printHistory();
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
		void forgetOldHistory();
		template<int color> int getEvaluation(int depth);
		int evaluatePawnStructure();
		template<int color> void deactivateCastlingRights();
		void togglePlaying();
		void startSearch(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch);
		std::string extractPV(int depth);

		template<int color> bool validPosition(int kingSq);
		template<int color> bool validPosition(bitboard occ, int kingSq);
		template<int color> bool validPositionNonChecked(int kingSq);
		template<int color> bool validPositionNonChecked(bitboard occ, int kingSq);
		template<int color> bool notAttacked(bitboard target, int targetSq);
		template<int color> bool notAttacked(bitboard target, bitboard occ, int targetSq);
		template<int color> bitboard kingIsAttackedBy(bitboard occ, int kingSq);

		template<int color> bool stalemate();

		bitboard bishopAttacks(bitboard occ, const int &sq);
		bitboard rookAttacks(bitboard occ, const int &sq);
		bitboard queenAttacks(bitboard occ, const int &sq);
		template<int color> bitboard getChecker(bitboard occ, unsigned long int sq, int kingSq);
		template<int color> void filterAttackBB(bitboard occ, unsigned long int sq, bitboard &attack, int kingSq);
		template<int color> bitboard getNPinnedPawns(bitboard occ, int kingSq);
		template<int color> int getMove(bitboard tf, int prom);
		char * moveToString(int move, char* m);

		template<SearchMode mode, int color, bool root> int search(int alpha, int beta, int depth);
		template<SearchMode mode, int color> void searchDeeper(int alpha, int beta, int depth, bool pvFound, int &score);
		template<int color> int quieSearch(int alpha, int beta);

		bool threefoldRepetition();
};

inline int Board::getPieceIndex(char p){
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A') | black;
	return getWhitePieceIndex(p) | white;
}

inline void Board::updatePieces(int sq, int ind){
	Pieces[ind] ^= filled::normal[sq];
	//if ( (ind & ~colormask) == KING ) kingSq[ind & colormask] = sq;
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

template<int color> bitboard Board::kingIsAttackedBy(bitboard occ, int kingSq){
	ASSUME(kingSq >= 0 && kingSq < 64);
	bitboard attackers = KnightMoves[kingSq];
	attackers &= Pieces[KNIGHT | (color^1)];
	attackers |= rookAttacks(occ, kingSq) & (Pieces[ROOK | (color^1)] | Pieces[QUEEN | (color^1)]);
	attackers |= bishopAttacks(occ, kingSq) & (Pieces[BISHOP | (color^1)] | Pieces[QUEEN | (color^1)]);
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
	ASSUME((target & (target-1))==0);
	ASSUME(targetSq >= 0 && targetSq < 64);
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

template<int color> inline bool Board::validPosition(bitboard occ, int kingSq) {
	ASSUME((Pieces[KING | color] & (Pieces[KING | color]-1)) == 0);
	return notAttacked<color^1>(Pieces[KING | color], occ, kingSq);
}

template<int color> inline bool Board::validPositionNonChecked(bitboard occ, int kingSq) {
	return (((Pieces[BISHOP | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )]) & bishopAttacks(occ, kingSq)) == 0) &&
			(((Pieces[ROOK | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )]) & rookAttacks(occ, kingSq)) == 0);
	/**bitboard att = Pieces[BISHOP | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )];
	if ((att & bishopAttacks(occ, kingSq)) != 0) return false;
	att = Pieces[ROOK | ( color ^ 1 )] | Pieces[QUEEN | ( color ^ 1 )];
	if ((att & rookAttacks(occ, kingSq)) != 0) return false;
	return true;**/
}

template<int color> inline bool Board::validPositionNonChecked(int kingSq) {
	return validPositionNonChecked<color>(All_Pieces(white) | All_Pieces(black), kingSq);
}

/**
 * uses only :
 *  kingSq[color],
 * 	Pieces[ [PAWN ... KING] | color^1 ],
 * 	Pieces[KING | color],
 * 	All_Pieces(white),
 * 	All_Pieces(black)
 */
template<int color> inline bool Board::validPosition(int kingSq) {
	ASSUME((Pieces[KING | color] & (Pieces[KING | color]-1)) == bitboard(0));
	return notAttacked<color^1>(Pieces[KING | color], kingSq);
}

template<SearchMode mode, int color> inline void Board::searchDeeper(int alpha, int beta, int depth, bool pvFound, int &score){
	addToHistory(zobr);
	if (mode >= quiescenceMask){
		score = -search<mode, color, false>(-beta, -alpha, depth - 1);
	} else if (mode == PV){
		if (pvFound) {
			score = -search<ZW, color, false>(-1-alpha, -alpha, depth - 1);
			if ( score > alpha ) score = -search<PV, color, false>(-beta, -alpha, depth - 1);
		} else {
			score = -search<PV, color, false>(-beta, -alpha, depth - 1);
		}
	} else if (mode == ZW){
		score = -search<ZW, color, false>(-beta, -alpha, depth - 1);
	} else {
		score = -search<Perft, color, false>(-beta, -alpha, depth - 1);
	}
	removeLastHistoryEntry();
}

/**
 *
 * @param alpha	lower bound
 * @param beta	lower bound
 * @param depth	number of plies till the horizon, 0 if the horizon has already been reached
 *
 * @return
 **/
template<SearchMode mode, int color, bool root> int Board::search(int alpha, int beta, int depth){
	//FIXME This is saved as a betaCutOff later in the TT!
	if (color==white) if (interruption_requested) return inf; //TODO Revision! does not seem such a good idea :(
	//count nodes searched
	++nodes;
	if ((!root) && (mode != Perft) && (mode < quiescenceMask) && (halfmoves >= 100 || threefoldRepetition())) return 0;
	if (mode >= quiescenceMask){
		//count nodes searched by quiescence
		++qNodes;
		int standPat = getEvaluation<color>(depth);
		if (color == black) standPat = -standPat;
		if (mated(standPat)) return standPat;
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
		//count horizon nodes reached (Useful for Perft)
		++horizonNodes;
		if (mode & Perft) {
			if (dividedepth == 0) std::cout << pre << getFEN(color) << '\n';
			return beta;
		}
		//Horizon has been reached! Start quiescence search
		//return search<QuiescencePV, color>(alpha, beta, depth);
		return search<(SearchMode) (mode | quiescenceMask), color, root>(mode==ZW?beta-1:alpha, beta, depth);
	}
#ifndef NO_TRANSPOSITION_TABLE
	//FIXME
	int killerMove = retrieveTTEntry<mode>(zobr, depth, alpha, beta);
	//if (ttResult == ttExactScoreReturned) return alpha;
	if (alpha >= beta) {
		statistics(++hashHitCutOff);
		return alpha;
	}
#endif

	//move state forward (halfmoves (oldhm), fullmoves (fullmoves+{0, 1}(color)), enPassant (tmpEnPassant)
	unsigned long int enSq;
	int oldhm (halfmoves);
	square( &enSq, enPassant );
	enSq &= 7;
	if (enPassant) zobr ^= zobrist::enPassant[enSq];
	bitboard tmpEnPassant (enPassant);
	fullmoves += color;							//if (color==black) ++fullmoves;
	enPassant = bitboard(0);
	halfmoves = 0;
	zobr ^= zobrist::blackKey;

	U64 stNodes (nodes);
	U64 stHorNodes (horizonNodes);
	int startingAlpha = alpha;

	int score;
	bool pvFound = false;

	bitboard bestTF(0);
	unsigned long int bestProm (0);

	unsigned long int kingSq;
	square(&kingSq, Pieces[KING | color]);
	bitboard oldP[(KING | black)+1];
	for (int i = 0 ; i <= (KING | black) ; ++i) oldP[i] = Pieces[i];
	bitboard all = 0;
	for (int i = PAWN | color ; i <= (KING | color) ; i += 2) all |= Pieces[i];
	ASSUME(All_Pieces(color) == all);
	/**
uci
isready
ucinewgame
isready
position startpos moves g1f3 g8f6 d2d4 d7d5 c2c4 c7c6 e2e3 c8g4 b1c3 e7e6 h2h3 g4h5 g2g4 h5g6 f3e5 b8d7 e5g6 h7g6 d1b3 d8c7 f1g2 d5c4 b3c4 e8c8 g4g5 f6d5 g2d5 e6d5 c4b3 d8e8 e1f1 d7b6 h3h4 f7f6 g5f6 g7f6 h1g1 h8h4 g1g6 c7h7
go infinite
	 */
	all = 0;
	for (int i = PAWN | (color^1) ; i <= (KING | (color^1)) ; i += 2) all |= Pieces[i];
	ASSUME(All_Pieces(color^1) == all);
	const bitboard occ = All_Pieces(white) | All_Pieces(black);
	bitboard oldAll = All_Pieces(color);
#ifndef NO_TRANSPOSITION_TABLE
#ifndef NO_KILLER_MOVE
	if (mode != Perft){
		if (killerMove != NULL_MOVE){
			bitboard tmp = occ;
			int killerFromSq = getTTMove_From(killerMove);
			int killerToSq = getTTMove_To(killerMove);
			bitboard killerFrom = bitboard(1) << killerFromSq;
			bitboard killerTo = bitboard(1) << killerToSq;
			bool killerMoveOk = true;
			if (((killerTo & All_Pieces(color)) == bitboard(0)) && (All_Pieces(color) & killerFrom)){
				if ((Pieces[PAWN | color] & killerFrom) == bitboard(0)){
					int killerPiece = KNIGHT | color;
					while ((Pieces[killerPiece] & killerFrom) == bitboard(0)) killerPiece += 2;
					if ((killerPiece == (KING | color)) && ((KingMoves[killerFromSq] & killerTo) == bitboard(0))){
						halfmoves = oldhm + 1;
						Pieces[KING | color] = killerTo;
						bitboard oldCastling = castling;
						key ct = zobrist::castling[(castling*castlingsmagic)>>59];
						castling &= castlingc<color>::deactrights;
						Zobrist toggle = ct;
						toggle ^= zobrist::keys[3+((color==black)?56:0)][KING | color];
						toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
						if (killerFrom > killerTo) {
							if ((oldCastling & castlingc<color>::KingSide & Pieces[ROOK | color]) && ((castlingc<color>::KingSideSpace & occ) == 0)){
								kingSq = castlingc<color>::kingSqAfterKSC;
								toggle ^= zobrist::keys[0+((color==black)?56:0)][ROOK | color];
								toggle ^= zobrist::keys[2+((color==black)?56:0)][ROOK | color];
								toggle ^= zobrist::keys[1+((color==black)?56:0)][KING | color];
								Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
								All_Pieces(color) ^= castlingc<color>::KSCFT;
								zobr ^= toggle;
								searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
								Pieces[ROOK | color] ^= castlingc<color>::KSCRT;
								All_Pieces(color) ^= castlingc<color>::KSCFT;
							} else {
								statistics(++ttError_Type1_SameHashKey);
								killerMoveOk = false;
							}
						} else {
							if ((oldCastling & castlingc<color>::QueenSide & Pieces[ROOK | color]) && ((castlingc<color>::QueenSideSpace & occ) == 0)){
								kingSq = castlingc<color>::kingSqAfterQSC;
								toggle ^= zobrist::keys[7+((color==black)?56:0)][ROOK | color];
								toggle ^= zobrist::keys[4+((color==black)?56:0)][ROOK | color];
								toggle ^= zobrist::keys[5+((color==black)?56:0)][KING | color];
								Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
								All_Pieces(color) ^= castlingc<color>::QSCFT;
								zobr ^= toggle;
								searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
								Pieces[ROOK | color] ^= castlingc<color>::QSCRT;
								All_Pieces(color) ^= castlingc<color>::QSCFT;
							} else {
								statistics(++ttError_Type1_SameHashKey);
								killerMoveOk = false;
							}
						}
						zobr ^= toggle;
						Pieces[KING | color] = killerFrom;
						castling = oldCastling;
						if (killerMoveOk) {
							if( score >= beta ) {
								halfmoves = oldhm;
								zobr ^= zobrist::blackKey;
								enPassant = tmpEnPassant;
								if (enPassant) zobr ^= zobrist::enPassant[enSq];
								if (color==black) --fullmoves;
								addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, killerMove, beta);
								statistics(++betaCutOff);
								statistics(++cutOffByKillerMove);
								return beta;			// fail-hard beta-cutoff
							}
							pvFound = true;
							if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
								alpha = score;	//Better move found!
								bestTF = (killerFrom > killerTo) ? castlingc<color>::KSCKT : castlingc<color>::QSCKT;
							}
						}
						halfmoves = 0;
						kingSq = castlingc<color>::kingSqBefore;
					} else {
						int capturedPiece = QUEEN | (color ^ 1);
						while (((Pieces[capturedPiece] & killerTo) == 0) && capturedPiece >= 0) capturedPiece -= 2;
						bitboard tf = killerFrom | killerTo;
						Zobrist toggle = zobrist::keys[killerFromSq][killerPiece];
						toggle ^= zobrist::keys[killerToSq][killerPiece];
						bitboard oldCastling = castling;
						if (killerPiece == (ROOK | color)) {
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
							castling &= ~killerFrom;
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
						} else if (killerPiece == (KING | color)){
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
							castling &= castlingc<color>::deactrights;
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>59];
						}
						int scoreD = 0;
						if (capturedPiece >= 0){
							halfmoves = 0;
							toggle ^= zobrist::keys[killerToSq][capturedPiece];
							scoreD = Value::piece[capturedPiece];
							Pieces[capturedPiece] ^= killerTo;
							All_Pieces(color ^ 1) ^= killerTo;
						} else {
							halfmoves = oldhm + 1;
						}
						Pieces[killerPiece] ^= tf;
						All_Pieces(color) ^= tf;
						pieceScore -= scoreD;
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						pieceScore += scoreD;
						Pieces[killerPiece] ^= tf;
						All_Pieces(color) ^= tf;
						if (capturedPiece >= 0){
							Pieces[capturedPiece] ^= killerTo;
							All_Pieces(color ^ 1) ^= killerTo;
						}
						castling = oldCastling;
						if( score >= beta ) {
							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
							if (color==black) --fullmoves;

							addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, killerMove, beta);
							statistics(++betaCutOff);
							statistics(++cutOffByKillerMove);
							return beta;			// fail-hard beta-cutoff
						}
						pvFound = true;
						if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
							alpha = score;	//Better move found!

							bestTF =  tf;
							ASSUME(bestTF & All_Pieces(color)); //FIXME REMOVE
							bestProm = 0;
						}
						halfmoves = 0;
					}
				} else {
					int promSp = getTTMove_Prom_spec(killerMove);
					int toPiece = getTTMove_fromPS_P(promSp);
					if ((toPiece & colormask) == color){
						int diff = killerToSq - killerFromSq;
						bitboard tf = killerFrom ^ killerTo;
						//-7 = 1..11 1001, -8 = 1..11 1000, -9 1..11 0111
						if (diff & 1) {
							int capturedPiece, capturedSq;
							bitboard capturedPos;
							if (promSp <= 0xF) {
								capturedPiece = QUEEN | (color ^ 1);
								while ((capturedPiece >= 0) && ((Pieces[capturedPiece] & killerTo) == 0)) capturedPiece -= 2;
								if (capturedPiece < 0){
									statistics(++ttError_Type1_SameHashKey);
									killerMoveOk = false;
								} else {
									capturedPos = killerTo;
									capturedSq = killerToSq;
								}
							} else if ((tmpEnPassant == ((color == white) ? (killerTo >> 8) : (killerTo << 8)))
									&& (Pieces[PAWN | (color ^ 1)] & tmpEnPassant)
									&& ((All_Pieces(color) & killerTo) == 0)){
								capturedPos = tmpEnPassant;
								capturedPiece = PAWN | (color ^ 1);
								capturedSq = (color == white) ? (killerToSq - 8) : (killerToSq + 8);
								ASSUME(toPiece == (PAWN | color));
								ASSUME(Pieces[PAWN | (color ^ 1)] & capturedPos);
								ASSUME(All_Pieces(color ^ 1) & capturedPos);
							} else {
								statistics(++ttError_Type1_SameHashKey);
								killerMoveOk = false;
							}
							if (killerMoveOk) {
								ASSUME((toPiece & colormask) == color);
								int scoreD = Value::piece[capturedPiece] + Value::piece[PAWN | color] - Value::piece[toPiece];
								Zobrist toggle = zobrist::keys[capturedSq][capturedPiece];
								toggle ^= zobrist::keys[killerFromSq][PAWN | color];
								toggle ^= zobrist::keys[killerToSq][toPiece];
								//ASSUME((promSp <= 0xF) || (Pieces[capturedPiece] & capturedPos));
								//ASSUME((promSp > 0xF) || (Pieces[capturedPiece] & capturedPos));
								Pieces[capturedPiece] ^= capturedPos;
								//ASSUME(Pieces[PAWN | color] & killerFrom);
								Pieces[PAWN | color] ^= killerFrom;
								//ASSUME((Pieces[toPiece] & killerTo)==0);
								Pieces[toPiece] ^= killerTo;
								All_Pieces(color) ^= tf;
								All_Pieces(color ^ 1) ^= capturedPos;
								pieceScore -= scoreD;
								zobr ^= toggle;
								searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
								zobr ^= toggle;
								pieceScore += scoreD;
								Pieces[capturedPiece] ^= capturedPos;
								Pieces[PAWN | color] ^= killerFrom;
								Pieces[toPiece] ^= killerTo;
								All_Pieces(color) ^= tf;
								All_Pieces(color ^ 1) ^= capturedPos;
								if( score >= beta ) {
									halfmoves = oldhm;
									zobr ^= zobrist::blackKey;
									enPassant = tmpEnPassant;
									if (enPassant) zobr ^= zobrist::enPassant[enSq];
									if (color==black) --fullmoves;
									addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, killerMove, beta);
									statistics(++betaCutOff);
									statistics(++cutOffByKillerMove);
									return beta;			// fail-hard beta-cutoff
								}
								pvFound = true;
								if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
									alpha = score;	//Better move found!
									bestTF = tf;
									bestProm = promSp;
								}
							}
						} else {
							int scoreD = Value::piece[PAWN | color] - Value::piece[toPiece];
							Zobrist toggle = zobrist::keys[killerFromSq][PAWN | color];
							toggle ^= zobrist::keys[killerToSq][toPiece];
							if (killerToSq == (killerFromSq + ((color == white) ? 16 : -16))){
								toggle ^= zobrist::enPassant[7&(killerFromSq + ((color == white) ? 8 : -8))];
								enPassant = ((color == white) ? (killerFrom << 8) : (killerFrom >> 8));
							}
							ASSUME(Pieces[PAWN | color] & killerFrom);
							Pieces[PAWN | color] ^= killerFrom;
							ASSUME(toPiece >= (PAWN | white));
							ASSUME(toPiece <= (KING | black));
							if ((toPiece & colormask) != color) std::cout << std::dec << toPiece << std::endl;
							ASSUME((toPiece & colormask) == color);
							ASSUME((Pieces[toPiece] & killerTo) == 0);
							Pieces[toPiece] ^= killerTo;
							All_Pieces(color) ^= tf;
							pieceScore -= scoreD;
							zobr ^= toggle;
							searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
							zobr ^= toggle;
							pieceScore += scoreD;
							Pieces[PAWN | color] ^= killerFrom;
							Pieces[toPiece] ^= killerTo;
							All_Pieces(color) ^= tf;
							if( score >= beta ) {
								halfmoves = oldhm;
								zobr ^= zobrist::blackKey;
								enPassant = tmpEnPassant;
								if (enPassant) zobr ^= zobrist::enPassant[enSq];
								if (color==black) --fullmoves;
								addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, killerMove, beta);
								statistics(++betaCutOff);
								statistics(++cutOffByKillerMove);
								return beta;			// fail-hard beta-cutoff
							}
							pvFound = true;
							if( ( mode == PV || mode >= quiescenceMask ) && score > alpha ){
								alpha = score;	//Better move found!
								bestTF = tf;
								bestProm = promSp;
							}
							enPassant = 0;
						}
					} else {
						statistics(++ttError_Type1_SameHashKey);
					}
				}
			} else {
				statistics(++ttError_Type1_SameHashKey);
			}
		}
	}
#endif
#endif
	//TODO add heuristics
	bitboard checkedBy = kingIsAttackedBy<color>(occ, kingSq);
	//FIXME REMOVE
	all = 0;
	for (int i = PAWN | color ; i <= (KING | color) ; i += 2) all |= Pieces[i];
	for (int i = 0 ; i <= (KING | black) ; ++i) if (oldP[i] != Pieces[i]) std::cout << i << std::hex << Pieces[i] << oldP[i] << std::endl;
	ASSUME(oldAll == All_Pieces(color));
	ASSUME(all == oldAll);
	if (checkedBy == bitboard(0)){
		bitboard nPinnedPawn = getNPinnedPawns<color>(occ, kingSq);
		bitboard attacking[2] = {Pieces[PAWN | color], Pieces[PAWN | color]};
		int kingFL = 7 & kingSq;
		int kingRK = kingSq >> 3;
		int kingAD = kingFL+kingRK;
		kingFL ^= 7;
		int kingMD = kingFL+kingRK;	//7-(kingFL^7)+kingRK

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
				bitboard tmp = attacking[at] & Pieces[captured];
				while (tmp){
					unsigned long int toSq;
					bitboard to = tmp & -tmp;
					bitboard from = to;
					if (color == white){
						from >>= diff;
					} else {
						from <<= -diff;
					}
					bitboard tf = to | from;
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tmp = attacking[at] & Pieces[captured];
				while (tmp){
					unsigned long int toSq;
					bitboard to = tmp & -tmp, from;
					if (color == white){
						from = to >> diff;
					} else {
						from = to << -diff;
					}
					bitboard tf = to | from;
					square(&toSq, to);
					Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tf = tmpEnPassant;
				bitboard cp = tmpEnPassant;
				if (color == white){
					tf |= tmpEnPassant >> diff;
					cp >>= 8;
				} else {
					tf |= tmpEnPassant << -diff;
					cp <<= 8;
				}
				unsigned long int toSq;
				square(&toSq, tmpEnPassant);
				Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
					if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			bitboard tmp = pawnsToForward;
			if (color == white){
				tmp = lastRank_w & (tmp << 8);
			} else {
				tmp = lastRank_b & (tmp >> 8);
			}
			tmp &= empty;
			pieceScore -= Value::piece[PAWN | color];
			while (tmp){
				bitboard to = tmp & -tmp;
				bitboard from = to;
				if (color == white){
					from >>= 8;
				} else {
					from <<= 8;
				}
				bitboard tf = to | from;
				unsigned long int toSq;
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
		bitboard attack[SIZE], frombb[SIZE];
		unsigned long int piecet[SIZE], fromSq[SIZE], n(0);
#undef SIZE
		bitboard KAttack = KingMoves[kingSq];
		unsigned long int firstRook, firstQueen;
		TargetSquareGenerator:{
			int dr;
			bitboard tmp = Pieces[KNIGHT | color];
			//TODO Only knights that are not pinned can move, so tmp's population is predictable from here
			while (tmp){
				bitboard xRay;
				frombb[n] = tmp & -tmp;
				square(fromSq+n, frombb[n]);
				dr = direction[kingSq][fromSq[n]];
				//A pinned knight has no legal moves.
				if (dr == WRONG_PIECE || (rays[kingSq][fromSq[n]] & occ) != 0 ||
						(((xRay = getChecker<color>(occ, fromSq[n], kingSq)) & Pieces[QUEEN | (color ^ 1)]) == 0
								&& (xRay & Pieces[dr | (color ^ 1)]) == 0)) {
					attack[n] = KnightMoves[fromSq[n]];
					piecet[n] = KNIGHT | color;
					++n;
				}
				tmp &= tmp - 1;
			}
			//TODO A pinned piece can only move between the pinner and the king
			//Also a bishop can only move if he is pinned by a queen or bishop, if he is pinned
			//a rook can only move if he is pinned by a queen or rook, if he is pinned
			//the pinner will be capturable by the pinned piece!
			tmp = Pieces[BISHOP | color];
			while (tmp){
				frombb[n] = tmp & -tmp;
				square(fromSq+n, frombb[n]);
				attack[n] = bishopAttacks(occ, fromSq[n]);
				piecet[n] = BISHOP | color;
				filterAttackBB<color>(occ, fromSq[n], attack[n], kingSq);
				++n;
				tmp &= tmp - 1;
			}
			firstRook = n;
			tmp = Pieces[ROOK | color];
			while (tmp){
				frombb[n] = tmp & -tmp;
				square(fromSq+n, frombb[n]);
				attack[n] = rookAttacks(occ, fromSq[n]);
				piecet[n] = ROOK | color;
				filterAttackBB<color>(occ, fromSq[n], attack[n], kingSq);
				++n;
				tmp &= tmp - 1;
			}
			firstQueen = n;
			tmp = Pieces[QUEEN | color];
			while (tmp){
				frombb[n] = tmp & -tmp;
				square(fromSq+n, frombb[n]);
				attack[n] = queenAttacks(occ, fromSq[n]);
				piecet[n] = QUEEN | color;
				filterAttackBB<color>(occ, fromSq[n], attack[n], kingSq);
				++n;
				tmp &= tmp - 1;
			}
		}
		//frombb[n] = Pieces[KING | color];
		//fromSq[n] = kingSq[color];//square(frombb[n]);
		//attack[n] = KingMoves[kingSq[color]]; KAttack
		//n : position of last bitboard generated
		if ((castling & castlingrights[color]) == 0){
			for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
				pieceScore -= Value::piece[captured];
				for (unsigned long int i = 0 ; i < n ; ++i) {
					bitboard tmp = Pieces[captured] & attack[i];
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						Pieces[captured] ^= to;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						zobr ^= toggle;
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tmp = Pieces[captured] & KAttack;
				while (tmp){
					bitboard to = tmp & -tmp;
					unsigned long int nkSq;
					square(&nkSq, to);
					bitboard tf = to | Pieces[KING | color];
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					if (validPosition<color>(nkSq)){
						Zobrist toggle = zobrist::keys[kingSq][captured];
						toggle ^= zobrist::keys[kingSq][KING | color];
						toggle ^= zobrist::keys[nkSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[captured] ^= to;
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;
							All_Pieces(color ^ 1) ^= to;
							pieceScore += Value::piece[captured];

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				pieceScore += Value::piece[captured];
			}

			if (mode < quiescenceMask){
				halfmoves = oldhm + 1;
				for (unsigned long int i = 0 ; i < n ; ++i) {
					bitboard tmp = attack[i] & empty;
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][piecet[i]];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tmp = KAttack & empty;
				while (tmp){
					bitboard to = tmp & -tmp;
					unsigned long int nkSq;
					square(&nkSq, to);
					bitboard tf = to | Pieces[KING | color];
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					if (validPosition<color>(nkSq)){
						Zobrist toggle = zobrist::keys[kingSq][KING | color];
						toggle ^= zobrist::keys[nkSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			}
		} else {
			bitboard oldcastling = castling;
			key ct = zobrist::castling[(castling*castlingsmagic)>>59];
			key ct2;
			unsigned long int i;
			for (int captured = QUEEN | (color ^ 1); captured >= 0 ; captured -= 2){
				pieceScore -= Value::piece[captured];
				for (i = 0; i < firstRook ; ++i) {
					bitboard tmp = Pieces[captured] & attack[i];
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][captured];
						toggle ^= zobrist::keys[toSq][piecet[i]];
						toggle ^= zobrist::keys[fromSq[i]][piecet[i]];
						Pieces[captured] ^= to;
						Pieces[piecet[i]] ^= tf;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= to;
						zobr ^= toggle;
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					bitboard tmp = Pieces[captured] & attack[i];
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][captured];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					bitboard tmp = Pieces[captured] & attack[i];
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][captured];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tmp = Pieces[captured] & KAttack;
				while (tmp){
					bitboard to = tmp & -tmp;
					unsigned long int nkSq;
					square(&nkSq, to);
					bitboard tf = to | Pieces[KING | color];
					Pieces[captured] ^= to;
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					All_Pieces(color ^ 1) ^= to;
					if (validPosition<color>(nkSq)){
						Zobrist toggle = zobrist::keys[nkSq][captured];
						toggle ^= zobrist::keys[nkSq][KING | color];
						toggle ^= zobrist::keys[kingSq][KING | color];
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

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				castling = oldcastling;
				zobr ^= ct;
				zobr ^= ct2;
				pieceScore += Value::piece[captured];
			}
			if (mode < quiescenceMask){
				halfmoves = oldhm + 1;
				if ((castling & (castlingc<color>::KingSide) & Pieces[ROOK | color]) && (castlingc<color>::KingSideSpace & occ)==0 && notAttacked<color^1>(castlingc<color>::KSCPassing, castlingc<color>::KSCPassingSq) && validPosition<color>(kingSq)){
					Pieces[KING | color] ^= castlingc<color>::KSCKT;
					All_Pieces(color) ^= castlingc<color>::KSCFT;
					if (validPosition<color>(castlingc<color>::kingSqAfterKSC)){
						castling &= castlingc<color>::deactrights;
						Zobrist toggle = zobrist::keys[0+((color==black)?56:0)][ROOK | color];
						toggle ^= zobrist::keys[2+((color==black)?56:0)][ROOK | color];
						toggle ^= zobrist::keys[3+((color==black)?56:0)][KING | color];
						toggle ^= zobrist::keys[1+((color==black)?56:0)][KING | color];
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

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					kingSq = castlingc<color>::kingSqBefore;
					Pieces[KING | color] ^= castlingc<color>::KSCKT;
					All_Pieces(color) ^= castlingc<color>::KSCFT;
				}
				if ((castling & (castlingc<color>::QueenSide) & Pieces[ROOK | color])!=0 && (castlingc<color>::QueenSideSpace & occ)==0 && notAttacked<color^1>(castlingc<color>::QSCPassing, castlingc<color>::QSCPassingSq) && validPosition<color>(kingSq)){
					Pieces[KING | color] ^= castlingc<color>::QSCKT;
					All_Pieces(color) ^= castlingc<color>::QSCFT;
					if (validPosition<color>(castlingc<color>::kingSqAfterQSC)){
						castling &= castlingc<color>::deactrights;
						Zobrist toggle = zobrist::keys[7+((color==black)?56:0)][ROOK | color];
						toggle ^= zobrist::keys[4+((color==black)?56:0)][ROOK | color];
						toggle ^= zobrist::keys[3+((color==black)?56:0)][KING | color];
						toggle ^= zobrist::keys[5+((color==black)?56:0)][KING | color];
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

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					Pieces[KING | color] ^= castlingc<color>::QSCKT;
					All_Pieces(color) ^= castlingc<color>::QSCFT;
				}

				for (i = 0; i < firstRook ; ++i) {
					bitboard tmp = attack[i] & empty;
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][piecet[i]];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					bitboard tmp = attack[i] & empty;
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][ROOK | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					bitboard tmp = attack[i] & empty;
					while (tmp){
						bitboard to = tmp & -tmp;
						bitboard tf = to | frombb[i];
						unsigned long int toSq;
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][QUEEN | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard tmp = KAttack & empty;
				while (tmp){
					bitboard to = tmp & -tmp;
					bitboard tf = to | Pieces[KING | color];
					unsigned long int nkSq;
					square(&nkSq, to);
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;
					if (validPosition<color>(nkSq)){
						Zobrist toggle = zobrist::keys[nkSq][KING | color];
						toggle ^= zobrist::keys[kingSq][KING | color];
						zobr ^= toggle;
						searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
						zobr ^= toggle;
						if( score >= beta ) {
							Pieces[KING | color] ^= tf;
							All_Pieces(color) ^= tf;

							castling = oldcastling;
							zobr ^= ct;
							zobr ^= ct2;

							halfmoves = oldhm;
							zobr ^= zobrist::blackKey;
							enPassant = tmpEnPassant;
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				castling = oldcastling;
				zobr ^= ct;
				zobr ^= ct2;
			}
		}
		if (mode < quiescenceMask){
			halfmoves = 0;

			bitboard tmp = pawnsToForward;
			if (color == white){
				tmp <<= 8;
				tmp &= notlastRank_w;
			} else {
				tmp >>= 8;
				tmp &= notlastRank_b;
			}
			tmp &= empty;
			while (tmp){
				bitboard to = tmp & -tmp, tf;
				if (color == white){
					tf = to | (to >> 8);
				} else {
					tf = to | (to << 8);
				}
				unsigned long int toSq;
				square(&toSq, to);
				Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
					if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmp){
				bitboard to = tmp & -tmp, tf;
				if (color == white){
					tf = to | (to >> 16);
					enPassant = to >> 8;
				} else {
					tf = to | (to << 16);
					enPassant = to << 8;
				}
				unsigned long int toSq, tmpSq;
				square(&toSq, to);
				square(&tmpSq, enPassant);
				Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
					if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
		//bitboard tmp = occ;
		//unsigned long int fromSq;
		if (( checkedBy & (checkedBy - 1) ) == bitboard(0)){
			//1) Capturing the attacking piece
			unsigned long int toSq;
			square(&toSq, checkedBy);
			int attacker = QUEEN | (color ^ 1);
			while ((Pieces[attacker] & checkedBy)==0) attacker -= 2;
			zobr ^= zobrist::keys[toSq][attacker];
			Pieces[attacker] ^= checkedBy;
			All_Pieces(color ^ 1) ^= checkedBy;
			pieceScore -= Value::piece[attacker];
			if ((checkedBy & (color==white?lastRank_w:lastRank_b)) == 0){
				for (int diff = (color==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
					bitboard att = (color == white) ? (checkedBy >> diff) : (checkedBy << -diff);
					att &= notFilled::file[f] & Pieces[PAWN | color];
					if (att){
						bitboard tf = checkedBy | (att & -att);
						All_Pieces(color) ^= tf;
						if (validPositionNonChecked<color>(kingSq)){
							Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
								if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
						bitboard att = (color == white) ? (tmpEnPassant >> diff) : (tmpEnPassant << -diff);
						att &=  notFilled::file[f] & Pieces[PAWN | color];
						if (att){
							bitboard tf = tmpEnPassant | (att & -att);
							All_Pieces(color) ^= tf;
							if (validPositionNonChecked<color>(kingSq)){
								unsigned long int toenpsq;
								square(&toenpsq, tmpEnPassant);
								Zobrist toggle = zobrist::keys[toenpsq][PAWN | color];
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
									if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
					bitboard att = (color == white) ? (checkedBy >> diff) : (checkedBy << -diff);
					att &=  notFilled::file[f] & Pieces[PAWN | color];
					if (att){
						bitboard from = att & -att;
						bitboard tf = checkedBy | from;
						All_Pieces(color) ^= tf;
						if (validPositionNonChecked<color>(kingSq)){
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
									if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			bitboard tmp = Pieces[KNIGHT | color] & KnightMoves[toSq];
			while (tmp){
				bitboard from = tmp & -tmp;
				bitboard tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)){
					unsigned long int fromSq;
					square(&fromSq, from);
					Zobrist toggle = zobrist::keys[fromSq][KNIGHT | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard from = tmp & -tmp;
				bitboard tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)){
					unsigned long int fromSq;
					square(&fromSq, from);
					Zobrist toggle = zobrist::keys[fromSq][BISHOP | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard from = tmp & -tmp;
				bitboard tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)){
					castling &= ~from;
					unsigned long int fromSq;
					square(&fromSq, from);
					Zobrist toggle = zobrist::keys[fromSq][ROOK | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
				bitboard from = tmp & -tmp;
				bitboard tf = from | checkedBy;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)){
					unsigned long int fromSq;
					square(&fromSq, from);
					Zobrist toggle = zobrist::keys[fromSq][QUEEN | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			unsigned long int tmpSq;
			square(&tmpSq, Pieces[KING | color]);
			bitboard ray = rays[tmpSq][toSq];
			/**
			 * FIXME
			 *
position startpos moves c2c4 e7e6 d2d4 g8f6 b1c3 c7c5 d4d5 e6d5 c4d5 d7d6 g1f3 g7g6 e2e4 a7a6 c1g5 h7h6 d1a4 c8d7 g5f6 d8f6 a4b3 b7b5 f1e2 f8g7 a1b1 e8g8 e2d1 f6f4 d1c2 f7f5 e4f5 f8e8 c3e2 d7f5 e1f1 f5c2 e2f4 c2b3 a2b3 g6g5 f4e6 e8e7 f3d2 b8d7 d2e4 g7e5 e6c7 a8c8 c7a6 d7b6 b1e1 e7a7 e4c5 b6d5 c5d3 a7a6 d3e5 d6e5 g2g3 c8e8 h1g1 e5e4 e1d1 e4e3 d1d5 a6a1 f1e2 a1g1 f2e3 g1g2 e2f3 g2h2 d5b5 h2d2 b5b6 d2d3 f3g4 e8e3 b6b8 g8f7 b8b7 f7f6 b7b6 f6e5 b6b5 e5e4 b5b4 e4d5
2012-08-30 17:15:43.911-->1:go wtime 32903 btime 25871
2012-08-30 17:15:43.914<--1:info depth 2 time 1 nodes 96 nps 96000 pv g4h5 e3g3  score cp -455 hashfull 49
2012-08-30 17:15:43.920<--1:info depth 3 time 1 nodes 340 nps 340000 pv g4h5 e3g3 h5h6  score cp -455 hashfull 49
2012-08-30 17:15:43.923<--1:info depth 4 time 1 nodes 1531 nps 1531000 pv g4h5 e3g3 h5h6  score cp -455 hashfull 49
2012-08-30 17:15:43.927<--1:info depth 5 time 3 nodes 4922 nps 1640666 pv g4h5 e3g3 h5h6  score cp -455 hashfull 49
2012-08-30 17:15:43.931<--1:info depth 6 time 9 nodes 27290 nps 3032222 pv g4h5 e3g3 h5h6  score cp -455 hashfull 49
2012-08-30 17:15:43.936<--1:info depth 7 time 13 nodes 38546 nps 2965076 pv g4h5 e3g3 h5h6  score cp -500 hashfull 49
2012-08-30 17:15:43.940<--1:info depth 8 time 15 nodes 43960 nps 2930666 pv g4h5 e3g3 h5h6  score cp -500 hashfull 49
2012-08-30 17:15:43.943<--1:info depth 9 time 15 nodes 43962 nps 2930800 pv g4h5 e3g3 h5h6  score cp -500 hashfull 49

			 * Assertion failed: (ray & ~occ) == ray, file c:\users\periklis\documents\coding\chapeiro\source\Board.h, line 2107
			 */
			ASSUME((ray & ~occ) == ray);
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
				unsigned long int tmpSq2;
				square(&tmpSq2, tmpEnPassant);
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
						bitboard tf = tmpEnPassant | attacker;
						Pieces[PAWN | color] ^= tf;
						Pieces[PAWN | (color^1)] ^= cp;
						All_Pieces(color) ^= tf;
						All_Pieces(color ^ 1) ^= cp;
						if (validPositionNonChecked<color>(kingSq)){
							Zobrist toggle = zobrist::keys[tmpSq2][PAWN | color];
							toggle ^= zobrist::keys[tmpSq2-diff][PAWN | color];
							toggle ^= zobrist::keys[tmpSq2+(color==white)?-8:8][PAWN | (color ^ 1)];
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
								if (enPassant) zobr ^= zobrist::enPassant[enSq];
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

			bitboard tmpP;
			bitboard tmp2 = Pieces[PAWN | color];
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
			while (tmp2){
				bitboard to = tmp2 & -tmp2;
				bitboard tf = to;
				if (color == white){
					tf |= (tf >> 16);
					enPassant = to >> 8;
				} else {
					tf |= (tf << 16);
					enPassant = to << 8;
				}
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)) {
					square(&toSq, to);
					square(&tmpSq, enPassant);
					Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmpP){
				bitboard to = tmpP & -tmpP;
				bitboard from = to;
				if (color == white){
					from >>= 8;
				} else {
					from <<= 8;
				}
				bitboard tf = to | from;
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)) {
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmp ){
				bitboard to = tmp & -tmp;
				bitboard tf = to;
				if (color == white){
					tf |= (tf >> 8);
				} else {
					tf |= (tf << 8);
				}
				All_Pieces(color) ^= tf;
				if (validPositionNonChecked<color>(kingSq)) {
					square(&toSq, to);
					Zobrist toggle = zobrist::keys[toSq][PAWN | color];
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
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			halfmoves = oldhm + 1;
			tmpP = Pieces[KNIGHT | color];
			while (tmpP){
				bitboard from = tmpP & -tmpP;
				unsigned long int fromSq;
				square(&fromSq, from);
				tmp = ray & KnightMoves[fromSq];
				while (tmp){
					bitboard to = tmp & -tmp;
					bitboard tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>(kingSq)){
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][KNIGHT | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmpP){
				bitboard from = tmpP & -tmpP;
				unsigned long int fromSq;
				square(&fromSq, from);
				tmp = ray & bishopAttacks(occ, fromSq);
				while (tmp){
					bitboard to = tmp & -tmp;
					bitboard tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>(kingSq)){
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][BISHOP | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmpP){
				bitboard from = tmpP & -tmpP;
				unsigned long int fromSq;
				square(&fromSq, from);
				tmp = ray & rookAttacks(occ, fromSq);
				while (tmp){
					bitboard to = tmp & -tmp;
					bitboard tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>(kingSq)){
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][ROOK | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
			while (tmpP){
				bitboard from = tmpP & -tmpP;
				unsigned long int fromSq;
				square(&fromSq, from);
				tmp = ray & queenAttacks(occ, fromSq);
				while (tmp){
					bitboard to = tmp & -tmp;
					bitboard tf = to | from;
					All_Pieces(color) ^= tf;
					if (validPositionNonChecked<color>(kingSq)){
						square(&toSq, to);
						Zobrist toggle = zobrist::keys[toSq][QUEEN | color];
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
							if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
		halfmoves = 0;
		bitboard from = Pieces[KING | color];
		unsigned long int fromSq;
		square(&fromSq, from);
		bitboard mv = KingMoves[fromSq];
		bitboard tmp1 = mv;
		bitboard oldcastling = castling;
		key ct = zobrist::castling[(castling*castlingsmagic)>>59];
		castling &= castlingc<color>::deactrights;
		ct ^= zobrist::castling[(castling*castlingsmagic)>>59];
		zobr ^= ct;
		for (int attacker = QUEEN | (color ^ 1); attacker >= 0 ; attacker -= 2){
			bitboard tmp = Pieces[attacker] & tmp1;
			while (tmp){
				bitboard to = tmp & -tmp;
				square(&kingSq, to);
				bitboard tf = from | to;
				Pieces[attacker] ^= to;
				All_Pieces(color ^ 1) ^= to;
				Pieces[KING | color] ^= tf;
				All_Pieces(color) ^= tf;
				if (validPosition<color>(kingSq)){
					Zobrist toggle = zobrist::keys[fromSq][KING | color];
					toggle ^= zobrist::keys[kingSq][KING | color];
					toggle ^= zobrist::keys[kingSq][attacker];
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

						halfmoves = oldhm;
						zobr ^= zobrist::blackKey;
						enPassant = tmpEnPassant;
						if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
		halfmoves = oldhm + 1;
		bitboard tmp = mv;
		tmp &= ~occ;
		while (tmp){
			bitboard to = tmp & -tmp;
			bitboard tf = to | from;
			square(&kingSq, to);
			Pieces[KING | color] ^= tf;
			All_Pieces(color) ^= tf;
			if (validPosition<color>(kingSq)){
				Zobrist toggle = zobrist::keys[kingSq][KING | color];
				toggle ^= zobrist::keys[fromSq][KING | color];
				zobr ^= toggle;
				searchDeeper<mode, color^1>(alpha, beta, depth, pvFound, score);
				zobr ^= toggle;
				if( score >= beta ) {
					Pieces[KING | color] ^= tf;
					All_Pieces(color) ^= tf;

					castling = oldcastling;
					zobr ^= ct;

					halfmoves = oldhm;
					zobr ^= zobrist::blackKey;
					enPassant = tmpEnPassant;
					if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
		castling = oldcastling;
		zobr ^= ct;
	}
	halfmoves = oldhm;
	zobr ^= zobrist::blackKey;
	enPassant = tmpEnPassant;
	if (enPassant) zobr ^= zobrist::enPassant[enSq];
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
	if (!interruption_requested){
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
				addTTEntry<mode == ZW ? AlphaCutoff : ExactScore>(zobr, depth, (bestTF == bitboard(0)) ? killerMove : getMove<color>(bestTF, bestProm), alpha);
			}
		} else {
			addTTEntry<QSearchAlphaCutoff>(zobr, depth, (bestTF == bitboard(0)) ? killerMove : getMove<color>(bestTF, bestProm), alpha);
		}
	}
	return alpha;
}

template<int color> inline int Board::getMove(bitboard tf, int prom){
	unsigned long int fromSq, toSq;
	ASSUME(tf != bitboard(0));
	ASSUME(prom < (TTMove_EnPassantPromFlag << 1));
	ASSUME((tf & All_Pieces(color)) != bitboard(0));
	ASSUME((tf & (~All_Pieces(color))) != bitboard(0));
	square(&fromSq, tf & All_Pieces(color));
	square(&toSq, tf & (~All_Pieces(color)));
	ASSUME(0 <= fromSq && fromSq < 64);
	ASSUME(0 <= toSq && toSq < 64);
	if (tf & Pieces[PAWN | color]) {
		ASSUME(color == (prom & colormask));
		return getTTMoveFormat(fromSq, toSq, prom);
	}
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
	unsigned long int kingSq;
	square(&kingSq, Pieces[KING | color]);
	while (moving != 0){
		to = moving & -moving;
		if (color == white){
			tf = to | (to >> 8);
		} else {
			tf = to | (to << 8);
		}
		if (validPosition<color>(occ ^ tf, kingSq)) return false;
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
		if (validPosition<color>(occ ^ tf, kingSq)) return false;
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
			if (validPosition<color>(occ ^ tf, kingSq)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from, kingSq);
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
			if (validPosition<color>(occ ^ tf, kingSq)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from, kingSq);
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
			if (validPosition<color>(occ ^ tf, kingSq)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from, kingSq);
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
			if (validPosition<color>(occ ^ tf, kingSq)) return false;
			moving &= moving - 1;
		}
		for (int captured = QUEEN | (color ^ 1) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving != 0){
				to = moving & -moving;
				Pieces[captured] ^= to;
				res = validPosition<color>(occ ^ from, kingSq);
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
				res = validPosition<color>(occ ^ from, kingSq);
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
			res = validPosition<color>(occ ^ cpt ^ from ^ enPassant, kingSq);
			Pieces[PAWN | (color ^ 1)] ^= cpt;
			if (res) return false;
			moving &= moving - 1;
		}
	}
	//If castling was available, King would had a normal move as well!
	return true;
}

template<int color> int Board::getEvaluation(int depth){
	/**
	 * FIXME
	 * 8/8/8/8/6k1/R5p1/P1r3P1/5K2 b - - 21 71
	 * 3-fold, not promoting pawn because of negative values for pawns on sides (1-2 steps on endgame)
	 *
	 * Mate in 1 :
	 * 8/8/pppppppK/NBBR1NRp/nbbrqnrP/PPPPPPPk/8/Q7 w - - 0 1
	 */
	unsigned long int kingSqW, kingSqB;
	square(((color == white) ? &kingSqW : &kingSqB), Pieces[KING | color]);
	if (stalemate<color>()){
		if (validPosition<color>(((color == white) ? kingSqW : kingSqB))) return 0; //stalemate
		if (color == white) return -Value::MAT+rootDepth-depth;	//White Mated
		return Value::MAT-rootDepth+depth;						//Black Mated
	}
	square(((color == black) ? &kingSqW : &kingSqB), Pieces[KING | color]);
	int score = pieceScore;
	score += Value::kingSq[kingSqW] - Value::kingSq[kingSqB];
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
	bitboard emptyFiles = Pieces[PAWN | white] | Pieces[PAWN | black];
	emptyFiles |= emptyFiles << 8;
	emptyFiles |= emptyFiles << 16;
	emptyFiles |= emptyFiles << 32;
	emptyFiles |= emptyFiles >> 8;
	emptyFiles |= emptyFiles >> 16;
	emptyFiles |= emptyFiles >> 32;
	emptyFiles = ~emptyFiles;
	score += Value::rookOnOpenFile*(popCount(emptyFiles & (Pieces[ROOK | white] | Pieces[QUEEN | white]))-popCount(emptyFiles & (Pieces[ROOK | black] | Pieces[QUEEN | black])));
	score += evaluatePawnStructure();
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

template<int color> inline void Board::filterAttackBB(bitboard occ, unsigned long int sq, bitboard &attack, int kingSq){
	int dr = direction[kingSq][sq];
	bitboard ray = rays[kingSq][sq];
	if (dr != WRONG_PIECE && (ray & occ) == bitboard(0)){
		bitboard xRay = XRayOFCMask[kingSq][sq];
		bitboard ray2 = xRay;
		xRay &= occ;
		xRay *= XRayOFCMagic[kingSq][sq];
		xRay >>= 64 - maxCheckAvoidanceShiftBits;
		xRay = XRayOFCChecker[kingSq][sq][xRay];
		if ((xRay & Pieces[QUEEN | (color ^ 1)]) != 0 || (xRay & Pieces[dr | (color ^ 1)]) != 0) attack &= ray | ray2;
	}
}

template<int color> inline bitboard Board::getNPinnedPawns(bitboard occ, int kingSq){
	bitboard pinnedPawns = bitboard(0);
	bitboard tmp = rookAttacks(occ & ~rookAttacks(occ, kingSq), kingSq);
	tmp &= Pieces[ROOK | (color ^ 1)] | Pieces[QUEEN | (color ^ 1)];
	unsigned long int sq;
	while (tmp){
		square(&sq, tmp);
		pinnedPawns |= rays[kingSq][sq];
		tmp &= tmp - 1;
	}
	tmp = bishopAttacks(occ & ~bishopAttacks(occ, kingSq), kingSq);
	tmp &= Pieces[BISHOP | (color ^ 1)] | Pieces[QUEEN | (color ^ 1)];
	while (tmp){
		square(&sq, tmp);
		pinnedPawns |= rays[kingSq][sq];
		tmp &= tmp - 1;
	}
	pinnedPawns &= Pieces[PAWN | color];
	return (~pinnedPawns);
}

template<int color> inline bitboard Board::getChecker(bitboard occ, unsigned long int sq, int kingSq){
	occ &= XRayOFCMask[kingSq][sq];
	occ *= XRayOFCMagic[kingSq][sq];
	occ >>= 64 - maxCheckAvoidanceShiftBits;
	return XRayOFCChecker[kingSq][sq][occ];
}

inline bool Board::threefoldRepetition(){
	int h = (halfmoves >= lastHistoryEntry) ? (lastHistoryEntry & 1) : (lastHistoryEntry - (halfmoves & (~1)));
	for ( ; h < lastHistoryEntry ; h += 2) if (history[h] == zobr) return true;
	return false;
}

inline int Board::evaluatePawnStructure(){
	bitboard pawns = Pieces[PAWN | white];
//	bitboard emptyFiles = pawns;
	int pscore = 0;
	while (pawns){
		unsigned long int sq;
		square(&sq, pawns & -pawns);
		pscore += Value::WpawnSq[sq];
		pawns &= pawns-1;
	}
	pawns = Pieces[PAWN | black];
//	emptyFiles |= pawns;
	while (pawns){
		unsigned long int sq;
		square(&sq, pawns & -pawns);
		pscore -= Value::BpawnSq[sq];
		pawns &= pawns-1;
	}
	return pscore;
}

inline time_td get_current_time();
inline time_duration milli_to_time(U64 milli);
inline time_duration get_zero_time();
inline time_td get_factored_time(time_td el_time);
#endif /* BOARD_H_ */
