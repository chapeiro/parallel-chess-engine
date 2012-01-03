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
#include "Board.h"
//#define bf

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

int getIndex(bitboard occ, int bits, bitboard magicl) {
	return (int) ((occ * magicl) >> (64 - bits));
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
		std::cout << "//" << count << std::endl;
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
	bitboard magicl;
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
		magicl = ~0ULL;
		for (int i = 0; i < maxLoopsForMagic; ++i) magicl &= getRandom(rdm);
#else
		next(magic, count);
#endif
		if (population((mask * magicl) & 0xFF00000000000000ULL) < 6) continue;
		for (int i = 0; i < 4096; ++i) used[i] = 0ULL;
		fail = false;
		for (int i = 0; i < max; ++i) {
			int ind = getIndex(occ[i], bits, magicl);
			if (used[ind] == 0ULL) {
				used[ind] = att[i];
			} else if (used[ind] != att[i]) {
				fail = true;
				break;
			}
		}
	} while (fail);
	return magicl;
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
		std::cout << ((bb & (1ULL << y)) >> y);
	}
	std::cout << std::endl;
}

bitboard RookMagicCalc[64];
bitboard BishopMagicCalc[64];
#ifdef _WIN32
#define format "%#018I64xULL"
#else
#define format "%#018llxULL"
#endif
void generate(const int bits[64], bool rook, int a, bitboard magicl[64], FILE* out){
	for (int square = 0; square < 64; ++square) {
		fprintf(out, "\t");
#ifndef fixedShift
		magicl[square] = generateMagics(square, min(bits[square], a), rook);
#else
		magicl[square] = generateMagics(square, a, rook);
#endif
		fprintf(out, format, magicl[square]);
		if (square != 63) fprintf(out, ",");
		fprintf(out, "\t//square : %02i\n", square);
	}
}

bitboard msb(bitboard i){
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
	i |= i >> 32;
	i ^= i >> 1;
	return i;
}

template <bool sb> bitboard getMagic(bitboard maskl, int shift){
	int pop = population(maskl);
	int max = 1 << pop;
	int count = 0;
	bool fail = true;
	bitboard used[64], occ[64];
	bitboard magicl;
	for (int i = 0 ; i < max ; ++i){
		occ[i] = indexToBitBoard(i, pop, maskl);
	}
#ifndef bf
	CRandomMersenne rdm(time(0));
#else
	magicl = 1ULL << 63;
#endif
	do {
		++count;
#ifndef bf
		magicl = ~0ULL;
		for (int i = 0; i < maxLoopsForMagic; ++i) magicl &= getRandom(rdm);
#else
		next(magicl, count);
#endif
		for (int i = 0; i < 64; ++i) used[i] = ~0ULL;
		fail = false;
		for (int i = 0; i < max; ++i) {
			int ind = (int) ((occ[i] * magicl) >> (64 - shift)); //(sb ? 6 : a)
			if (used[ind] == ~0ULL) {
				used[ind] = (sb ? (occ[i] & (-occ[i])) : msb(occ[i]));
			} else if (used[ind] != (sb ? (occ[i] & (-occ[i])) : msb(occ[i]))) {
				fail = true;
				break;
			}
		}
	} while (fail);
	return magicl;
}

void MagicGenerator(int maxBitsRook, int maxBitsBishop, int maxCheckAvoidanceShiftBits, bitboard raysl[64][64], FILE* out) {
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
	int king, piece, f1, f2, r1, r2, count2 (0), flag[64][64];
	bitboard magicCA[64][64], mask[64][64];
	for (king = 63 ; king >= 0 ; --king){
		flag[king][king] = 0;
		for (piece = king + 1 ; piece < 64 ; ++piece){
			f1 = file(king);
			f2 = file(piece);
			r1 = rank(king);
			r2 = rank(piece);
			if (f1 == f2){
				flag[king][piece] = ROOK;
				mask[king][piece] = raysl[piece][piece|56];
				if ((piece >> 3) != 7) mask[king][piece] |= 1ull << (piece|56);
			} else if (r1 == r2){
				flag[king][piece] = ROOK;
				mask[king][piece] = raysl[piece][(piece&56)| 7];
				if ((piece & 7) != 7) mask[king][piece] |= 1ull << ((piece&56)| 7);
			} else if (r1+f1 == r2+f2){
				flag[king][piece] = BISHOP;
				int mp;
				if ((r1+f1) <= 7){
					mp = ((r1+f1) << 3) | 7;
				} else {
					mp = (14-(r1+f1)) | 56;
				}
				mask[king][piece] = raysl[piece][mp];
				if (piece != mp) mask[king][piece] |= 1ull << mp;
			} else if (r1-f1 == r2-f2){
				flag[king][piece] = BISHOP;
				int mp;
				if ((r1-f1) <= 0){
					mp = (7 + (r1-f1))<<3;
				} else {
					mp = (r1-f1) | 56;
				}
				mask[king][piece] = raysl[piece][mp];
				if (piece != mp) mask[king][piece] |= 1ull << mp;
			} else {
				flag[king][piece] = WRONG_PIECE;
			}
			if (flag[king][piece] != WRONG_PIECE){
				++count2;
				magicCA[king][piece] = getMagic<true>(mask[king][piece], maxCheckAvoidanceShiftBits);
			}
		}
	}
	for (king = 1 ; king < 64 ; ++king){
		for (piece = king-1 ; piece >= 0 ; --piece){
			f1 = file(king);
			f2 = file(piece);
			r1 = rank(king);
			r2 = rank(piece);
			if (f1 == f2){
				flag[king][piece] = ROOK;
				mask[king][piece] = raysl[piece][piece&7];
				if ((piece >> 3) != 0) mask[king][piece] |= 1ull << (piece&7);
			} else if (r1 == r2){
				flag[king][piece] = ROOK;
				mask[king][piece] = raysl[piece][piece&56];
				if ((piece & 7) != 0) mask[king][piece] |= 1ull << (piece&56);
			} else if (r1+f1 == r2+f2){
				flag[king][piece] = BISHOP;
				int mp;
				if ((r1+f1) <= 7){
					mp = 7 - (r1+f1);
				} else {
					mp = ((r1+f1)-7)<<3;
				}
				mask[king][piece] = raysl[piece][mp];
				if (piece != mp) mask[king][piece] |= 1ull << mp;
			} else if (r1-f1 == r2-f2){
				flag[king][piece] = BISHOP;
				int mp;
				if ((r1-f1) <= 0){
					mp = 7 + (r1-f1);
				} else {
					mp = 7 | ((r1-f1) << 3);
				}
				mask[king][piece] = raysl[piece][mp];
				if (piece != mp) mask[king][piece] |= 1ull << mp;
			} else {
				flag[king][piece] = WRONG_PIECE;
			}
			if (flag[king][piece] != WRONG_PIECE){
				++count2;
				magicCA[king][piece] = getMagic<false>(mask[king][piece], maxCheckAvoidanceShiftBits);
			}
		}
	}
	fprintf(out, "const int direction[64][64] = {\n");
	for (int sq = 0 ; sq < 64 ; ++sq){
		fprintf(out, "{");
		for (int i = 0 ; i < 64 ; ++i){
			if (i%8==0) {
				fprintf(out, "\n\t");
			} else {
				fprintf(out, " ");
			}
			fprintf(out, "%d", flag[sq][i]);
			if (i != 63) fprintf(out, ",");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "};\n\n");
	fprintf(out, "const bitboard XRayOFCMask[64][64] = {\n#define _____________________ 0ULL\n");
	for (int sq = 0 ; sq < 64 ; ++sq){
		fprintf(out, "{");
		for (int i = 0 ; i < 64 ; ++i){
			if (i%8==0) {
				fprintf(out, "\n\t");
			} else {
				fprintf(out, " ");
			}
			if (mask[sq][i]!=0){
				fprintf(out, format, mask[sq][i]);
			} else {
				fprintf(out, "_____________________");
			}
			if (i != 63) fprintf(out, ",");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "#undef _____________________\n};\n\n");
	fprintf(out, "const bitboard XRayOFCMagic[64][64] = {\n#define _____________________ 0ULL\n");
	for (int sq = 0 ; sq < 64 ; ++sq){
		fprintf(out, "{");
		for (int i = 0 ; i < 64 ; ++i){
			if (i%8==0) {
				fprintf(out, "\n\t");
			} else {
				fprintf(out, " ");
			}
			if (magicCA[sq][i]!=0){
				fprintf(out, format, magicCA[sq][i]);
			} else {
				fprintf(out, "_____________________");
			}
			if (i != 63) fprintf(out, ",");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "#undef _____________________\n};\n\n");
	fprintf(out, "const bitboard XRayOFCChecker[64][64][64] = {\n#define _____________________ 0ULL\n");
	bitboard att[64];
	for (int sq = 0 ; sq < 64 ; ++sq){
		fprintf(out, "{");
		for (int piece = 0 ; piece < 64 ; ++piece){
			fprintf(out, "\t{");
			for (int i = 0 ; i < 64 ; ++i) att[i] = 0ull;
			int pop = population(mask[sq][piece]);
			int max = 1 << pop;
			bitboard occ, t;
			for (int i = 0 ; i < max ; ++i) {
				occ = indexToBitBoard(i, pop, mask[sq][piece]);
				t = (sq < piece ? ( occ & -occ ) : msb(occ) );
				att[(indexToBitBoard(i, pop, mask[sq][piece])*magicCA[sq][piece]) >> (64 - maxCheckAvoidanceShiftBits)] = t;
			}
			for (int i = 0 ; i < 64 ; ++i){
				if (i%8==0) {
					fprintf(out, "\n\t\t");
				} else {
					fprintf(out, " ");
				}
				if (att[i]!=0){
					fprintf(out, format, att[i]);
				} else {
					fprintf(out, "_____________________");
				}
				if (i != 63) fprintf(out, ",");
			}
			fprintf(out, "}");
			if (piece != 63) fprintf(out, ",");
			fprintf(out, "\n");
		}
		fprintf(out, "}");
		if (sq != 63) fprintf(out, ",");
		fprintf(out, "\n");
	}
	fprintf(out, "#undef _____________________\n};\n\n");
}
