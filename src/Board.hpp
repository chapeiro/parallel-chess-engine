/*
 * Board.h
 *
 * General Board Public Interface
 *
 *  Created on: 29 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#ifndef BOARD_HPP_
#define BOARD_HPP_
#include "cchapeiro.hpp"
#include "Utilities.hpp"
#include "zobristKeys.hpp"
#include "MagicsAndPrecomputedData.hpp"
#include "TimeManagement/TimeManager.hpp"
#include "BoardInterface/BoardInterface.hpp"
#include "SquareMapping.hpp"
#include "MoveEncoding.hpp"
#include <string>
#include "Values.hpp"
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
#include <atomic>
#endif

#include "TimeManagement/TimeManager.hpp"
#include "TranspositionTable.hpp"
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

// #define PIECEMASK (14)
// #define LASTPIECE (12)
// #define PIECESMAX LASTPIECE
// #define WRONG_PIECE (-10)

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
constexpr bitboard lastRank_w = filled::rank[7];
constexpr bitboard lastRank_b = filled::rank[0];
constexpr bitboard notfile0 = notFilled::file[0];
constexpr bitboard notfile7 = notFilled::file[7];
constexpr bitboard notlastRank_w = ~filled::rank[7];
constexpr bitboard notlastRank_b = ~filled::rank[0];
constexpr bitboard dfRank_w = filled::rank[3];
constexpr bitboard dfRank_b = filled::rank[4];
constexpr bitboard pstartRank_w = filled::rank[1];
constexpr bitboard pstartRank_b = filled::rank[6];
**/
constexpr bitboard lastRank_w = fd_rank(7);//filled::rank[7];
constexpr bitboard lastRank_b = fd_rank(0);//filled::rank[0];
constexpr bitboard notfile0 = ~fd_file(0);//notFilled::file[0];
constexpr bitboard notfile7 = ~fd_file(7);//notFilled::file[7];
constexpr bitboard notlastRank_w = ~fd_rank(7);//~filled::rank[7];
constexpr bitboard notlastRank_b = ~fd_rank(0);//~filled::rank[0];
constexpr bitboard dfRank_w = fd_rank(3);//filled::rank[3];
constexpr bitboard dfRank_b = fd_rank(4);//filled::rank[4];
constexpr bitboard pstartRank_w = fd_rank(1);//filled::rank[1];
constexpr bitboard pstartRank_b = fd_rank(6);//filled::rank[6];

//plr definitions

constexpr color white = WHITE;
constexpr color black = BLACK;
constexpr int colormask = 1;


// #define white (0)
// #define black (1)
// #define colormask (1)

struct KingException : public std::exception {
	bool plr;
	int number;
	KingException(bool k, int num) : plr(k), number(num){}
	const char* what() const throw() {
		if (plr == white){
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
template<color plr> class castlingc {
public:
	static constexpr bitboard KingSide 				= (plr == WHITE) ? 0x0000000000000001ull : 0x0100000000000000ull;
	static constexpr bitboard QueenSide 			= (plr == WHITE) ? 0x0000000000000080ull : 0x8000000000000000ull;
	static constexpr bitboard KingSideSpace 		= (plr == WHITE) ? 0x0000000000000006ull : 0x0600000000000000ull;
	static constexpr bitboard QueenSideSpace 		= (plr == WHITE) ? 0x0000000000000070ull : 0x7000000000000000ull;
	static constexpr bitboard KSCPassing 			= (plr == WHITE) ? 0x0000000000000004ull : 0x0400000000000000ull;
	static constexpr int KSCPassingSq 				= (plr == WHITE) ? 2 : 58;//square(KSCPassing);
	static constexpr bitboard QSCPassing 			= (plr == WHITE) ? 0x0000000000000010ull : 0x1000000000000000ull;
	static constexpr int QSCPassingSq 				= (plr == WHITE) ? 4 : 60;//square(QSCPassing);
	static constexpr int kingSqBefore 				= (plr == WHITE) ? 3 : 59;
	static constexpr int kingSqAfterKSC 			= (plr == WHITE) ? 1 : 57;
	static constexpr int kingSqAfterQSC 			= (plr == WHITE) ? 5 : 61;
	static constexpr bitboard KSCKT 				= (plr == WHITE) ? 0x000000000000000Aull : 0x0A00000000000000ull;
	static constexpr bitboard QSCKT 				= (plr == WHITE) ? 0x0000000000000028ull : 0x2800000000000000ull;
	static constexpr bitboard KSCRT 				= (plr == WHITE) ? 0x0000000000000005ull : 0x0500000000000000ull;
	static constexpr bitboard QSCRT 				= (plr == WHITE) ? 0x0000000000000090ull : 0x9000000000000000ull;
	static constexpr bitboard KSCFT 				= (plr == WHITE) ? 0x000000000000000Full : 0x0F00000000000000ull;
	static constexpr bitboard QSCFT 				= (plr == WHITE) ? 0x00000000000000B8ull : 0xB800000000000000ull;
	static constexpr bitboard deactrights 			= (plr == WHITE) ? 0xFFFFFFFFFFFFFF7Eull : 0x7EFFFFFFFFFFFFFFull;
};
constexpr bitboard castlingsmagic    = 0x1040000000000041ull;//0x8100000000000081ull;
constexpr bitboard allcastlingrights = 0x8100000000000081ull;
constexpr bitboard castlingrights[2] = {0x0000000000000081ull, 0x8100000000000000ull};

#define All_Pieces(x) ((((x)&colormask)==WHITE) ? White_Pieces : Black_Pieces)

class Task;

struct node_statistics{
	U64 horizonNodes;
	U64 nodes;
	U64 qNodes;

	inline node_statistics(){
		reset();
	}

	inline void reset(){
		horizonNodes = 0;
		nodes        = 0;
		qNodes       = 0;
	}

	inline node_statistics& operator+=(const node_statistics& other){
		horizonNodes += horizonNodes;
		nodes        += nodes;
		qNodes       += qNodes;
		return *this;
	}
};

extern node_statistics gstats;

extern int rootDepth;
class Board { //cache_align
	friend class Task;
	private:
		//State
		//cache_align 
		bitboard Pieces[PIECESMAX];
		bitboard White_Pieces, Black_Pieces;
		//unsigned long int kingSq[colormask + 1];
		Zobrist zobr;
		bitboard enPassant;
		int pieceScore;
		int playing;
		int halfmoves;
		int fullmoves;
		bitboard castling;
		//Memory
		Zobrist history[256];
		int lastHistoryEntry;
		unsigned int thread_id;

		static Board bmem[MAX_BOARDS];
		static std::mutex bmem_m;
		static unsigned int bmem_unused[MAX_BOARDS];
		static unsigned int bmem_unused_last;

		//for Perft
		node_statistics stats;

	public:
		int dividedepth;
#ifdef DIVIDEPERFT
		std::string pre;
		HANDLE child_input_write;
		HANDLE child_output_read;
#endif

	public:
		struct internal_move{
			bitboard tf;
			unsigned int prom;

			public:
				inline internal_move(bitboard tf, unsigned int prom);
				inline internal_move(bitboard tf);

				inline void set(bitboard tf, unsigned int prom) __restrict;
				inline void set(bitboard tf) __restrict;
		};

	private:
		struct search_state{
			int           alpha;
			int           beta ;
			internal_move bmove;
			int           depth;
			int           score;
			bool          pvFound;

			search_state(int alpha, int beta, int depth);
		};

	public:
		//Construction
		static Board* createBoard(const char FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		static void initialize(){};
		Board(Board * __restrict b);
		Board(char fenBoard[] = (char*) "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", char fenPlaying = 'w', char fenCastling[] = (char*) "KQkq", int fenEnPX = -1, int fenEnPY = -1, int fenHC = 0, int fenFM = 1);

		void make(chapeiro::move m);

		//for debug
		std::string getFEN() __restrict;
		std::string getFEN(int playingl) __restrict;
		void print();
		void printHistory();
		U64 perft(int depth);
		int test(int depth);
		// void go(int maxDepth, time_control tc);
		// void go(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch);

		static void * operator new(size_t size);
		static void operator delete(void * p);
	private:
		char * moveToString(int move, char* m) const __restrict;
		/**
		 * if p is a char representing a white piece,
		 * piece's index at Pieces array is returned.
		 * Else <code>WRONG_PIECE</code> is returned.
		 */
		int getWhitePieceIndex(char p) __restrict;
		/**
		 * if p is a char representing a piece,
		 * piece's index at Pieces array is returned.
		 * else <code>WRONG_PIECE | white</code> or
		 * <code>WRONG_PIECE | black</code> is returned.
		 */
		int getPieceIndex(char p) __restrict;

		void updatePieces(int sq, int ind) __restrict;
		void capture(int to) __restrict;

		void addToHistory(Zobrist position) __restrict;
		void removeLastHistoryEntry() __restrict;
		void forgetOldHistory() __restrict;
		template<color plr> int getEvaluation(int depth) __restrict;
		int evaluatePawnStructure() __restrict;
		template<color plr> void deactivateCastlingRights() __restrict;
		void togglePlaying() __restrict;
		void startSearch(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch);
	public:
		void go(int maxDepth, time_control tc);
		int search(int depth, int alpha, int beta) __restrict;
		int searchT(int depth, int alpha, int beta) __restrict;
		void setThreadID(unsigned int thrd_id);
	private:
		std::string extractPV(int depth);

		template<color plr> bool validPosition(int kingSq) __restrict;
		template<color plr> bool validPosition(bitboard occ, int kingSq) __restrict;
		template<color plr> bool validPositionNonChecked(int kingSq) __restrict;
		template<color plr> bool validPositionNonChecked(bitboard occ, int kingSq) __restrict;
		template<color plr> bool notAttacked(bitboard target, int targetSq) __restrict;
		template<color plr> bool notAttacked(bitboard target, bitboard occ, int targetSq) __restrict;
		template<color plr> bitboard kingIsAttackedBy(bitboard occ, int kingSq) __restrict;

		template<color plr> bool stalemate() __restrict;
		void assert_state() const __restrict;

		static constexpr bitboard bishopAttacks(bitboard occ, const int sq);
		static constexpr bitboard rookAttacks(bitboard occ, const int sq);
		static constexpr bitboard queenAttacks(bitboard occ, const int sq);
		template<color plr> bitboard getChecker(bitboard occ, unsigned long int sq, int kingSq) __restrict;
		template<color plr> void filterAttackBB(bitboard occ, unsigned long int sq, bitboard &attack, int kingSq) __restrict;
		template<color plr> bitboard getNPinnedPawns(bitboard occ, int kingSq) __restrict;
		template<color plr> int getMove(bitboard tf, int prom) __restrict;

		template<SearchMode mode, color plr, bool root> void prepare_beta_cutoff(int oldhm, bitboard old_enpassant, const internal_move& move_entry, int depth, int beta) __restrict;

		template<SearchMode mode, color plr> void searchDeeper(int alpha, int beta, int depth, bool pvFound, int &score) __restrict;
		template<SearchMode mode, color plr, bool root, class UnaryPredicate, class UnaryPredicate2>
			inline __attribute__((always_inline)) bool deeper(const internal_move &child, int oldhm, bitboard old_enpassant, search_state &sst, const UnaryPredicate& toggleMove, int scoreD, const UnaryPredicate2& toggleGroupMove, int scoreGD) __restrict;
		template<SearchMode mode, color plr, bool root> int search(int alpha, int beta, int depth) __restrict;
		template<color plr> int quieSearch(int alpha, int beta) __restrict;

		bool threefoldRepetition() __restrict;
};

inline Board::internal_move::internal_move(bitboard tf, unsigned int prom): tf(tf), prom(prom){ }

inline Board::internal_move::internal_move(bitboard tf): tf(tf), prom(0){ }

inline void Board::internal_move::set(bitboard tf, unsigned int prom) __restrict{
	this->tf   = tf;
	this->prom = prom;
}

inline void Board::internal_move::set(bitboard tf) __restrict{
	this->tf   = tf;
}

inline Board::search_state::search_state(int alpha, int beta, int depth): alpha(alpha), beta(beta), bmove(0, 0), depth(depth), score(0), pvFound(false){ }

inline int Board::getPieceIndex(char p) __restrict{
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A') | BLACK;
	return getWhitePieceIndex(p) | WHITE;
}

inline void Board::updatePieces(int sq, int ind) __restrict{
	Pieces[ind] ^= filled::normal[sq];
	//if ( (ind & ~colormask) == KING ) kingSq[ind & colormask] = sq;
	zobr ^= zobrist::keys[ind][sq];
	All_Pieces(ind) ^= filled::normal[sq];
}

inline void Board::addToHistory(Zobrist position) __restrict{
	history[++lastHistoryEntry] = position;
}

inline void Board::removeLastHistoryEntry() __restrict{
	--lastHistoryEntry;
}

template<color plr> inline void Board::deactivateCastlingRights() __restrict{
	bitboard oldc = castling;
	castling &= castlingc<plr>::deactrights;
	zobr ^= zobrist::castling[((castling^oldc)*castlingsmagic)>>60];
}

inline void Board::togglePlaying() __restrict{
	playing ^= 1;
	zobr ^= zobrist::blackKey;
}

template<color plr> bitboard Board::kingIsAttackedBy(bitboard occ, int kingSq) __restrict{
	ASSUME(kingSq >= 0 && kingSq < 64);
	bitboard attackers = KnightMoves[kingSq];
	attackers &= Pieces[KNIGHT | (!plr)];
	attackers |= rookAttacks(occ, kingSq) & (Pieces[ROOK | (!plr)] | Pieces[QUEEN | (!plr)]);
	attackers |= bishopAttacks(occ, kingSq) & (Pieces[BISHOP | (!plr)] | Pieces[QUEEN | (!plr)]);
	if (plr == black){
		attackers |= (((Pieces[KING | plr] >> 7) & notfile7) | ((Pieces[KING | plr] >> 9) & notfile0)) & Pieces[PAWN | white];
	} else {
		attackers |= (((Pieces[KING | plr] << 9) & notfile7) | ((Pieces[KING | plr] << 7) & notfile0)) & Pieces[PAWN | black];
	}
	return attackers;
}

/**
 * uses only Pieces[ [PAWN ... KING] | plr ], target, All_Pieces([white, black])
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<color plr> inline bool Board::notAttacked(bitboard target, int targetSq) __restrict{
	bitboard occ = All_Pieces(white) | All_Pieces(black);
	return notAttacked<plr>(target, occ, targetSq);
}

/**
 * uses only Pieces[ [PAWN ... KING] | plr ], target, occ
 * returns true if <code>target</code> is not attacked by <code>playing</code>
 */
template<color plr> inline bool Board::notAttacked(bitboard target, bitboard occ, int targetSq) __restrict{
	ASSUME((target & (target-1))==0);
	ASSUME(targetSq >= 0 && targetSq < 64);
	if (plr == black){
		if ( ( (Pieces[PAWN | black] >> 7) & target & notfile7) != 0) return false;
		if ( ( (Pieces[PAWN | black] >> 9) & target & notfile0) != 0) return false;
	} else {
		if ( ( (Pieces[PAWN | white] << 7) & target & notfile0) != 0) return false;
		if ( ( (Pieces[PAWN | white] << 9) & target & notfile7) != 0) return false;
	}
	if ((Pieces[KNIGHT | plr] & KnightMoves[targetSq])!=0) return false;
	if ((Pieces[KING | plr] & KingMoves[targetSq])!=0) return false;
	bitboard att = Pieces[BISHOP | plr] | Pieces[QUEEN | plr];
	if ((att & bishopAttacks(occ, targetSq)) != 0) return false;
	att = Pieces[ROOK | plr] | Pieces[QUEEN | plr];
	return ((att & rookAttacks(occ, targetSq)) == 0ull);
}

template<color plr> inline bool Board::validPosition(bitboard occ, int kingSq) __restrict{
	ASSUME((Pieces[KING | plr] & (Pieces[KING | plr]-1)) == 0);
	return notAttacked<!plr>(Pieces[KING | plr], occ, kingSq);
}

template<color plr> inline bool Board::validPositionNonChecked(bitboard occ, int kingSq) __restrict{
	return (((Pieces[BISHOP | ( !plr )] | Pieces[QUEEN | ( !plr )]) & bishopAttacks(occ, kingSq)) == 0) &&
			(((Pieces[ROOK | ( !plr )] | Pieces[QUEEN | ( !plr )]) & rookAttacks(occ, kingSq)) == 0);
	/**bitboard att = Pieces[BISHOP | ( !plr )] | Pieces[QUEEN | ( !plr )];
	if ((att & bishopAttacks(occ, kingSq)) != 0) return false;
	att = Pieces[ROOK | ( !plr )] | Pieces[QUEEN | ( !plr )];
	if ((att & rookAttacks(occ, kingSq)) != 0) return false;
	return true;**/
}

template<color plr> inline bool Board::validPositionNonChecked(int kingSq) __restrict{
	return validPositionNonChecked<plr>(All_Pieces(white) | All_Pieces(black), kingSq);
}

/**
 * uses only :
 *  kingSq[plr],
 * 	Pieces[ [PAWN ... KING] | !plr ],
 * 	Pieces[KING | plr],
 * 	All_Pieces(white),
 * 	All_Pieces(black)
 */
template<color plr> inline bool Board::validPosition(int kingSq) __restrict{
	ASSUME((Pieces[KING | plr] & (Pieces[KING | plr]-1)) == bitboard(0));
	return notAttacked<!plr>(Pieces[KING | plr], kingSq);
}

template<SearchMode mode, color plr> inline void Board::searchDeeper(int alpha, int beta, int depth, bool pvFound, int &score) __restrict{
	addToHistory(zobr);
	if (mode >= quiescenceMask){
		score = -search<mode, plr, false>(-beta, -alpha, depth - 1);
	} else if (mode == PV){
		if (pvFound) {
			playing = plr; //will only be used in search deeper, when copying the board, so it must be fine...
			if (!(board_interface->search(this, thread_id, depth, alpha, beta, 0, 0))){
				score = -search<ZW, plr, false>(-1-alpha, -alpha, depth - 1);
				if ( score > alpha ) {
					score = -search<PV, plr, false>(-beta, -alpha, depth - 1);
				}
			}
		} else {
			score = -search<PV, plr, false>(-beta, -alpha, depth - 1);
		}
	} else if (mode == ZW){
		score = -search<ZW, plr, false>(-beta, -alpha, depth - 1);
	} else {
		score = -search<Perft, plr, false>(-beta, -alpha, depth - 1);
	}
	removeLastHistoryEntry();
}

template<SearchMode mode, color plr, bool root>
inline void Board::prepare_beta_cutoff(int oldhm, bitboard old_enpassant, const internal_move& move_entry, int depth, int beta) __restrict{
	halfmoves = oldhm;
	if (!root) {
		playing = !plr;
		zobr   ^= zobrist::blackKey;
	}
	enPassant = old_enpassant;
	if (enPassant) zobr ^= zobrist::enPassant[7 & square(enPassant)];
	if (plr==black) --fullmoves;

	addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, depth, getMove<plr>(move_entry.tf, move_entry.prom), beta);
	statistics(++betaCutOff);
}

template<SearchMode mode, color plr, bool root, class UnaryPredicate, class UnaryPredicate2>
inline __attribute__((always_inline)) bool Board::deeper(const internal_move &child, int oldhm, bitboard old_enpassant, search_state &sst, const UnaryPredicate& toggleMove, int scoreD, const UnaryPredicate2& toggleGroupMove, int scoreGD) __restrict{
	pieceScore += scoreD;
	toggleMove();
	searchDeeper<mode, !plr>(sst.alpha, sst.beta, sst.depth, sst.pvFound, sst.score);
	toggleMove();
	pieceScore -= scoreD;

	if( sst.score >= sst.beta ) {
		pieceScore += scoreGD;
		toggleGroupMove();

		halfmoves = oldhm;
		if (!root) {
			playing = !plr;
			zobr   ^= zobrist::blackKey;
		}
		enPassant = old_enpassant;
		if (old_enpassant) zobr ^= zobrist::enPassant[7 & square(old_enpassant)];
		if (plr == black) --fullmoves;

		addTTEntry<(mode < quiescenceMask) ? BetaCutoff : QSearchBetaCutoff>(zobr, sst.depth, getMove<plr>(child.tf, child.prom), sst.beta);
		statistics(++betaCutOff);
		// prepare_beta_cutoff<mode, plr, root>(oldhm, old_enpassant, child, sst.depth, sst.beta);
		sst.score = sst.beta;		// fail-hard beta-cutoff
		return true;
	}
	sst.pvFound = true;
	if( ( mode == PV || mode >= quiescenceMask ) && sst.score > sst.alpha ){
		sst.alpha = sst.score;		//Better move found!
		sst.bmove = child;
	}
	return false;
};

/**
 *
 * @param alpha	lower bound
 * @param beta	lower bound
 * @param depth	number of plies till the horizon, 0 if the horizon has already been reached
 *
 * @return
 **/
template<SearchMode mode, color plr, bool root> int Board::search(int alpha, int beta, int depth) __restrict{
	assert_state();
	//FIXME This is saved as a betaCutOff later in the TT!
	if (plr==white) if (interruption_requested) return INF; //TODO Revision! does not seem such a good idea :(
	//count nodes searched
	++stats.nodes;
	if ((!root) && (mode != Perft) && (mode < quiescenceMask) && (halfmoves >= 100 || threefoldRepetition())) return 0;
	if (mode >= quiescenceMask){
		//count nodes searched by quiescence
		++stats.qNodes;
		int standPat = getEvaluation<plr>(depth);
		if (plr == black) standPat = -standPat;
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
		++stats.horizonNodes;
		if (mode & Perft) {
			// if (dividedepth == 0) std::cout << pre << getFEN(plr) << '\n';
			return beta;
		}
		//Horizon has been reached! Start quiescence search
		//return search<QuiescencePV, plr>(alpha, beta, depth);
		return search<(SearchMode) (mode | quiescenceMask), plr, root>(mode==ZW?beta-1:alpha, beta, depth);
	}
#ifndef NO_TRANSPOSITION_TABLE
	//FIXME fix
	int killerMove = retrieveTTEntry<mode>(zobr, depth, alpha, beta);
	//if (ttResult == ttExactScoreReturned) return alpha;
	if (alpha >= beta) {
		statistics(++hashHitCutOff);
		return alpha;
	}
#endif
	//move state forward (halfmoves (oldhm), fullmoves (fullmoves+{0, 1}(plr)), enPassant (tmpEnPassant)
	int oldhm (halfmoves);

	if (enPassant) zobr ^= zobrist::enPassant[7 & square(enPassant)];
	bitboard tmpEnPassant (enPassant);
	fullmoves += plr;							//if (plr==black) ++fullmoves;
	enPassant = bitboard(0);
	halfmoves = 0;

	if (!root) {
		playing = plr;
		zobr   ^= zobrist::blackKey;
	}

	U64 stNodes (stats.nodes);
	U64 stHorNodes (stats.horizonNodes);

	search_state sst(alpha, beta, depth);

#ifndef NDEBUG
	bitboard all = 0;
	for (int i = PAWN | plr ; i <= (KING | plr) ; i += 2) all |= Pieces[i];
	ASSUME(All_Pieces(plr) == all);
	/**
uci
isready
ucinewgame
isready
position startpos moves g1f3 g8f6 d2d4 d7d5 c2c4 c7c6 e2e3 c8g4 b1c3 e7e6 h2h3 g4h5 g2g4 h5g6 f3e5 b8d7 e5g6 h7g6 d1b3 d8c7 f1g2 d5c4 b3c4 e8c8 g4g5 f6d5 g2d5 e6d5 c4b3 d8e8 e1f1 d7b6 h3h4 f7f6 g5f6 g7f6 h1g1 h8h4 g1g6 c7h7
go infinite
	 */
	all = 0;
	for (int i = PAWN | (!plr) ; i <= (KING | (!plr)) ; i += 2) all |= Pieces[i];
	ASSUME(All_Pieces(!plr) == all);
#endif
	const bitboard occ = All_Pieces(white) | All_Pieces(black);
#ifndef NO_TRANSPOSITION_TABLE
#ifndef NO_KILLER_MOVE
	if (mode != Perft){
		if (killerMove != NULL_MOVE){
			int killerFromSq    = getTTMove_From(killerMove);
			int killerToSq      = getTTMove_To(killerMove);
			bitboard killerFrom = bitboard(1) << killerFromSq;
			bitboard killerTo   = bitboard(1) << killerToSq;

			if (((killerTo & All_Pieces(plr)) == bitboard(0)) && (All_Pieces(plr) & killerFrom)){
				if ((Pieces[PAWN | plr] & killerFrom) == bitboard(0)){
					int killerPiece = KNIGHT | plr;
					while (!(Pieces[killerPiece] & killerFrom)) killerPiece += 2;
					if ((killerPiece == (KING | plr)) && ((KingMoves[killerFromSq] & killerTo) == bitboard(0))){
						halfmoves = oldhm + 1;

						bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
						
						Zobrist toggle = zobrist::keys[KING | plr][3+((plr==black)?56:0)];
						toggle ^= zobrist::castling[(castling*castlingsmagic)>>60];
						toggle ^= zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
						
						if (killerFrom > killerTo) {
							if ((castling & castlingc<plr>::KingSide & Pieces[ROOK | plr]) && ((castlingc<plr>::KingSideSpace & occ) == 0)){
								toggle ^= zobrist::keys[ROOK | plr][0+((plr==black)?56:0)];
								toggle ^= zobrist::keys[ROOK | plr][2+((plr==black)?56:0)];
								toggle ^= zobrist::keys[KING | plr][1+((plr==black)?56:0)];
								auto toggleMove = [this, toggle, toggleCastling](){
									Pieces[KING | plr] ^= castlingc<plr>::KSCKT;
									Pieces[ROOK | plr] ^= castlingc<plr>::KSCRT;
									All_Pieces(plr)    ^= castlingc<plr>::KSCFT;
									zobr               ^= toggle;
									castling           ^= toggleCastling;
								};
								
								internal_move smove(castlingc<plr>::KSCKT);

								if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){statistics(++cutOffByKillerMove);}, 0)) return sst.score;

							} else {
								statistics(++ttError_Type1_SameHashKey);
							}
						} else {
							if ((castling & castlingc<plr>::QueenSide & Pieces[ROOK | plr]) && ((castlingc<plr>::QueenSideSpace & occ) == 0)){
								toggle ^= zobrist::keys[ROOK | plr][7+((plr==black)?56:0)];
								toggle ^= zobrist::keys[ROOK | plr][4+((plr==black)?56:0)];
								toggle ^= zobrist::keys[KING | plr][5+((plr==black)?56:0)];
								auto toggleMove = [this, toggle, toggleCastling](){
									Pieces[KING | plr] ^= castlingc<plr>::QSCKT;
									Pieces[ROOK | plr] ^= castlingc<plr>::QSCRT;
									All_Pieces(plr)    ^= castlingc<plr>::QSCFT;
									zobr               ^= toggle;
									castling           ^= toggleCastling;
								};

								internal_move smove(castlingc<plr>::QSCKT);

								if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){statistics(++cutOffByKillerMove);}, 0)) return sst.score;
							} else {
								statistics(++ttError_Type1_SameHashKey);
							}
						}
						halfmoves = 0;
					} else {
						int capturedPiece = QUEEN | (!plr);
						while (capturedPiece >= 0 && !(Pieces[capturedPiece] & killerTo)) capturedPiece -= 2;
						bitboard tf = killerFrom | killerTo;

						Zobrist toggle = zobrist::keys[killerPiece][killerFromSq];
						toggle        ^= zobrist::keys[killerPiece][killerToSq];
						
						bitboard toggleCastling(0);

						if (killerPiece == (ROOK | plr)) {
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>60];
							toggleCastling = castling & killerFrom;
							toggle ^= zobrist::castling[((castling ^ toggleCastling)*castlingsmagic)>>60];
						} else if (killerPiece == (KING | plr)){
							toggle ^= zobrist::castling[(castling*castlingsmagic)>>60];
							toggleCastling = castling & ~castlingc<plr>::deactrights;
							toggle ^= zobrist::castling[((castling ^ toggleCastling)*castlingsmagic)>>60];
						}

						int scoreD = 0;
						if (capturedPiece >= 0){
							halfmoves = 0;
							toggle ^= zobrist::keys[capturedPiece][killerToSq];
							scoreD = Value::piece[capturedPiece];
						} else {
							halfmoves = oldhm + 1;
						}
						auto toggleMove = [this, toggle, killerPiece, tf, capturedPiece, killerTo, toggleCastling](){
							Pieces[killerPiece] ^= tf;
							All_Pieces(plr)     ^= tf;
							zobr                ^= toggle;
							castling            ^= toggleCastling;
							if (capturedPiece >= 0){
								Pieces[capturedPiece] ^= killerTo;
								All_Pieces(!plr)      ^= killerTo;
							}
						};

						internal_move smove(tf, 0);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, -scoreD, [](){statistics(++cutOffByKillerMove);}, 0)) return sst.score;
						halfmoves = 0;
					}
				} else {
					int promSp = getTTMove_Prom_spec(killerMove);
					int toPiece = getTTMove_fromPS_P(promSp);
					if ((toPiece & colormask) == plr){
						int diff = killerToSq - killerFromSq;
						bitboard tf = killerFrom ^ killerTo;
						//-7 = 1..11 1001, -8 = 1..11 1000, -9 1..11 0111
						if (diff & 1) { //capture
							int capturedPiece, capturedSq;
							bitboard capturedPos;
							int killerMoveOk = true;
							if (promSp <= 0xF) {
								capturedPiece = QUEEN | (!plr);
								while ((capturedPiece >= 0) && ((Pieces[capturedPiece] & killerTo) == 0)) capturedPiece -= 2;
								if (capturedPiece < 0){
									statistics(++ttError_Type1_SameHashKey);
									killerMoveOk = false;
								} else {
									capturedPos = killerTo;
									capturedSq = killerToSq;
								}
							} else if ((tmpEnPassant == ((plr == white) ? (killerTo >> 8) : (killerTo << 8)))
									&& (Pieces[PAWN | (!plr)] & tmpEnPassant)
									&& ((All_Pieces(plr) & killerTo) == 0)){
								capturedPos = tmpEnPassant;
								capturedPiece = PAWN | (!plr);
								capturedSq = (plr == white) ? (killerToSq - 8) : (killerToSq + 8);
								ASSUME(toPiece == (PAWN | plr));
								ASSUME(Pieces[PAWN | (!plr)] & capturedPos);
								ASSUME(All_Pieces(!plr) & capturedPos);
							} else {
								statistics(++ttError_Type1_SameHashKey);
								killerMoveOk = false;
							}
							if (killerMoveOk) {
								ASSUME((toPiece & colormask) == plr);
								int scoreD = Value::piece[capturedPiece] + Value::piece[PAWN | plr] - Value::piece[toPiece];
								Zobrist toggle = zobrist::keys[capturedPiece][capturedSq];
								toggle ^= zobrist::keys[PAWN | plr][killerFromSq];
								toggle ^= zobrist::keys[toPiece][killerToSq];

								auto toggleMove = [this, toggle, capturedPiece, capturedPos, killerFrom, killerTo, toPiece, tf](){
									Pieces[capturedPiece] ^= capturedPos;
									Pieces[PAWN | plr]    ^= killerFrom;
									Pieces[toPiece]       ^= killerTo;
									All_Pieces( plr)      ^= tf;
									All_Pieces(!plr)      ^= capturedPos;
									zobr                  ^= toggle;
								};
								//ASSUME(Pieces[PAWN | plr] & killerFrom);
								//ASSUME((Pieces[toPiece] & killerTo)==0);
								internal_move smove(tf, promSp);
								
								if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, scoreD, [](){statistics(++cutOffByKillerMove);}, 0)) return sst.score;
							}
						} else {
							int scoreD = Value::piece[PAWN | plr] - Value::piece[toPiece];
							Zobrist toggle = zobrist::keys[PAWN | plr][killerFromSq];
							toggle        ^= zobrist::keys[toPiece][killerToSq];
							if (killerToSq == (killerFromSq + ((plr == white) ? 16 : -16))){
								toggle   ^= zobrist::enPassant[7&(killerFromSq + ((plr == white) ? 8 : -8))];
								enPassant = ((plr == white) ? (killerFrom << 8) : (killerFrom >> 8));
							}
							ASSUME(Pieces[PAWN | plr] & killerFrom);
							ASSUME(toPiece >= (PAWN | white));
							ASSUME(toPiece <= (KING | black));
							ASSUME((toPiece & colormask) == plr);
							ASSUME((Pieces[toPiece] & killerTo) == 0);

							auto toggleMove = [this, toggle, toPiece, killerFrom, killerTo, tf](){
								Pieces[PAWN | plr] ^= killerFrom;
								Pieces[toPiece]    ^= killerTo;
								All_Pieces(plr)    ^= tf;
								zobr               ^= toggle;
							};

							internal_move smove(tf, promSp);
							if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, scoreD, [](){statistics(++cutOffByKillerMove);}, 0)) return sst.score;
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
	unsigned long int kingSq = square(Pieces[KING | plr]);

	//TODO add heuristics
	bitboard checkedBy = kingIsAttackedBy<plr>(occ, kingSq);

	if (!checkedBy){
		bitboard nPinnedPawn = getNPinnedPawns<plr>(occ, kingSq);
		bitboard attacking[2] = {Pieces[PAWN | plr], Pieces[PAWN | plr]};
		int kingFL = 7 & kingSq;
		int kingRK = kingSq >> 3;
		int kingAD = kingFL+kingRK;
		kingFL ^= 7;
		int kingMD = kingFL+kingRK;	//7-(kingFL^7)+kingRK

		if (plr==white){
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

		for (int captured = QUEEN | (!plr); captured >= 0 ; captured-=2){
			int scoreD = - Value::piece[PAWN | plr] - Value::piece[captured];
			for (int diff = ((plr==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
				bitboard tmp = attacking[at] & Pieces[captured];

				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard from((plr==white)?(to >> diff):(to << -diff));
					bitboard tf = to | from;
					unsigned long int toSq = square(to);

					Zobrist toggle = zobrist::keys[captured][toSq];
					toggle        ^= zobrist::keys[PAWN | plr][toSq-diff];

					auto toggleGroupMove = [this, toggle, captured, to, tf, from](){
						Pieces[captured]   ^= to;
						Pieces[PAWN | plr] ^= from;
						All_Pieces( plr)   ^= tf;
						All_Pieces(!plr)   ^= to;
						zobr               ^= toggle;
					};

					toggleGroupMove();
					for (int prom = QUEEN | plr; prom > (PAWN | colormask) ; prom -= 2){
						scoreD += Value::piece[prom];

						auto toggleMove = [this, toggle, prom, to, toSq](){
							Pieces[prom] ^= to;
							zobr         ^= zobrist::keys[prom][toSq];
						};

						internal_move smove(tf, prom);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, scoreD, toggleGroupMove, 0)) return sst.score;

						scoreD -= Value::piece[prom];
					}
					toggleGroupMove();
				}
			}
		}

		attacking[0] = attacking[1] = Pieces[PAWN | plr];
		if (plr==white){
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

		for (int captured = QUEEN | (!plr); captured >= 0 ; captured-=2){
			int scoreGD = Value::piece[captured];
			pieceScore -= scoreGD;
			for (int diff = ((plr==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
				bitboard tmp = attacking[at] & Pieces[captured];
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard from((plr==white)?(to >> diff):(to << -diff));
					bitboard tf = to | from;
					unsigned long int toSq = square(to);
					Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
					toggle ^= zobrist::keys[PAWN | plr][toSq-diff];
					toggle ^= zobrist::keys[captured][toSq];

					auto toggleMove = [this, toggle, tf, to, captured](){
						Pieces[PAWN | plr] ^= tf;
						Pieces[captured]   ^= to;
						All_Pieces( plr)   ^= tf;
						All_Pieces(!plr)   ^= to;
						zobr               ^= toggle;
					};

					internal_move smove(tf, PAWN | plr);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, scoreGD)) return sst.score;
				}
			}
			pieceScore += scoreGD;
		}

		for (int diff = ((plr==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
			if ((attacking[at] & tmpEnPassant) != 0){
				bitboard tf = tmpEnPassant;
				bitboard cp = tmpEnPassant;
				if (plr == white){
					tf |= tmpEnPassant >> diff;
					cp >>= 8;
				} else {
					tf |= tmpEnPassant << -diff;
					cp <<= 8;
				}
				unsigned long int toSq = square(tmpEnPassant);
				Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
				toggle ^= zobrist::keys[PAWN |   plr ][toSq-diff];
				toggle ^= zobrist::keys[PAWN | (!plr)][toSq+((plr==white)?-8:8)];
				auto toggleMove = [this, toggle, tf, cp](){
					Pieces[PAWN |   plr ] ^= tf;
					Pieces[PAWN | (!plr)] ^= cp;
					All_Pieces( plr)      ^= tf;
					All_Pieces(!plr)      ^= cp;
					zobr                  ^= toggle;
				};

				internal_move smove(tf,  PAWN | plr | TTMove_EnPassantPromFlag);
				if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, -Value::piece[PAWN | (!plr)], [](){}, 0)) return sst.score;
			}
		}

		bitboard empty = ~occ;
		bitboard pawnsToForward = Pieces[PAWN | plr];
		if (mode < quiescenceMask){
			pawnsToForward &= nPinnedPawn | filled::file[kingFL];
			bitboard tmp = pawnsToForward;
			if (plr == white){
				tmp = lastRank_w & (tmp << 8);
			} else {
				tmp = lastRank_b & (tmp >> 8);
			}
			tmp &= empty;
			pieceScore -= Value::piece[PAWN | plr];
			while (tmp){
				bitboard to = pop_lsb(tmp);
				bitboard from = (plr == white) ? (to >> 8) : (to << 8);
				bitboard tf = to | from;
				unsigned long int toSq = square(to);

				auto toggleGroupMove = [this, tf, toSq, from](){
					All_Pieces(plr)    ^= tf;
					Pieces[PAWN | plr] ^= from;
					zobr ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-8:8)];
				};

				toggleGroupMove();
				for (int prom = QUEEN | plr; prom > (PAWN | colormask) ; prom -= 2){

					auto toggleMove = [this, prom, toSq, to](){
						Pieces[prom] ^= to;
						zobr ^= zobrist::keys[prom][toSq];
					};

					internal_move smove(tf, prom);
					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, Value::piece[prom], toggleGroupMove, Value::piece[PAWN | plr])) return sst.score;
				}
				toggleGroupMove();
			}
			pieceScore += Value::piece[PAWN | plr];
		}

#ifdef HYPERPOSITION
#define SIZE 64
#else
#define SIZE 16
#endif
		struct move_data{
			//bitboard frombb;
			bitboard attack;
			unsigned int fromSq;
			unsigned int piecet;
		} dt[SIZE];
		static_assert(sizeof(move_data) == 128/8, "move_data size");
		unsigned long int n(0);
#undef SIZE
		bitboard KAttack = KingMoves[kingSq];
		unsigned long int firstRook, firstQueen;
		{ 												//TargetSquareGenerator:
			bitboard tmp = Pieces[KNIGHT | plr];
			//TODO Only knights that are not pinned can move, so tmp's population is predictable from here
			while (tmp){
				bitboard xRay;
				bitboard frombb = pop_lsb(tmp);
				dt[n].fromSq = square(frombb);
				int dr = direction[kingSq][dt[n].fromSq];
				//A pinned knight has no legal moves.
				if (dr == WRONG_PIECE || (rays[kingSq][dt[n].fromSq] & occ) != 0 ||
						(((xRay = getChecker<plr>(occ, dt[n].fromSq, kingSq)) & Pieces[QUEEN | (!plr)]) == 0
								&& (xRay & Pieces[dr | (!plr)]) == 0)) {
					dt[n].attack = KnightMoves[dt[n].fromSq];
					dt[n].piecet = KNIGHT | plr;
					++n;
				}
			}
			//TODO A pinned piece can only move between the pinner and the king
			//Also a bishop can only move if he is pinned by a queen or bishop, if he is pinned
			//a rook can only move if he is pinned by a queen or rook, if he is pinned
			//the pinner will be capturable by the pinned piece!
			tmp = Pieces[BISHOP | plr];
			while (tmp){
				bitboard frombb = pop_lsb(tmp);
				dt[n].fromSq = square(frombb);
				dt[n].attack = bishopAttacks(occ, dt[n].fromSq);
				dt[n].piecet = BISHOP | plr;
				filterAttackBB<plr>(occ, dt[n].fromSq, dt[n].attack, kingSq);
				++n;
			}
			firstRook = n;
			tmp = Pieces[ROOK | plr];
			while (tmp){
				bitboard frombb = pop_lsb(tmp);
				dt[n].fromSq = square(frombb);
				dt[n].attack = rookAttacks(occ, dt[n].fromSq);
				dt[n].piecet = ROOK | plr;
				filterAttackBB<plr>(occ, dt[n].fromSq, dt[n].attack, kingSq);
				++n;
			}
			firstQueen = n;
			tmp = Pieces[QUEEN | plr];
			while (tmp){
				bitboard frombb = pop_lsb(tmp);
				dt[n].fromSq = square(frombb);
				dt[n].attack = queenAttacks(occ, dt[n].fromSq);
				dt[n].piecet = QUEEN | plr;
				filterAttackBB<plr>(occ, dt[n].fromSq, dt[n].attack, kingSq);
				++n;
			}
		}
		//frombb[n] = Pieces[KING | plr];
		//fromSq[n] = kingSq[plr];//square(frombb[n]);
		//attack[n] = KingMoves[kingSq[plr]]; KAttack
		//n : position of last bitboard generated
		if (!(castling & castlingrights[plr])){
			for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
				pieceScore -= Value::piece[captured];
				for (unsigned long int i = 0 ; i < n ; ++i) {
					bitboard tmp = Pieces[captured] & dt[i].attack;
					unsigned int fromSq = dt[i].fromSq;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						unsigned int mpiece = dt[i].piecet;
						Zobrist toggle = zobrist::keys[captured][toSq];
						toggle ^= zobrist::keys[mpiece][toSq];
						toggle ^= zobrist::keys[mpiece][fromSq];

						auto toggleMove = [this, toggle, tf, to, captured, mpiece](){
							Pieces[captured] ^= to;
							Pieces[mpiece]   ^= tf;
							All_Pieces( plr) ^= tf;
							All_Pieces(!plr) ^= to;
							zobr             ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, Value::piece[captured])) return sst.score;
					}
				}
				bitboard tmp = Pieces[captured] & KAttack;
				while (tmp){
					bitboard to = pop_lsb(tmp);
					unsigned long int nkSq = square(to);
					bitboard tf = to | Pieces[KING | plr];

					auto toggleGroupMove = [this, to, tf, captured](){
						Pieces[captured]   ^= to;
						Pieces[KING | plr] ^= tf;
						All_Pieces( plr)   ^= tf;
						All_Pieces(!plr)   ^= to;
					};
					toggleGroupMove();
					if (validPosition<plr>(nkSq)){
						Zobrist toggle = zobrist::keys[captured][nkSq];
						toggle        ^= zobrist::keys[KING | plr][kingSq];
						toggle        ^= zobrist::keys[KING | plr][nkSq];

						auto toggleMove = [this, toggle](){
							zobr ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[captured])) return sst.score;
					}
					toggleGroupMove();
				}
				pieceScore += Value::piece[captured];
			}
			if (mode < quiescenceMask){
				halfmoves = oldhm + 1;
				for (unsigned long int i = 0 ; i < n ; ++i) {
					bitboard tmp = dt[i].attack & empty;
					unsigned int fromSq = dt[i].fromSq;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						unsigned int mpiece = dt[i].piecet;
						Zobrist toggle = zobrist::keys[mpiece][toSq];
						toggle ^= zobrist::keys[mpiece][fromSq];

						auto toggleMove = [this, toggle, tf, mpiece](){
							All_Pieces(plr) ^= tf;
							Pieces[mpiece]  ^= tf;
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, 0)) return sst.score;
					}
				}
				bitboard tmp = KAttack & empty;
				while (tmp){
					bitboard to = pop_lsb(tmp);
					unsigned long int nkSq = square(to);
					bitboard tf = to | Pieces[KING | plr];

					auto toggleGroupMove = [this, tf](){
						Pieces[KING | plr] ^= tf;
						All_Pieces(plr)    ^= tf;
					};
					toggleGroupMove();
					if (validPosition<plr>(nkSq)){
						Zobrist toggle = zobrist::keys[KING | plr][kingSq];
						toggle ^= zobrist::keys[KING | plr][nkSq];
						
						auto toggleMove = [this, toggle](){
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
		} else {
			key ct = zobrist::castling[(castling*castlingsmagic)>>60];
			for (int captured = QUEEN | (!plr); captured >= 0 ; captured -= 2){
				unsigned int i = 0;
				pieceScore -= Value::piece[captured];
				for (; i < firstRook ; ++i) {
					bitboard tmp = Pieces[captured] & dt[i].attack;
					unsigned int fromSq = dt[i].fromSq;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						unsigned int mpiece = dt[i].piecet;
						Zobrist toggle = zobrist::keys[captured][toSq];
						toggle        ^= zobrist::keys[mpiece][toSq];
						toggle        ^= zobrist::keys[mpiece][fromSq];

						auto toggleMove = [this, toggle, captured, to, tf, mpiece](){
							Pieces[captured] ^= to;
							Pieces[mpiece]   ^= tf;
							All_Pieces(plr)  ^= tf;
							All_Pieces(!plr) ^= to;
							zobr             ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, Value::piece[captured])) return sst.score;
					}
				}
				for ( ; i < firstQueen ; ++i){
					unsigned int fromSq = dt[i].fromSq;
					bitboard toggleCastling = castling & (bitboard(1)<<fromSq);
					key ct2 = ct ^ zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
					
					auto toggleGroupMove = [this, toggleCastling, ct2](){
						castling ^= toggleCastling;
						zobr     ^= ct2;
					};

					toggleGroupMove();

					bitboard tmp = Pieces[captured] & dt[i].attack;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						Zobrist toggle = zobrist::keys[captured][toSq];
						toggle ^= zobrist::keys[ROOK | plr][toSq];
						toggle ^= zobrist::keys[ROOK | plr][fromSq];

						auto toggleMove = [this, toggle, captured, to, tf](){
							Pieces[captured]   ^= to;
							Pieces[ROOK | plr] ^= tf;
							All_Pieces( plr)   ^= tf;
							All_Pieces(!plr)   ^= to;
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[captured])) return sst.score;
					}
					toggleGroupMove();
				}
				for (; i < n ; ++i) {
					unsigned int fromSq = dt[i].fromSq;
					bitboard tmp = Pieces[captured] & dt[i].attack;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						Zobrist toggle = zobrist::keys[captured][toSq];
						toggle ^= zobrist::keys[QUEEN | plr][toSq];
						toggle ^= zobrist::keys[QUEEN | plr][fromSq];

						auto toggleMove = [this, toggle, captured, to, tf](){
							Pieces[captured] ^= to;
							Pieces[QUEEN | plr] ^= tf;
							All_Pieces(plr) ^= tf;
							All_Pieces(!plr) ^= to;
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, Value::piece[captured])) return sst.score;
					}
				}
				bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
				key ct2 = ct ^ zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
				bitboard tmp = Pieces[captured] & KAttack;
				while (tmp){
					bitboard to = pop_lsb(tmp);
					unsigned long int nkSq = square(to);
					bitboard tf = to | Pieces[KING | plr];

					auto toggleGroupMove = [this, to, tf, captured, ct2, toggleCastling](){
						Pieces[captured]   ^= to;
						Pieces[KING | plr] ^= tf;
						All_Pieces( plr)   ^= tf;
						All_Pieces(!plr)   ^= to;
						zobr               ^= ct2;
						castling           ^= toggleCastling;
					};

					toggleGroupMove();
					if (validPosition<plr>(nkSq)){
						Zobrist toggle = zobrist::keys[captured][nkSq];
						toggle ^= zobrist::keys[KING | plr][nkSq];
						toggle ^= zobrist::keys[KING | plr][kingSq];

						auto toggleMove = [this, toggle](){
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[captured])) return sst.score;
					}
					toggleGroupMove();
				}
				pieceScore += Value::piece[captured];
			}
			if (mode < quiescenceMask){
				halfmoves = oldhm + 1;
				if ((castling & (castlingc<plr>::KingSide) & Pieces[ROOK | plr]) && (castlingc<plr>::KingSideSpace & occ)==0 && notAttacked<!plr>(castlingc<plr>::KSCPassing, castlingc<plr>::KSCPassingSq) && validPosition<plr>(kingSq)){
					auto toggleGroupMove = [this](){
						Pieces[KING | plr] ^= castlingc<plr>::KSCKT;
						All_Pieces(plr)    ^= castlingc<plr>::KSCFT;
					};
					toggleGroupMove();
					if (validPosition<plr>(castlingc<plr>::kingSqAfterKSC)){
						bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
						Zobrist toggle = zobrist::keys[ROOK | plr][0+((plr==black)?56:0)];
						toggle ^= zobrist::keys[ROOK | plr][2+((plr==black)?56:0)];
						toggle ^= zobrist::keys[KING | plr][3+((plr==black)?56:0)];
						toggle ^= zobrist::keys[KING | plr][1+((plr==black)?56:0)];
						toggle ^= ct;
						toggle ^= zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];

						auto toggleMove = [this, toggle, toggleCastling](){
							Pieces[ROOK | plr] ^= castlingc<plr>::KSCRT;
							zobr               ^= toggle;
							castling           ^= toggleCastling;
						};

						internal_move smove(castlingc<plr>::KSCKT);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
				if ((castling & (castlingc<plr>::QueenSide) & Pieces[ROOK | plr])!=0 && (castlingc<plr>::QueenSideSpace & occ)==0 && notAttacked<!plr>(castlingc<plr>::QSCPassing, castlingc<plr>::QSCPassingSq) && validPosition<plr>(kingSq)){
					auto toggleGroupMove = [this](){
						Pieces[KING | plr] ^= castlingc<plr>::QSCKT;
						All_Pieces(plr)    ^= castlingc<plr>::QSCFT;
					};
					toggleGroupMove();
					if (validPosition<plr>(castlingc<plr>::kingSqAfterQSC)){
						bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
						Zobrist toggle = zobrist::keys[ROOK | plr][7+((plr==black)?56:0)];
						toggle ^= zobrist::keys[ROOK | plr][4+((plr==black)?56:0)];
						toggle ^= zobrist::keys[KING | plr][3+((plr==black)?56:0)];
						toggle ^= zobrist::keys[KING | plr][5+((plr==black)?56:0)];
						toggle ^= ct;
						toggle ^= zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];

						auto toggleMove = [this, toggle, toggleCastling](){
							Pieces[ROOK | plr] ^= castlingc<plr>::QSCRT;
							zobr               ^= toggle;
							castling           ^= toggleCastling;
						};

						internal_move smove(castlingc<plr>::QSCKT);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}

				unsigned int i = 0;
				for (; i < firstRook ; ++i) {
					unsigned int fromSq = dt[i].fromSq;
					bitboard tmp = dt[i].attack & empty;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						unsigned int mpiece = dt[i].piecet;
						Zobrist toggle = zobrist::keys[mpiece][toSq];
						toggle ^= zobrist::keys[mpiece][fromSq];

						auto toggleMove = [this, toggle, tf, mpiece](){
							All_Pieces(plr) ^= tf;
							Pieces[mpiece]  ^= tf;
							zobr            ^= toggle;
						};
						
						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, 0)) return sst.score;
					}
				}
				for (; i < firstQueen ; ++i) {
					unsigned int fromSq = dt[i].fromSq;
					bitboard toggleCastling = castling & (UINT64_C(1)<<fromSq);
					key ct2 = ct ^ zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
					
					auto toggleGroupMove = [this, toggleCastling, ct2](){
						zobr     ^= ct2;
						castling ^= toggleCastling;
					};
					toggleGroupMove();
					bitboard tmp = dt[i].attack & empty;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						Zobrist toggle = zobrist::keys[ROOK | plr][toSq];
						toggle        ^= zobrist::keys[ROOK | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							All_Pieces(plr)    ^= tf;
							Pieces[ROOK | plr] ^= tf;
							zobr               ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
				for (; i < n ; ++i) {
					unsigned int fromSq = dt[i].fromSq;
					bitboard tmp = dt[i].attack & empty;
					while (tmp){
						bitboard to = pop_lsb(tmp);
						bitboard tf = to | (UINT64_C(1) << fromSq);
						unsigned long int toSq = square(to);
						Zobrist toggle = zobrist::keys[QUEEN | plr][toSq];
						toggle ^= zobrist::keys[QUEEN | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							All_Pieces(plr)     ^= tf;
							Pieces[QUEEN | plr] ^= tf;
							zobr                ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, 0)) return sst.score;
					}
				}
				bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
				key ct2 = ct ^ zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
				
				bitboard tmp = KAttack & empty;
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard tf = to | Pieces[KING | plr];
					unsigned long int nkSq = square(to);

					auto toggleGroupMove = [this, tf, toggleCastling, ct2](){
						Pieces[KING | plr] ^= tf;
						All_Pieces(plr)    ^= tf;
						zobr               ^= ct2;
						castling           ^= toggleCastling;
					};
					toggleGroupMove();
					if (validPosition<plr>(nkSq)){
						Zobrist toggle = zobrist::keys[KING | plr][nkSq];
						toggle        ^= zobrist::keys[KING | plr][kingSq];

						auto toggleMove = [this, toggle](){
							zobr ^= toggle;
						};

						internal_move smove(tf);
						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
		}
		if (mode < quiescenceMask){
			halfmoves = 0;

			bitboard tmp = pawnsToForward;
			if (plr == white){
				tmp <<= 8;
				tmp &= notlastRank_w;
			} else {
				tmp >>= 8;
				tmp &= notlastRank_b;
			}
			tmp &= empty;
			while (tmp){
				bitboard to = pop_lsb(tmp), tf;
				if (plr == white){
					tf = to | (to >> 8);
				} else {
					tf = to | (to << 8);
				}
				unsigned long int toSq = square(to);
				Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
				toggle ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-8:8)];

				auto toggleMove = [this, toggle, tf](){
					All_Pieces(plr)    ^= tf;
					Pieces[PAWN | plr] ^= tf;
					zobr               ^= toggle;
				};

				internal_move smove(tf, PAWN | plr);
				if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, 0)) return sst.score;
			}
			tmp = pawnsToForward;
			if (plr == white){
				tmp = ( ( ( ( tmp & pstartRank_w ) << 8 ) & empty ) << 8 ) & empty;
			} else {
				tmp = ( ( ( ( tmp & pstartRank_b ) >> 8 ) & empty ) >> 8 ) & empty;
			}
			while (tmp){
				bitboard to = pop_lsb(tmp), tf;
				if (plr == white){
					tf = to | (to >> 16);
					enPassant = to >> 8;
				} else {
					tf = to | (to << 16);
					enPassant = to << 8;
				}
				unsigned long int toSq  = square(to);
				unsigned long int tmpSq = square(enPassant);
				Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
				toggle ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-16:16)];
				toggle ^= zobrist::enPassant[7&tmpSq];

				auto toggleMove = [this, toggle, tf](){
					All_Pieces(plr)    ^= tf;
					Pieces[PAWN | plr] ^= tf;
					zobr               ^= toggle;
				};

				internal_move smove(tf, PAWN | plr);
				if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, [](){}, 0)) return sst.score;
			}
			enPassant = bitboard(0);
		}
	} else {

		if (!( checkedBy & (checkedBy - 1) )){
			//1) Capturing the attacking piece
			unsigned long int toSq = square(checkedBy);
			int attacker = QUEEN | (!plr);
			while (!(Pieces[attacker] & checkedBy)) attacker -= 2;

			auto togglePartialMove = [this, attacker, toSq, checkedBy](){
				zobr             ^= zobrist::keys[attacker][toSq];
				Pieces[attacker] ^= checkedBy;
				All_Pieces(!plr) ^= checkedBy;
			};
			togglePartialMove();

			pieceScore -= Value::piece[attacker];
			if ((checkedBy & (plr==white?lastRank_w:lastRank_b)) == 0){
				for (int diff = (plr==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
					bitboard att = (plr == white) ? (checkedBy >> diff) : (checkedBy << -diff);
					att &= notFilled::file[f] & Pieces[PAWN | plr];
					if (att){
						assert(!(att & (att-1)));
						bitboard tf = checkedBy | att;

						auto togglePartial2Move = [this, tf](){
							All_Pieces(plr) ^= tf;
						};
						togglePartial2Move();
						if (validPositionNonChecked<plr>(kingSq)){
							Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
							toggle ^= zobrist::keys[PAWN | plr][toSq - diff];

							auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
								togglePartialMove();
								togglePartial2Move();
							};

							auto toggleMove = [this, toggle, tf](){
								Pieces[PAWN | plr] ^= tf;
								zobr               ^= toggle;
							};

							internal_move smove(tf, PAWN | plr);
							if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
						}
						togglePartial2Move();
					}
				}
				if (((plr==white)?(checkedBy<<8):(checkedBy>>8)) == tmpEnPassant){
					for (int diff = (plr==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
						bitboard att = (plr == white) ? (tmpEnPassant >> diff) : (tmpEnPassant << -diff);
						att &=  notFilled::file[f] & Pieces[PAWN | plr];
						if (att){
							assert(!(att & (att-1)));
							bitboard tf = tmpEnPassant | att;
							
							auto togglePartial2Move = [this, tf](){
								All_Pieces(plr) ^= tf;
							};
							togglePartial2Move();
							if (validPositionNonChecked<plr>(kingSq)){
								unsigned long int toenpsq = square(tmpEnPassant);
								Zobrist toggle = zobrist::keys[PAWN | plr][toenpsq];
								toggle ^= zobrist::keys[PAWN | plr][toenpsq - diff];

								auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
									togglePartialMove();
									togglePartial2Move();
								};

								auto toggleMove = [this, toggle, tf](){
									Pieces[PAWN | plr] ^= tf;
									zobr               ^= toggle;
								};

								internal_move smove(tf, PAWN | plr | TTMove_EnPassantPromFlag);
								if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
							}
							togglePartial2Move();
						}
					}
				}
			} else {
				for (int diff = (plr==white?7:-9), f = 7 ; f >= 0 ; diff += 2, f -= 7){
					bitboard att = (plr == white) ? (checkedBy >> diff) : (checkedBy << -diff);
					att &=  notFilled::file[f] & Pieces[PAWN | plr];
					if (att){
						assert(!(att & (att-1)));
						bitboard from = att;
						bitboard tf = checkedBy | from;

						auto togglePartial2Move = [this, tf](){
							All_Pieces(plr) ^= tf;
						};
						togglePartial2Move();
						if (validPositionNonChecked<plr>(kingSq)){
							auto togglePartial3Move = [this, from, toSq, diff](){
								Pieces[PAWN | plr] ^= from;
								zobr ^= zobrist::keys[PAWN | plr][toSq - diff];
							};
							auto toggleGroupMove = [togglePartialMove, togglePartial2Move, togglePartial3Move](){
								togglePartialMove();
								togglePartial2Move();
								togglePartial3Move();
							};

							pieceScore -= Value::piece[PAWN | plr];
							togglePartial3Move();
							for (int prom = QUEEN | plr; prom > (PAWN | colormask) ; prom -= 2){
								auto toggleMove = [this, toSq, prom, checkedBy](){
									Pieces[prom] ^= checkedBy;
									zobr         ^= zobrist::keys[prom][toSq];
								};

								internal_move smove(tf, prom);

								if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, Value::piece[prom], toggleGroupMove, Value::piece[attacker]+Value::piece[PAWN | plr])) return sst.score;
							}
							togglePartial3Move();
							pieceScore += Value::piece[PAWN | plr];
						}
						togglePartial2Move();
					}
				}
			}
			bitboard tmp = Pieces[KNIGHT | plr] & KnightMoves[toSq];
			while (tmp){
				bitboard from = pop_lsb(tmp);
				bitboard tf = from | checkedBy;

				auto togglePartial2Move = [this, tf](){
					All_Pieces(plr) ^= tf;
				};
				togglePartial2Move();

				if (validPositionNonChecked<plr>(kingSq)){
					unsigned long int fromSq = square(from);
					Zobrist toggle = zobrist::keys[KNIGHT | plr][fromSq];
					toggle        ^= zobrist::keys[KNIGHT | plr][toSq];

					auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
						togglePartialMove();
						togglePartial2Move();
					};

					auto toggleMove = [this, toggle, tf](){
						Pieces[KNIGHT | plr] ^= tf;
						zobr                 ^= toggle;
					};

					internal_move smove(tf);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
				}
				togglePartial2Move();
			}
			tmp = Pieces[BISHOP | plr] & bishopAttacks(occ, toSq);
			while (tmp){
				bitboard from = pop_lsb(tmp);
				bitboard tf = from | checkedBy;
				auto togglePartial2Move = [this, tf](){
					All_Pieces(plr) ^= tf;
				};
				togglePartial2Move();
				if (validPositionNonChecked<plr>(kingSq)){
					unsigned long int fromSq = square(from);
					Zobrist toggle = zobrist::keys[BISHOP | plr][fromSq];
					toggle        ^= zobrist::keys[BISHOP | plr][toSq];

					auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
						togglePartialMove();
						togglePartial2Move();
					};

					auto toggleMove = [this, toggle, tf](){
						Pieces[BISHOP | plr] ^= tf;
						zobr                 ^= toggle;
					};

					internal_move smove(tf);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
				}
				togglePartial2Move();
			}
			tmp = Pieces[ROOK | plr] & rookAttacks(occ, toSq);
			key ct = zobrist::castling[(castling*castlingsmagic)>>60];
			while (tmp){
				bitboard from = pop_lsb(tmp);
				bitboard tf = from | checkedBy;
				auto togglePartial2Move = [this, tf, ct](){
					All_Pieces(plr) ^= tf;
					zobr            ^= ct;
				};
				togglePartial2Move();
				if (validPositionNonChecked<plr>(kingSq)){
					bitboard toggleCastling = castling & from;
					unsigned long int fromSq = square(from);
					Zobrist toggle = zobrist::keys[ROOK | plr][fromSq];
					toggle        ^= zobrist::keys[ROOK | plr][toSq];
					toggle ^= zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];

					auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
						togglePartialMove();
						togglePartial2Move();
					};

					auto toggleMove = [this, toggle, toggleCastling, tf](){
						Pieces[ROOK | plr] ^= tf;
						zobr               ^= toggle;
						castling           ^= toggleCastling;
					};

					internal_move smove(tf);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
				}
				togglePartial2Move();
			}
			tmp = Pieces[QUEEN | plr] & queenAttacks(occ, toSq);
			while (tmp){
				bitboard from = pop_lsb(tmp);
				bitboard tf   = from | checkedBy;
				auto togglePartial2Move = [this, tf, ct](){
					All_Pieces(plr) ^= tf;
				};
				togglePartial2Move();
				if (validPositionNonChecked<plr>(kingSq)){
					Zobrist toggle = zobrist::keys[QUEEN | plr][square(from)];
					toggle        ^= zobrist::keys[QUEEN | plr][toSq];

					auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
						togglePartialMove();
						togglePartial2Move();
					};

					auto toggleMove = [this, toggle, tf](){
						Pieces[QUEEN | plr] ^= tf;
						zobr                ^= toggle;
					};

					internal_move smove(tf);
					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, Value::piece[attacker])) return sst.score;
				}
				togglePartial2Move();
			}
			pieceScore += Value::piece[attacker];
			togglePartialMove();
			//2) Block it if it is a ray piece
			//ray is a subset of empty
			unsigned long int tmpSq = square(Pieces[KING | plr]);
			bitboard ray = rays[tmpSq][toSq];
			/**
			 * FIXME bug
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
				if (plr == white){
					cp >>= 8;
				} else {
					cp <<= 8;
				}
				for (int diff = ((plr==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
					if (plr == white){
						attacker = tmpEnPassant >> diff;
					} else {
						attacker = tmpEnPassant << -diff;
					}
					if ((attacker & Pieces[PAWN | plr]) != 0){
						bitboard tf = tmpEnPassant | attacker;

						auto toggleGroupMove = [this, tf, cp](){
							Pieces[PAWN | plr] ^= tf;
							Pieces[PAWN | (!plr)] ^= cp;
							All_Pieces(plr) ^= tf;
							All_Pieces(!plr) ^= cp;
						};
						toggleGroupMove();
						if (validPositionNonChecked<plr>(kingSq)){
							Zobrist toggle = zobrist::keys[PAWN | plr][tmpSq2];
							toggle ^= zobrist::keys[PAWN | plr][tmpSq2-diff];
							toggle ^= zobrist::keys[PAWN | (!plr)][tmpSq2+(plr==white)?-8:8];
							
							auto toggleMove = [this, toggle](){
								zobr ^= toggle;
							};

							internal_move smove(tf, PAWN | plr | TTMove_EnPassantPromFlag);

							if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, Value::piece[PAWN | (!plr)], toggleGroupMove, 0)) return sst.score;
						}
						toggleGroupMove();
					}
				}
			}
#endif
			bitboard tmpP;
			bitboard tmp2 = Pieces[PAWN | plr];
			tmp = ray;
			if (plr == white){
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
				bitboard to = pop_lsb(tmp2);
				bitboard tf = to;
				if (plr == white){
					tf |= (tf >> 16);
					enPassant = to >> 8;
				} else {
					tf |= (tf << 16);
					enPassant = to << 8;
				}
				auto toggleGroupMove = [this, tf](){
					All_Pieces(plr) ^= tf;
				};
				toggleGroupMove();
				if (validPositionNonChecked<plr>(kingSq)) {
					toSq  = square(to);
					tmpSq = square(enPassant);
					Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
					toggle ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-16:16)];
					toggle ^= zobrist::enPassant[7&tmpSq];

					auto toggleMove = [this, toggle, tf](){
						Pieces[PAWN | plr] ^= tf;
						zobr               ^= toggle;
					};

					internal_move smove(tf, PAWN | plr);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
				}
				toggleGroupMove();
			}
			enPassant = 0;
			while (tmpP){
				bitboard to = pop_lsb(tmpP);
				bitboard from = (plr == white) ? (to >> 8) : (to << 8);
				bitboard tf = to | from;

				auto togglePartialMove = [this, tf](){
					All_Pieces(plr) ^= tf;
				};
				togglePartialMove();
				if (validPositionNonChecked<plr>(kingSq)) {
					toSq = square(to);
					auto togglePartial2Move = [this, from, toSq](){
						Pieces[PAWN | plr] ^= from;
						zobr ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-8:8)];
					};
					togglePartial2Move();
					auto toggleGroupMove = [togglePartialMove, togglePartial2Move](){
						togglePartialMove();
						togglePartial2Move();
					};
					pieceScore -= Value::piece[PAWN | plr];
					for (int prom = QUEEN | plr; prom > (PAWN | colormask) ; prom -= 2){
						auto toggleMove = [this, prom, toSq, to](){
							Pieces[prom] ^= to;
							zobr         ^= zobrist::keys[prom][toSq];
						};

						internal_move smove(tf, prom);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, Value::piece[prom], toggleGroupMove, Value::piece[PAWN | plr])) return sst.score;
					}
					pieceScore += Value::piece[PAWN | plr];
					togglePartial2Move();
				}
				togglePartialMove();
			}
			while (tmp){
				bitboard to = pop_lsb(tmp);
				bitboard tf = to | ((plr == white) ? (to >> 8) : (to << 8));
				auto toggleGroupMove = [this, tf](){
					All_Pieces(plr) ^= tf;
				};
				toggleGroupMove();
				if (validPositionNonChecked<plr>(kingSq)) {
					toSq = square(to);
					Zobrist toggle = zobrist::keys[PAWN | plr][toSq];
					toggle ^= zobrist::keys[PAWN | plr][toSq+((plr==white)?-8:8)];

					auto toggleMove = [this, toggle, tf](){
						Pieces[PAWN | plr] ^= tf;
						zobr               ^= toggle;
					};

					internal_move smove(tf, PAWN | plr);
					
					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
				}
				toggleGroupMove();
			}
			halfmoves = oldhm + 1;
			tmpP = Pieces[KNIGHT | plr];
			while (tmpP){
				bitboard from = pop_lsb(tmpP);
				unsigned long int fromSq = square(from);
				tmp = ray & KnightMoves[fromSq];
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard tf = to | from;
					auto toggleGroupMove = [this, tf](){
						All_Pieces(plr) ^= tf;
					};
					toggleGroupMove();
					if (validPositionNonChecked<plr>(kingSq)){
						toSq = square(to);
						Zobrist toggle = zobrist::keys[KNIGHT | plr][toSq];
						toggle        ^= zobrist::keys[KNIGHT | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							Pieces[KNIGHT | plr] ^= tf;
							zobr                 ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
			tmpP = Pieces[BISHOP | plr];
			while (tmpP){
				bitboard from = pop_lsb(tmpP);
				unsigned long int fromSq = square(from);
				tmp = ray & bishopAttacks(occ, fromSq);
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard tf = to | from;
					auto toggleGroupMove = [this, tf](){
						All_Pieces(plr) ^= tf;
					};
					toggleGroupMove();
					if (validPositionNonChecked<plr>(kingSq)){
						toSq = square(to);
						Zobrist toggle = zobrist::keys[BISHOP | plr][toSq];
						toggle ^= zobrist::keys[BISHOP | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							Pieces[BISHOP | plr] ^= tf;
							zobr                 ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
			tmpP = Pieces[ROOK | plr];
			//Rooks in corners can not get into ray, so changing castling rights is useless
			//as rooks will never be in a position where they have castling right.
			while (tmpP){
				bitboard from = pop_lsb(tmpP);
				unsigned long int fromSq = square(from);
				tmp = ray & rookAttacks(occ, fromSq);
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard tf = to | from;
					auto toggleGroupMove = [this, tf](){
						All_Pieces(plr) ^= tf;
					};
					toggleGroupMove();
					if (validPositionNonChecked<plr>(kingSq)){
						toSq = square(to);
						Zobrist toggle = zobrist::keys[ROOK | plr][toSq];
						toggle ^= zobrist::keys[ROOK | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							Pieces[ROOK | plr] ^= tf;
							zobr               ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
			tmpP = Pieces[QUEEN | plr];
			while (tmpP){
				bitboard from = pop_lsb(tmpP);
				unsigned long int fromSq = square(from);
				tmp = ray & queenAttacks(occ, fromSq);
				while (tmp){
					bitboard to = pop_lsb(tmp);
					bitboard tf = to | from;
					auto toggleGroupMove = [this, tf](){
						All_Pieces(plr) ^= tf;
					};
					toggleGroupMove();
					if (validPositionNonChecked<plr>(kingSq)){
						toSq = square(to);
						Zobrist toggle = zobrist::keys[QUEEN | plr][toSq];
						toggle ^= zobrist::keys[QUEEN | plr][fromSq];

						auto toggleMove = [this, toggle, tf](){
							Pieces[QUEEN | plr] ^= tf;
							zobr ^= toggle;
						};

						internal_move smove(tf);

						if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
					}
					toggleGroupMove();
				}
			}
		}
		//3) Move the king
		halfmoves = 0;
		bitboard from = Pieces[KING | plr];
		unsigned long int fromSq = square(from);
		bitboard mv = KingMoves[fromSq];
		bitboard tmp1 = mv;
		key ct = zobrist::castling[(castling*castlingsmagic)>>60];
		bitboard toggleCastling = castling & ~castlingc<plr>::deactrights;
		ct ^= zobrist::castling[((castling^toggleCastling)*castlingsmagic)>>60];
		for (int attacker = QUEEN | (!plr); attacker >= 0 ; attacker -= 2){
			bitboard tmp = Pieces[attacker] & tmp1;
			while (tmp){
				bitboard to = pop_lsb(tmp);
				kingSq      = square(to);
				bitboard tf = from | to;
				auto toggleGroupMove = [this, tf, to, ct, attacker, toggleCastling](){
					Pieces[attacker]   ^= to;
					Pieces[KING | plr] ^= tf;
					All_Pieces( plr)   ^= tf;
					All_Pieces(!plr)   ^= to;
					castling           ^= toggleCastling;
					zobr               ^= ct;
				};
				toggleGroupMove();
				if (validPosition<plr>(kingSq)){
					Zobrist toggle = zobrist::keys[KING | plr][fromSq];
					toggle ^= zobrist::keys[KING | plr][kingSq];
					toggle ^= zobrist::keys[attacker][kingSq];

					auto toggleMove = [this, toggle](){
						zobr     ^= toggle;
					};

					internal_move smove(tf);

					if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, -Value::piece[attacker], toggleGroupMove, 0)) return sst.score;
				}
				toggleGroupMove();
			}
		}
		halfmoves = oldhm + 1;
		bitboard tmp = mv;
		tmp &= ~occ;
		while (tmp){
			bitboard to = pop_lsb(tmp);
			bitboard tf = to | from;
			kingSq      = square(to);
			auto toggleGroupMove = [this, tf, ct, toggleCastling](){
				Pieces[KING | plr] ^= tf;
				All_Pieces(plr)    ^= tf;
				castling           ^= toggleCastling;
				zobr               ^= ct;
			};
			toggleGroupMove();
			if (validPosition<plr>(kingSq)){
				Zobrist toggle = zobrist::keys[KING | plr][kingSq];
				toggle        ^= zobrist::keys[KING | plr][fromSq];

				auto toggleMove = [this, toggle](){
					zobr ^= toggle;
				};
				
				internal_move smove(tf);

				if (deeper<mode, plr, root>(smove, oldhm, tmpEnPassant, sst, toggleMove, 0, toggleGroupMove, 0)) return sst.score;
			}
			toggleGroupMove();
		}
	}
	while (board_interface->collectNextScore(sst.score, thread_id));
	halfmoves = oldhm;
	if (!root) {
		playing = !plr;
		zobr   ^= zobrist::blackKey;
	}
	enPassant = tmpEnPassant;
	if (enPassant) zobr ^= zobrist::enPassant[7 & square(enPassant)];
	if (plr==black) --fullmoves;
	if (mode == Perft && sst.depth == dividedepth) {
		U64 moves = stats.horizonNodes;
		moves -= stHorNodes;
#ifdef DIVIDEPERFT
		DWORD bytes_read, bytes_written;
		CHAR buffer[4096];
		// Write a message to the child process
		WriteFile(child_input_write, ("setboard "+getFEN(plr)+"\n").c_str(), ("setboard "+getFEN(plr)+"\n").length(), &bytes_written, NULL);
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
			std::cout << pre << getFEN(plr) << '\t' << moves << "\tFailed!\t" << a << '\n';
			std::cout << "-----------------------------------------\n";
			std::string oldpre = pre;
			pre += "\t";
			dividedepth = depth-1;
			search<mode, plr>(alpha, beta, depth);
			std::cout << "-----------------------------------------" << std::endl;
			pre = oldpre;
			dividedepth = depth;
		}
#else
		// std::cout << pre << getFEN(plr) << '\t' << moves << std::endl;
#endif
	}
	if (mode == Perft) return sst.alpha + 1;
	if (!interruption_requested){
		if (mode < quiescenceMask){
			if (stNodes == stats.nodes){
				if (checkedBy == bitboard(0)) {
					sst.alpha = 0;								//PAT
				} else {
					sst.alpha = -Value::MAT+rootDepth-sst.depth;	//MATed
				}
				addTTEntry<ExactScore>(zobr, sst.depth, 0,sst.alpha);//ExactScore
			} else if (sst.alpha != alpha) {
				assert(sst.bmove.tf != bitboard(0));
				addTTEntry<AlphaCutoff>(zobr, sst.depth, getMove<plr>(sst.bmove.tf, sst.bmove.prom), sst.alpha);
			} else {
				assert(sst.bmove.tf == bitboard(0));//ExactScore
				addTTEntry<mode == ZW ? AlphaCutoff : ExactScore>(zobr, sst.depth, (sst.bmove.tf == bitboard(0)) ? killerMove : getMove<plr>(sst.bmove.tf, sst.bmove.prom), sst.alpha);
			}
		} else {
			addTTEntry<QSearchAlphaCutoff>(zobr, sst.depth, (sst.bmove.tf == bitboard(0)) ? killerMove : getMove<plr>(sst.bmove.tf, sst.bmove.prom), sst.alpha);
		}
	}
	return sst.alpha;
}

template<color plr> inline int Board::getMove(bitboard tf, int prom) __restrict{
	ASSUME(tf != bitboard(0));
	ASSUME(prom < (TTMove_EnPassantPromFlag << 1));
	ASSUME((tf & All_Pieces(plr)) != bitboard(0));
	ASSUME((tf & (~All_Pieces(plr))) != bitboard(0));
	unsigned long int fromSq = square(tf &   All_Pieces(plr) );
	unsigned long int toSq   = square(tf & (~All_Pieces(plr)));
	ASSUME(0 <= fromSq && fromSq < 64);
	ASSUME(0 <= toSq && toSq < 64);
	if (tf & Pieces[PAWN | plr]) {
		ASSUME(plr == (prom & colormask));
		return getTTMoveFormat(fromSq, toSq, prom);
	}
	return getTTMoveFormat(fromSq, toSq, 0);
}

template<color plr> bool Board::stalemate() __restrict{
	bitboard occ = All_Pieces(white);
	occ |= All_Pieces(black);
	bitboard empty = ~occ;
	bitboard moving = empty;
	bool res = false;
	if (plr==white){
		moving &= Pieces[PAWN | plr] << 8;
	} else {
		moving &= Pieces[PAWN | plr] >> 8;
	}
	bitboard moving2 = moving;
	bitboard to, tf;
	unsigned long int kingSq = square(Pieces[KING | plr]);
	while (moving){
		to = pop_lsb(moving);
		tf = to | ((plr == white) ? (to >> 8) : (to << 8));
		if (validPosition<plr>(occ ^ tf, kingSq)) return false;
	}
	if (plr==white){
		moving2 <<= 8;
		moving2 &= dfRank_w;
	} else {
		moving2 >>= 8;
		moving2 &= dfRank_b;
	}
	moving2 &= empty;
	while (moving2){
		to = pop_lsb(moving2);
		tf = to | ((plr == white) ? (to >> 16) : (to << 16));
		if (validPosition<plr>(occ ^ tf, kingSq)) return false;
	}
	bitboard from = Pieces[KING | plr];
	bitboard att = KingMoves[square(from)];
	moving = empty & att;
	occ ^= from;
	while (moving){
		to = pop_lsb(moving);
		if (notAttacked<!plr>(to, occ^to, square(to))) {
			occ ^= from;
			return false;
		}
	}
	for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
		moving = Pieces[captured] & att;
		while (moving){
			to = pop_lsb(moving);
			Pieces[captured] ^= to;
			res = notAttacked<!plr>(to, occ, square(to));
			Pieces[captured] ^= to;
			if (res) {
				occ ^= from;
				return false;
			}
		}
	}
	occ ^= from;
	bitboard temp = Pieces[KNIGHT | plr];
	while (temp){
		from = pop_lsb(temp);
		att = KnightMoves[square(from)];
		moving = empty & att;
		while (moving){
			to = pop_lsb(moving);
			tf = to | from;
			if (validPosition<plr>(occ ^ tf, kingSq)) return false;
		}
		for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving){
				to = pop_lsb(moving);
				Pieces[captured] ^= to;
				res = validPosition<plr>(occ ^ from, kingSq);
				Pieces[captured] ^= to;
				if (res) return false;
			}
		}
	}
	temp = Pieces[BISHOP | plr];
	while (temp){
		from = pop_lsb(temp);
		att = bishopAttacks(occ, square(from));
		moving = empty & att;
		while (moving){
			to = pop_lsb(moving);
			tf = to | from;
			if (validPosition<plr>(occ ^ tf, kingSq)) return false;
		}
		for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving){
				to = pop_lsb(moving);
				Pieces[captured] ^= to;
				res = validPosition<plr>(occ ^ from, kingSq);
				Pieces[captured] ^= to;
				if (res) return false;
			}
		}
	}
	temp = Pieces[ROOK | plr];
	while (temp){
		from = pop_lsb(temp);
		att = rookAttacks(occ, square(from));
		moving = empty & att;
		while (moving){
			to = pop_lsb(moving);
			tf = to | from;
			if (validPosition<plr>(occ ^ tf, kingSq)) return false;
		}
		for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving){
				to = pop_lsb(moving);
				Pieces[captured] ^= to;
				res = validPosition<plr>(occ ^ from, kingSq);
				Pieces[captured] ^= to;
				if (res) return false;
			}
		}
	}
	temp = Pieces[QUEEN | plr];
	while (temp){
		from = pop_lsb(temp);
		att = queenAttacks(occ, square(from));
		moving = empty & att;
		while (moving){
			to = pop_lsb(moving);
			tf = to | from;
			if (validPosition<plr>(occ ^ tf, kingSq)) return false;
		}
		for (int captured = QUEEN | (!plr) ; captured >= 0 ; captured -= 2){
			moving = Pieces[captured] & att;
			while (moving){
				to = pop_lsb(moving);
				Pieces[captured] ^= to;
				res = validPosition<plr>(occ ^ from, kingSq);
				Pieces[captured] ^= to;
				if (res) return false;
			}
		}
	}
	bitboard attacking[2], attc;
	if (plr==white){
		attacking[0] = notfile0 & (Pieces[PAWN | plr] << 7);
		attacking[1] = notfile7 & (Pieces[PAWN | plr] << 9);
	} else {
		attacking[0] = notfile0 & (Pieces[PAWN | plr] >> 9);
		attacking[1] = notfile7 & (Pieces[PAWN | plr] >> 7);
	}
	for (int captured = QUEEN | (!plr); captured >= 0 ; captured-=2){
		for (int diff = ((plr==white)?7:-9), at = 0; at < 2 ; diff += 2, ++at){
			attc = attacking[at] & Pieces[captured];
			while (attc){
				to = pop_lsb(attc);
				from = (plr == white) ? (to >> diff) : (to << -diff);
				tf = to | from;
				Pieces[captured] ^= to;
				res = validPosition<plr>(occ ^ from, kingSq);
				Pieces[captured] ^= to;
				if (res) return false;
			}
		}
	}
	if (enPassant){
		if (plr == white){
			moving = (enPassant >> 9) | (enPassant >> 7);
		} else {
			moving = (enPassant << 9) | (enPassant << 7);
		}
		bitboard cpt;
		moving &= Pieces[PAWN | plr];
		while (moving){
			from = pop_lsb(moving);
			cpt = (plr == white) ? (enPassant >> 8) : (enPassant << 8);
			Pieces[PAWN | (!plr)] ^= cpt;
			res = validPosition<plr>(occ ^ cpt ^ from ^ enPassant, kingSq);
			Pieces[PAWN | (!plr)] ^= cpt;
			if (res) return false;
		}
	}
	//If castling was available, King would had a normal move as well!
	return true;
}

template<color plr> int Board::getEvaluation(int depth) __restrict{
	/**
	 * FIXME bad evaluation
	 * 8/8/8/8/6k1/R5p1/P1r3P1/5K2 b - - 21 71
	 * 3-fold, not promoting pawn because of negative values for pawns on sides (1-2 steps on endgame)
	 *
	 * Mate in 1 :
	 * 8/8/pppppppK/NBBR1NRp/nbbrqnrP/PPPPPPPk/8/Q7 w - - 0 1
	 */
	unsigned long int kingSqW, kingSqB;
	((plr == white) ? kingSqW : kingSqB) = square(Pieces[KING | plr]);
	if (stalemate<plr>()){
		if (validPosition<plr>(((plr == white) ? kingSqW : kingSqB))) return 0; //stalemate
		if (plr == white) return -Value::MAT+rootDepth-depth;	//White Mated
		return Value::MAT-rootDepth+depth;						//Black Mated
	}
	((plr == black) ? kingSqW : kingSqB) = square(Pieces[KING | plr]);
	int score = pieceScore;
	score += Value::kingSq[kingSqW] - Value::kingSq[kingSqB];
	bitboard knights = Pieces[KNIGHT | white];
	while (knights) score += Value::knightSq[square(pop_lsb(knights))];
	knights = Pieces[KNIGHT | black];
	while (knights) score -= Value::knightSq[square(pop_lsb(knights))];
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

constexpr bitboard Board::bishopAttacks(bitboard occ, const int sq){
// 	occ &= BishopMask[sq];
// 	occ *= BishopMagic[sq];
// #ifndef fixedShift
// 	occ >>= BishopShift[sq];
// #else
// 	occ >>= 64-maxBishopBits;
// #endif
// 	return BishopAttacks[sq][occ];
#ifndef fixedShift
	return BishopAttacks[sq][((occ & BishopData[sq].mask) * BishopData[sq].magic) >> BishopShift[sq]];
#else
	return BishopAttacks[sq][((occ & BishopData[sq].mask) * BishopData[sq].magic) >> (64-maxBishopBits)];
#endif
}

constexpr bitboard Board::rookAttacks(bitboard occ, const int sq){
// 	occ &= RookMask[sq];
// 	occ *= RookMagic[sq];
// #ifndef fixedShift
// 	occ >>= RookShift[sq];
// #else
// 	occ >>= 64-maxRookBits;
// #endif
// 	return RookAttacks[sq][occ];
#ifndef fixedShift
	return RookAttacks[sq][((occ & RookData[sq].mask) * RookData[sq].magic) >> RookShift[sq]];
#else
	return RookAttacks[sq][((occ & RookData[sq].mask) * RookData[sq].magic) >> (64-maxRookBits)];
#endif
}

constexpr bitboard Board::queenAttacks(bitboard occ, const int sq){
	return rookAttacks(occ, sq) | bishopAttacks(occ, sq);
}

template<color plr> inline void Board::filterAttackBB(bitboard occ, unsigned long int sq, bitboard &attack, int kingSq) __restrict{
	int dr = direction[kingSq][sq];
	bitboard ray = rays[kingSq][sq];
	if (dr != WRONG_PIECE && (ray & occ) == bitboard(0)){
		bitboard xRay = XRayOFCMask[kingSq][sq];
		bitboard ray2 = xRay;
		xRay &= occ;
		xRay *= XRayOFCMagic[kingSq][sq];
		xRay >>= 64 - maxCheckAvoidanceShiftBits;
		xRay = XRayOFCChecker[kingSq][sq][xRay];
		if ((xRay & Pieces[QUEEN | (!plr)]) != 0 || (xRay & Pieces[dr | (!plr)]) != 0) attack &= ray | ray2;
	}
}

template<color plr> inline bitboard Board::getNPinnedPawns(bitboard occ, int kingSq) __restrict{
	bitboard pinnedPawns = bitboard(0);
	bitboard tmp = rookAttacks(occ & ~rookAttacks(occ, kingSq), kingSq);
	tmp &= Pieces[ROOK | (!plr)] | Pieces[QUEEN | (!plr)];
	while (tmp) pinnedPawns |= rays[kingSq][square(pop_lsb(tmp))];
	tmp = bishopAttacks(occ & ~bishopAttacks(occ, kingSq), kingSq);
	tmp &= Pieces[BISHOP | (!plr)] | Pieces[QUEEN | (!plr)];
	while (tmp) pinnedPawns |= rays[kingSq][square(pop_lsb(tmp))];
	pinnedPawns &= Pieces[PAWN | plr];
	return (~pinnedPawns);
}

template<color plr> inline bitboard Board::getChecker(bitboard occ, unsigned long int sq, int kingSq) __restrict{
	occ &= XRayOFCMask[kingSq][sq];
	occ *= XRayOFCMagic[kingSq][sq];
	occ >>= 64 - maxCheckAvoidanceShiftBits;
	return XRayOFCChecker[kingSq][sq][occ];
}

inline bool Board::threefoldRepetition() __restrict{
	int h = (halfmoves >= lastHistoryEntry) ? (lastHistoryEntry & 1) : (lastHistoryEntry - (halfmoves & (~1)));
	for ( ; h < lastHistoryEntry ; h += 2) if (history[h] == zobr) return true;
	return false;
}

inline int Board::evaluatePawnStructure() __restrict{
	bitboard pawns = Pieces[PAWN | white];
//	bitboard emptyFiles = pawns;
	int pscore = 0;
	while (pawns) pscore += Value::WpawnSq[square(pop_lsb(pawns))];
	pawns = Pieces[PAWN | black];
//	emptyFiles |= pawns;
	while (pawns) pscore -= Value::BpawnSq[square(pop_lsb(pawns))];
	return pscore;
}

inline void Board::assert_state() const __restrict{
#ifndef NDEBUG
	bitboard all = 0;
	int score = 0;
	key z = 0;
	for (unsigned int i = PAWN | white ; i <= (KING | white) ; i += 2) {
		ASSUME(!(all & Pieces[i]));
		all |= Pieces[i];
		score += popCount(Pieces[i]) * Value::piece[i];
		bitboard t = Pieces[i];
		while(t){
			int s = square(pop_lsb(t));
			z ^= zobrist::keys[i][s];
		}
	}
	ASSUME(White_Pieces == all);
	all = 0;
	for (unsigned int i = PAWN | black ; i <= (KING | black) ; i += 2) {
		ASSUME(!(all & Pieces[i]));
		all |= Pieces[i];
		score += popCount(Pieces[i]) * Value::piece[i];
		bitboard t = Pieces[i];
		while(t){
			int s = square(pop_lsb(t));
			z ^= zobrist::keys[i][s];
		}
	}
	if (playing == BLACK) z ^= zobrist::blackKey;
	z ^= zobrist::castling[(castling*castlingsmagic)>>60];
	if (enPassant) z ^= zobrist::enPassant[7 & square(enPassant)];
	ASSUME(Black_Pieces == all);
	ASSUME(!(Black_Pieces & White_Pieces));
	ASSUME(score == this->pieceScore);
	ASSUME(z == zobr);
#endif
}

inline time_td get_current_time();
inline time_duration milli_to_time(U64 milli);
inline time_duration get_zero_time();
inline time_td get_factored_time(time_td el_time);

#endif /* BOARD_HPP_ */
