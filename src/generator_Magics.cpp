/*
 * GenerateMagics.cpp
 *
 *  Created on: May 25, 2011
 *      Author: periklis
 */
#include <iostream>
#include <stdio.h>
#include "randomc.h"
#include <stdlib.h>
#include <time.h>
#include "cchapeiro.h"
#include "SquareMapping.h"
//#define bf

using namespace std;
typedef chapeiro::bitboard bitboard;

bitboard leftOccluded(bitboard gen, bitboard pro) {
	pro &= 0XFEFEFEFEFEFEFEFEULL; // make H-File all occupied, to consider A-H-wraps after shift
	gen |= pro & (gen << 1);
	pro = pro & (pro << 1);
	gen |= pro & (gen << 2);
	pro = pro & (pro << 2);
	gen |= pro & (gen << 4);
	return gen;
}

bitboard rightOccluded(bitboard gen, bitboard pro) { //14
	pro &= 0X7F7F7F7F7F7F7F7FULL;
	gen |= pro & (gen >> 1);
	pro &= (pro >> 1);
	gen |= pro & (gen >> 2);
	pro &= (pro >> 2);
	gen |= pro & (gen >> 4);
	return gen;
}

bitboard southOccluded(bitboard gen, bitboard pro) {
	gen |= pro & (gen >> 8);
	pro &= (pro >> 8);
	gen |= pro & (gen >> 16);
	pro &= (pro >> 16);
	gen |= pro & (gen >> 32);
	return gen;
}

bitboard northOccluded(bitboard gen, bitboard pro) {
	gen |= pro & (gen << 8);
	pro &= (pro << 8);
	gen |= pro & (gen << 16);
	pro &= (pro << 16);
	gen |= pro & (gen << 32);
	return gen;
}

bitboard northLeftOccluded(bitboard gen, bitboard pro) {
	pro &= 0XFEFEFEFEFEFEFEFEULL;
	gen |= pro & (gen << 9);
	pro &= (pro << 9);
	gen |= pro & (gen << 18);
	pro &= (pro << 18);
	gen |= pro & (gen << 36);
	return gen;
}

bitboard southLeftOccluded(bitboard gen, bitboard pro) {
	pro &= 0XFEFEFEFEFEFEFEFEULL;
	gen |= pro & (gen >> 7);
	pro &= (pro >> 7);
	gen |= pro & (gen >> 14);
	pro &= (pro >> 14);
	gen |= pro & (gen >> 28);
	return gen;
}

bitboard southRightOccluded(bitboard gen, bitboard pro) {
	pro &= 0X7F7F7F7F7F7F7F7FULL;
	gen |= pro & (gen >> 9);
	pro &= (pro >> 9);
	gen |= pro & (gen >> 18);
	pro &= (pro >> 18);
	gen |= pro & (gen >> 36);
	return gen;
}

bitboard northRightOccluded(bitboard gen, bitboard pro) {
	pro &= 0X7F7F7F7F7F7F7F7FULL;
	gen |= pro & (gen << 7);
	pro &= (pro << 7);
	gen |= pro & (gen << 14);
	pro &= (pro << 14);
	gen |= pro & (gen << 28);
	return gen;
}

bitboard southOne(bitboard b) {
	return b >> 8;
}
bitboard northOne(bitboard b) {
	return b << 8;
}
bitboard leftOne(bitboard b) {
	return (b << 1) & 0XFEFEFEFEFEFEFEFEULL;
}
bitboard rightOne(bitboard b) {
	return (b >> 1) & 0X7F7F7F7F7F7F7F7FULL;
}
bitboard northLeftOne(bitboard b) {
	return (b << 9) & 0XFEFEFEFEFEFEFEFEULL;
}
bitboard southLeftOne(bitboard b) {
	return (b >> 7) & 0XFEFEFEFEFEFEFEFEULL;
}
bitboard southRightOne(bitboard b) {
	return (b >> 9) & 0X7F7F7F7F7F7F7F7FULL;
}
bitboard northRightOne(bitboard b) {
	return (b << 7) & 0X7F7F7F7F7F7F7F7FULL;
}

bitboard leftAttacks(bitboard rq, bitboard empty) {
	return leftOne(leftOccluded(rq, empty));
}
bitboard rightAttacks(bitboard rq, bitboard empty) {
	return rightOne(rightOccluded(rq, empty));
}
bitboard southAttacks(bitboard rq, bitboard empty) {
	return southOne(southOccluded(rq, empty));
}
bitboard northAttacks(bitboard rq, bitboard empty) {
	return northOne(northOccluded(rq, empty));
}
bitboard northLeftAttacks(bitboard bq, bitboard empty) {
	return northLeftOne(northLeftOccluded(bq, empty));
}
bitboard southLeftAttacks(bitboard bq, bitboard empty) {
	return southLeftOne(southLeftOccluded(bq, empty));
}
bitboard southRightAttacks(bitboard bq, bitboard empty) {
	return southRightOne(southRightOccluded(bq, empty));
}
bitboard northRightAttacks(bitboard bq, bitboard empty) {
	return northRightOne(northRightOccluded(bq, empty));
}

bitboard bishopMask(bitboard bishop) {
	bitboard southRight = southRightOccluded(bishop, ~0);
	southRight &= southRight << 9;
	bitboard southLeft = southLeftOccluded(bishop, ~0);
	southLeft &= southLeft << 7;
	bitboard northLeft = northLeftOccluded(bishop, ~0);
	northLeft &= northLeft >> 9;
	bitboard northRight = northRightOccluded(bishop, ~0);
	northRight &= northRight >> 7;
	return (~bishop) & (northRight | northLeft | southLeft | southRight);
}

bitboard rookMask(bitboard rook) {
	bitboard south = southOccluded(rook, ~0);
	south &= south << 8;
	bitboard north = northOccluded(rook, ~0);
	north &= north >> 8;
	bitboard left = leftOccluded(rook, ~0);
	left &= left >> 1;
	bitboard right = rightOccluded(rook, ~0);
	right &= right << 1;
	return (~rook) & (right | left | north | south);
}

bitboard indexToBitBoard(int index, int bits, bitboard mask) {
	bitboard result(0);
	for (int i = 0; i < bits; ++i) {
		if ((index & (1ULL << i)) == 0)
			result |= mask & (-mask);
		mask = mask & (mask - 1);
	}
	return result;
}

int population(bitboard b) {
	int c = 0;
	while (b != 0ULL) {
		b &= b - 1;
		++c;
	}
	return c;
}

bitboard bishopAttacks(bitboard bishop, bitboard empty) {
	return northRightAttacks(bishop, empty) | northLeftAttacks(bishop, empty)
			| southLeftAttacks(bishop, empty) | southRightAttacks(bishop, empty);
}

bitboard rookAttacks(bitboard rook, bitboard empty) {
	return leftAttacks(rook, empty) | rightAttacks(rook, empty) | southAttacks(
			rook, empty) | northAttacks(rook, empty);
}

bitboard getRandom(CRandomMersenne &rdm) {
	return (((bitboard) rdm.BRandom()) << 32) | rdm.BRandom();//rdm.BRandom(); // rand();
}

int getIndex(bitboard occ, int bits, bitboard magic) {
	return (int) ((occ * magic) >> (64 - bits));
}

void reverse(bitboard &v) {
	// swap odd and even bits
	v = ((v >> 1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF00FF00FFull) | ((v & 0x00FF00FF00FF00FFull) << 8);
	// swap 2-byte long pairs
	v = ((v >> 16) & 0x0000FFFF0000FFFFull) | ((v & 0x0000FFFF0000FFFFull) << 16);
	v = ((v >> 32) & 0x00000000FFFFFFFFull) | ((v & 0x00000000FFFFFFFFull) << 32);
}

void next(bitboard &a, const int &count) {
	if ((a & 1) != 0 && (a & (a + 1)) == 0) {
		a |= a + 1;
		reverse(a);
		cout << "//" << count << endl;
	} else {
		int bs(0);
		bitboard ls;
		bool rp;
		do {
			ls = a & (-a);
			a = a & (a - 1);
			ls >>= 1;
			rp = (ls == 0 || (ls & ((1ULL << bs) - 1)) != 0);
			if (rp) {
				++bs;
			} else {
				a |= ls;
			}
		} while (rp);
		if (bs != 0) {
			ls = a & (-a);
			do {
				ls >>= 1;
				a |= ls;
				--bs;
			} while (bs != 0);
		}
	}
}

bitboard generateMagics(int index, int bits, bool rook) {
#define maxLoopsForMagic 4
	bitboard piece = 1ULL << index;
	bitboard mask = rook?rookMask(piece):bishopMask(piece);
	bitboard occ[4096], att[4096], used[4096];
	bitboard magic;
	int count = 0;
	int n = population(mask);
	int max = 1 << n;
	bool fail = true;
	if (rook) {
		for (int i = 0; i < max; ++i) {
			occ[i] = indexToBitBoard(i, n, mask); //if bit j of i is 1, j of 1s of mask will be 1
			att[i] = rookAttacks(piece, ~(occ[i]));
		}
	} else {
		for (int i = 0; i < max; ++i) {
			occ[i] = indexToBitBoard(i, n, mask);
			att[i] = bishopAttacks(piece, ~(occ[i]));
		}
	}
#ifndef bf
	CRandomMersenne rdm(time(0));
#else
	magic = 1ULL << 63;
#endif
	do {
		++count;
#ifndef bf
#ifdef showInfos
		if ((count & 8388607)==0) cout << "\n//" << count << endl;
#endif
		magic = ~0ULL;
		for (int i = 0; i < maxLoopsForMagic; ++i) magic &= getRandom(rdm);
#else
		next(magic, count);
#endif
		if (population((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
		for (int i = 0; i < 4096; ++i) used[i] = 0ULL;
		fail = false;
		for (int i = 0; i < max; ++i) {
			int ind = getIndex(occ[i], bits, magic);
			if (used[ind] == 0ULL) {
				used[ind] = att[i];
			} else if (used[ind] != att[i]) {
				fail = true;
				break;
			}
		}
	} while (fail);
	return magic;
}

const int RookBits[64] = {
		12, 11, 11, 11, 11, 11, 11, 12,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		12, 11, 11, 11, 11, 11, 11, 12
};

const int BishopBits[64] = {
		6, 5, 5, 5, 5, 5, 5, 6,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		6, 5, 5, 5, 5, 5, 5, 6
};

void printgm(bitboard bb) {
	for (int y = 63; y >= 0; --y) {
		cout << ((bb & (1ULL << y)) >> y);
	}
	cout << endl;
}

bitboard RookMagicCalc[64];
bitboard BishopMagicCalc[64];
#ifdef _WIN32
#define format "%#018I64xULL"
#else
#define format "%#018llxULL"
#endif
void generate(const int bits[64], bool rook, int a, bitboard magic[64], FILE* out){
	for (int square = 0; square < 64; ++square) {
		fprintf(out, "\t");
#ifndef fixedShift
		magic[square] = generateMagics(square, min(bits[square], a), rook);
#else
		magic[square] = generateMagics(square, a, rook);
#endif
		fprintf(out, format, magic[square]);
		if (square != 63) fprintf(out, ",");
		fprintf(out, "\t//square : %02i\n", square);
	}
}

void MagicGenerator(int maxBitsRook, int maxBitsBishop, FILE* out) {
	char ct[20];
	time_t rawtime;
	time(&rawtime);
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	fprintf(out, "/**   Auto-generated file.\n");
	fprintf(out, " * Contains pre-computed data for CChapeiro.\n");
	fprintf(out, " * Generated : %s\n", ct);
	fprintf(out, " **/\n\n");
	fprintf(out, "#include \"cchapeiro.h\"\n");
	fprintf(out, "#include \"MagicsAndPrecomputedData.h\"\n\n");
	fprintf(out, "typedef chapeiro::bitboard bitboard;\n\n");
	fprintf(out, "const bitboard RookMagic[64] = {\n");
	generate(RookBits, true, maxBitsRook, RookMagicCalc, out);
	fprintf(out, "};\n\n");
	fprintf(out, "const bitboard BishopMagic[64] = {\n");
	generate(BishopBits, false, maxBitsBishop, BishopMagicCalc, out);
	fprintf(out, "};\n\n");
	fprintf(out, "const bitboard RookMask[64] = {");
	for (int i = 0; i < 64 ; ++i) {
		if (i%8==0) {
			fprintf(out, "\n\t");
		} else {
			fprintf(out, " ");
		}
		fprintf(out, format, rookMask(1ULL << i));
		if (i != 63) fprintf(out, ",");
	}
	fprintf(out, "\n};\n\n");
	fprintf(out, "const bitboard BishopMask[64] = {");
	for (int i = 0; i < 64 ; ++i) {
		if (i%8==0) {
			fprintf(out, "\n\t");
		} else {
			fprintf(out, " ");
		}
		fprintf(out, format, bishopMask(1ULL << i));
		if (i != 63) fprintf(out, ",");
	}
	fprintf(out, "\n};\n\n");
#ifndef fixedShift
	fprintf(out, "const int RookShift[64] = {");
	for (int i = 0; i < 64 ; ++i) {
		if (i%8==0) {
			fprintf(out, "\n\t");
		} else {
			fprintf(out, " ");
		}
		fprintf(out, "%02i", 64-RookBits[i]);
		if (i != 63) fprintf(out, ",");
	}
	fprintf(out, "\n};\n\n");
	fprintf(out, "const int BishopShift[64] = {");
	for (int i = 0; i < 64 ; ++i) {
		if (i%8==0) {
			fprintf(out, "\n\t");
		} else {
			fprintf(out, " ");
		}
		fprintf(out, "%02i", 64-BishopBits[i]);
		if (i != 63) fprintf(out, ",");
	}
	fprintf(out, "\n};\n\n");
#endif
	fprintf(out, "const bitboard RookAttacks[64][4096] = {\n#define _____________________ 0ULL\n");
	bitboard used[4096];
	for (int sq = 0 ; sq < 64 ; ++sq){
		for (int i = 0; i < 4096; ++i) used[i] = 0ULL;
		bitboard piece = 1ULL << sq;
		bitboard mask = rookMask(piece);
		int n = population(mask);
		int max = 1 << n;
		bitboard occ;
		for (int i = 0; i < max; ++i) {
			occ = indexToBitBoard(i, n, mask);
#ifndef fixedShift
			int ind = getIndex(occ, RookBits[sq], RookMagicCalc[sq]);
#else
			int ind = getIndex(occ, maxBitsRook, RookMagicCalc[sq]);
#endif
			if (used[ind] == 0ULL) used[ind] = rookAttacks(piece, ~occ);
		}
		fprintf(out, "{");
		for (int i = 0 ; i < 4096 ; ++i){
			if (i%8==0) {
				fprintf(out, "\n\t");
			} else {
				fprintf(out, " ");
			}
			if (used[i]!=0){
				fprintf(out, format, used[i]);
			} else {
				fprintf(out, "_____________________");
			}
			if (i != 4096) fprintf(out, ",");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "#undef _____________________\n};\n\n");
	fprintf(out, "const bitboard BishopAttacks[64][512] = {\n#define _____________________ 0ULL\n");
	for (int sq = 0 ; sq < 64 ; ++sq){
		for (int i = 0; i < 512; ++i) used[i] = 0ULL;
		bitboard piece = 1ULL << sq;
		bitboard mask = bishopMask(piece);
		int n = population(mask);
		int max = 1 << n;
		bitboard occ;
		for (int i = 0; i < max; ++i) {
			occ = indexToBitBoard(i, n, mask);
#ifndef fixedShift
			int ind = getIndex(occ, BishopBits[sq], BishopMagicCalc[sq]);
#else
			int ind = getIndex(occ, maxBitsBishop,  BishopMagicCalc[sq]);
#endif
			if (used[ind] == 0ULL) used[ind] = bishopAttacks(piece, ~occ);
		}
		fprintf(out, "{");
		for (int i = 0 ; i < 512 ; ++i){
			if (i%8==0) {
				fprintf(out, "\n\t");
			} else {
				fprintf(out, " ");
			}
			if (used[i]!=0){
				fprintf(out, format, used[i]);
			} else {
				fprintf(out, "_____________________");
			}
			if (i != 512) fprintf(out, ",");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "#undef _____________________\n};\n\n");
}
