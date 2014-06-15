/*
 * PrecomputedDataGenerator.cpp
 *
 *  Created on: 18 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#include "cchapeiro.hpp"
#include "SquareMapping.hpp"
#include "MagicsAndPrecomputedData.hpp"
#include "Utilities.cpp"
#include <stdio.h>
#include <ctime>
#include <iostream>

void precomputeData(){
	FILE * pd;
	pd = fopen("precompd_PrecomputedData.cpp", "w");
	char ct[20];
	time_t rawtime;
	time(&rawtime);
	strftime (ct, 20, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
	fprintf(pd, "/**   Auto-generated file.\n");
	fprintf(pd, " * Contains pre-computed data for CChapeiro.\n");
	fprintf(pd, " * Generated : %s\n", ct);
	fprintf(pd, " **/\n\n");
	fprintf(pd, "#include \"Board.h\"\n");
	fprintf(pd, "#include \"MagicsAndPrecomputedData.h\"\n\n");
	std::cout << ndbgline << "Generating De Bruijn's number and bitscan ..." << std::endl;
	CGenBitScan a;
	a.generateBitScan(pd);
	std::cout << ndbgline << "Generating squares, files and ranks ..." << std::endl;
	bitboard fnormal[64];
	bitboard frank[8], ffile[8];
	bitboard notFnormal[64];
	bitboard notFrank[8], notFfile[8];
	bitboard mDiag[15], aDiag[15];
	for (int y = 0 ; y < 8 ; ++y){
		for (int x = 0 ; x < 8 ; ++x){
			fnormal[index(x, y)] = bitboard(1) << index(x, y);
			notFnormal[index(x, y)] = ~(fnormal[index(x, y)]);
		}
	}
	frank[0] = bitboard(1);
	frank[0] |= frank[0] << 1;
	frank[0] |= frank[0] << 2;
	frank[0] |= frank[0] << 4;
	for (int y = 1 ; y < 8 ; ++y) frank[y] = frank[y-1] << 8;
	for (int y = 0 ; y < 8 ; ++y) notFrank[y] = ~(frank[y]);
	ffile[7] = bitboard(1);
	ffile[7] |= ffile[7] << 8;
	ffile[7] |= ffile[7] << 16;
	ffile[7] |= ffile[7] << 32;
	for (int y = 6 ; y >= 0 ; --y) ffile[y] = ffile[y+1] << 1;
	for (int y = 7 ; y >= 0 ; --y) notFfile[y] = ~(ffile[y]);
	mDiag[7] = bitboard(1);
	mDiag[7] |= mDiag[7] << 9;
	mDiag[7] |= mDiag[7] << 18;
	mDiag[7] |= mDiag[7] << 36;
	for (int y = 6 ; y >= 0 ; --y) mDiag[y] = (mDiag[y+1] << 1) & (notFfile[7]);
	for (int y = 8 ; y < 15 ; ++y) mDiag[y] = (mDiag[y-1] >> 1) & (notFfile[0]);
	aDiag[7] = bitboard(1) << 7;
	aDiag[7] |= aDiag[7] << 7;
	aDiag[7] |= aDiag[7] << 14;
	aDiag[7] |= aDiag[7] << 28;
	for (int y = 6 ; y >= 0 ; --y) aDiag[y] = (aDiag[y+1] >> 1) & (notFfile[0]);
	for (int y = 8 ; y < 15 ; ++y) aDiag[y] = (aDiag[y-1] << 1) & (notFfile[7]);
	fprintf(pd, "\nconst bitboard filled::normal[64] = {\n\t");
	for (int i = 0 ; i < 64 ; ++i){
		fprintf(pd, formatBitboard, fnormal[i]);
		if (i != 63) {
			fprintf(pd, ", ");
			if ((i&7)==7) fprintf(pd, "\n\t");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "const bitboard notFilled::normal[64] = {\n\t");
	for (int i = 0 ; i < 64 ; ++i){
		fprintf(pd, formatBitboard, notFnormal[i]);
		if (i != 63) {
			fprintf(pd, ", ");
			if ((i&7)==7) fprintf(pd, "\n\t");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "\nconst bitboard filled::rank[8] = {\n\t");
	for (int i = 0 ; i < 8 ; ++i){
		fprintf(pd, formatBitboard, frank[i]);
		if (i != 7) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "const bitboard notFilled::rank[8] = {\n\t");
	for (int i = 0 ; i < 8 ; ++i){
		fprintf(pd, formatBitboard, notFrank[i]);
		if (i != 7) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "\nconst bitboard filled::file[8] = {\n\t");
	for (int i = 0 ; i < 8 ; ++i){
		fprintf(pd, formatBitboard, ffile[i]);
		if (i != 7) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "const bitboard notFilled::file[8] = {\n\t");
	for (int i = 0 ; i < 8 ; ++i){
		fprintf(pd, formatBitboard, notFfile[i]);
		if (i != 7) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "\nconst bitboard filled::mainDiag[15] = {\n\t");
	for (int i = 0 ; i < 15 ; ++i){
		fprintf(pd, formatBitboard, mDiag[i]);
		if (i != 14) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	fprintf(pd, "\nconst bitboard filled::antiDiag[15] = {\n\t");
	for (int i = 0 ; i < 15 ; ++i){
		fprintf(pd, formatBitboard, aDiag[i]);
		if (i != 14) {
			fprintf(pd, ", ");
		} else {
			fprintf(pd, "\n");
		}
	}
	fprintf(pd, "\n};\n\n");
	bitboard KnightMovesl[64], KingMovesl[64], raysl[64][64];
	int rayTp[64][64];
	for (int i = 0 ; i < 64 ; ++i){
		bitboard b = bitboard(1) << i;
		KnightMovesl[i] = (
			( ((b >> 15) | (b << 17)) & notFfile[7] ) |
			( ((b << 15) | (b >> 17)) & notFfile[0] ) |
			( ((b >> 6)  | (b << 10)) & notFfile[7] & notFfile[6]) |
			( ((b >> 10) | (b << 6))  & notFfile[0] & notFfile[1])
					);
		KingMovesl[i] = (
			( ( ( b << 1) | ( b << 9 ) | ( b >> 7 ) ) & notFfile[7]) |
			( ( ( b >> 1) | ( b >> 9 ) | ( b << 7 ) ) & notFfile[0]) |
			(   ( b << 8) | ( b >> 8 ) )
					);
	}
	std::cout << ndbgline << "Generating Knight's Moves..." << std::endl;
	fprintf(pd, "const bitboard KnightMoves[64] = {\n\t");
	for (int i = 0 ; i < 64 ; ++i){
		fprintf(pd, formatBitboard, KnightMovesl[i]);
		if (i != 63) {
			fprintf(pd, ", ");
			if ((i&7)==7) fprintf(pd, "\n\t");
		}
	}
	fprintf(pd, "\n};\n\n");
	std::cout << ndbgline << "Generating King's Moves..." << std::endl;
	fprintf(pd, "const bitboard KingMoves[64] = {\n\t");
	for (int i = 0 ; i < 64 ; ++i){
		fprintf(pd, formatBitboard, KingMovesl[i]);
		if (i != 63) {
			fprintf(pd, ", ");
			if ((i&7)==7) fprintf(pd, "\n\t");
		}
	}
	fprintf(pd, "\n};\n");
	std::cout << ndbgline << "Generating Rays..." << std::endl;
	bitboard b, a1;
#define BISHOP 4
#define ROOK 6
	for (int i = 0 ; i < 64 ; ++i){
		raysl[i][i] = 0;
		for (int j = i + 1 ; j < 64 ; ++j){
			if (rank(i) == rank(j)){
				rayTp[i][j] = rayTp[j][i] = ROOK;
				raysl[i][j] = raysl[j][i] = (1ull << j) - (1ull << (i + 1));
			} else if (file(i) == file(j)){
				rayTp[i][j] = rayTp[j][i] = ROOK;
				raysl[i][j] = raysl[j][i] = ffile[file(i)] & ((1ull << j) - (1ull << (i + 1)));
			} else if (rank(i) + file(i) == rank(j) + file(j)){
				rayTp[i][j] = rayTp[j][i] = BISHOP;
				a1 = 1ull << (i + 9);
				b = 1ull << j;
				raysl[j][i] = 0;
				while (a1 < b){
					raysl[j][i] |= a1;
					a1 <<= 9;
				}
				raysl[i][j] = raysl[j][i];
			} else if (rank(i) - file(i) == rank(j) - file(j)){
				rayTp[i][j] = rayTp[j][i] = BISHOP;
				a1 = 1ull << (i + 7);
				b = 1ull << j;
				raysl[j][i] = 0;
				while (a1 < b){
					raysl[j][i] |= a1;
					a1 <<= 7;
				}
				raysl[i][j] = raysl[j][i];
			} else {
				rayTp[i][j] = rayTp[j][i] = 0;
				raysl[j][i] = raysl[i][j] = 0;
			}
		}
	}
	fprintf(pd, "const bitboard rays[64][64] = {\n#define _____________________ 0ULL\n\t");
	for (int i = 0 ; i < 64 ; ++i){
		fprintf(pd, "{\n\t\t");
		for (int j = 0 ; j < 64 ; ++j){
			if (raysl[i][j] != 0){
				fprintf(pd, formatBitboard, raysl[i][j]);
			} else {
				fprintf(pd, "_____________________");
			}
			if (j != 63) {
				fprintf(pd, ", ");
				if ((j&7)==7) fprintf(pd, "\n\t\t");
			}
		}
		if (i != 63) fprintf(pd, "\n\t},\n\t");
	}
	fprintf(pd, "\n\t}\n};");
	fclose(pd);
	std::cout << ndbgline << "Generating Magics For Rooks and Bishops..." << std::endl;
	FILE * out;
	out = fopen ("precompd_magics.cpp", "w");
	MagicGenerator(maxRookBits, maxBishopBits, maxCheckAvoidanceShiftBits, raysl, out);
	fclose(out);
}
