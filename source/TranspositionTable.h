/*
 * TranspositionTable.h
 *
 *  Created on: 2012/4/20
 *      Author: Chrysogelos Periklis
 */

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_
#include "zobristKeys.h"
#include "MoveEncoding.h"
#include <cassert>
//#include "Board.h"

#define TRANSPOSITION_TABLE_SIZE (0x7fffff)
#define clearDeeperKey 4

#define getTTIndex(x) ((x) % TRANSPOSITION_TABLE_SIZE)

struct ttEntry{
	U64 zobrXORdata;
	U64 data;
};

#define tte_bits_depth (8)
#define tte_bits_searchState (4)
#define tte_bits_killerMove TTMoveSize
#define tte_bits_score (22)

#define tte_shift_score (0)
#define tte_shift_killerMove (tte_shift_score + tte_bits_score)
#define tte_shift_searchState (tte_shift_killerMove + tte_bits_killerMove)
#define tte_shift_depth (tte_shift_searchState + tte_bits_searchState)

#define tte_mask_depth ((1ull << tte_bits_depth) - 1)
#define tte_mask_searchState ((1ull << tte_bits_searchState) - 1)
#define tte_mask_killerMove ((1ull << tte_bits_killerMove) - 1)
#define tte_mask_score ((1ull << tte_bits_score) - 1)

#define tte_getSearchState(x) (((x) >> tte_shift_searchState) & tte_mask_searchState)
#define tte_getKillerMove(x) (((x) >> tte_shift_killerMove) & tte_mask_killerMove)

#define ttExactScoreReturned (-1)

extern ttEntry transpositionTable[TRANSPOSITION_TABLE_SIZE];
extern unsigned int ttUsed;
statistics(extern U64 ttmisses);
statistics(extern U64 ttaccesses);
statistics(extern U64 hashHitCutOff);
statistics(extern U64 betaCutOff);
statistics(extern U64 cutOffByKillerMove);
statistics(extern U64 ttError_Type1_SameHashKey);

enum SearchState{
	ExactScore = 0,
	AlphaCutoff = 1,
	BetaCutoff = 2,
	QSearchAlphaCutoff = 3,
	QSearchBetaCutoff = 4
};

inline int tte_getDepth(U64 data){
	int d = (((data) >> tte_shift_depth) & tte_mask_depth);
	if (d & (1 << (tte_bits_depth-1))) d |= ~tte_mask_depth;
	return d;
}

inline int tte_getScore(U64 data){
	int sc = (((data) >> tte_shift_score) & tte_mask_score);
	if (sc & (1<<(tte_bits_score-1))) sc |= ~tte_mask_score;
	return sc;
}

template<SearchState state> inline bool replaceTTEntry(U64 data, int depth){
	return (tte_getDepth(data) < depth) || ((state <= 2) && (tte_getSearchState(data) > 2));
}

template<SearchState state> inline void addTTEntry(chapeiro::zobrist zobr, int depth, int move, int score){
	//if (state > 2) return;
	int index = getTTIndex(zobr);
	ttEntry * entry = transpositionTable + index;
	U64 data = entry->data;
	if (data == U64(0)){
		++ttUsed;
	} else if (!replaceTTEntry<state>(data, depth)) {
		return;
	}
	U64 newData = ((U64) state) << tte_shift_searchState;
	newData |= (((U64) depth) & tte_mask_depth) << tte_shift_depth;
	newData |= ((U64) move) << tte_shift_killerMove;
	newData |= (((U64) score) & tte_mask_score) << tte_shift_score;
	assert(tte_getDepth(newData) == depth);
	assert(tte_getKillerMove(newData) == move);
	assert(tte_getScore(newData) == score || boost::this_thread::interruption_requested());
	assert(tte_getSearchState(newData) == state);
	U64 newZXD = zobr ^ newData;
	entry->zobrXORdata = newZXD;
	entry->data = newData;
}

template<int mode> inline int retrieveTTEntry(chapeiro::zobrist zobr, int depth, int &alpha, int &beta){
	int index = getTTIndex(zobr);
	ttEntry * entry = transpositionTable + index;
	U64 zobrXD = entry->zobrXORdata;
	U64 data = entry->data;
	statistics(++ttaccesses);
	if ((data == U64(0)) || (zobrXD ^ data) != zobr) {
		statistics(++ttmisses);
		return NULL_MOVE;
	}
	if (depth > tte_getDepth(data)) return tte_getKillerMove(data);
	int ds = tte_getSearchState(data);
	int sc = tte_getScore(data);
	if (ds == ExactScore) {
		alpha = sc;
		beta = sc;
		return ttExactScoreReturned;
	}
	if (ds == AlphaCutoff){
		if (sc < beta) beta = sc;
	} else if (ds == BetaCutoff){
		if (sc > alpha) alpha = sc;
	} else if (mode >= quiescenceMask){
		if (ds == QSearchAlphaCutoff){
			if (sc < beta) beta = sc;
		} else {
			if (sc > alpha) alpha = sc;
		}
	}
	return tte_getKillerMove(data);
}

int getBestMove(chapeiro::zobrist zobr);
void ttNewGame();

#endif /* RANSPOSITIONTABLE_H_ */
