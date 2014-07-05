/*
 * HashTable.cpp
 *
 *  Created on: 2012/4/20
 *      Author: Chrysogelos Periklis
 */

#include "TranspositionTable.hpp"
#include <iostream>

ttEntry transpositionTable[TRANSPOSITION_TABLE_SIZE];
unsigned int ttUsed = 0;
statistics(U64 ttmisses = 0);
statistics(U64 ttaccesses = 0);
statistics(U64 hashHitCutOff = 0);
statistics(U64 betaCutOff = 0);
statistics(U64 cutOffByKillerMove = 0);
statistics(U64 ttError_Type1_SameHashKey = 0);

int getBestMove(chapeiro::zobrist zobr){
	int index = getTTIndex(zobr);
	ttEntry * entry = transpositionTable + index;
	U64 zxd = entry->zobrXORdata;
	U64 data = entry->data;
	if ((zxd ^ data) == zobr) return tte_getKillerMove(data);
	//std::cerr << "Hash Miss!!!" << std::endl;
	return 0;
}

void ttNewGame(){
	ttUsed = 0;
	for (int i = 0 ; i < TRANSPOSITION_TABLE_SIZE; ++i) {
		//if (tte_getDepth(transpositionTable[i].data) > clearDeeperKey){
			transpositionTable[i].zobrXORdata = 0;
			transpositionTable[i].data = 0;
		//}
	}
}

void ttPreparePVS(chapeiro::zobrist zobr){
	int index = getTTIndex(zobr);
	ttEntry * entry = transpositionTable + index;
	U64 zobrXD = entry->zobrXORdata;
	U64 data   = entry->data;
	if (data == U64(0)) return;
	if ((zobrXD ^ data) == zobr) return;
	entry->data        = U64(0);
	entry->zobrXORdata = U64(0);
}
