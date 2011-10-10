/*
 * Board.cpp
 *	BitBoards implemented using magic bitboards.
 *  Created on: 29 ��� 2011
 *      Author: Chrysogelos Periklis
 */
#include "Board.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#define NDEBUG
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;

const string PiecesName[] = {"Pawns", "Knights", "Bishops",
							 "Rooks", "Queens", "Kings"};
const char PiecesNameSort[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

inline int Board::getPieceIndex(char p){
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A') | black;
	return getWhitePieceIndex(p);
}

inline bool Board::isPieceBlack(int pieceIndex){
	if ((pieceIndex&1)==white) return false;
	return true;
}

int Board::getWhitePieceIndex(char p){
	for (int i = 0 ; i < (PIECESMAX>>1) ; ++i){
		if (p == PiecesNameSort[i]) return i<<1;
	}
	return WRONG_PIECE;
}

Board::Board(char fenBoard[], char fenPlaying, char fenCastling[], int fenEnPX, int fenEnPY, int fenHC, int fenFM){
	//General initialize
	pre = "";
	dividedepth = -1;
	for (int i = 0 ; i < PIECESMAX ; ++i) Pieces[i] = bitboard(0);
	White_Pieces = bitboard(0);
	Black_Pieces = bitboard(0);
	castling = 0;
	zobr = 0;
	lastHistoryEntry = 0;
	int wk (0), bk(0);
	//FEN processing
	//	according to :
	//	http://en.wikipedia.org/w/index.php?title=Forsyth%E2%80%93Edwards_Notation&oldid=396200404
	// 1. Piece Placement
	int index = 0;
	int x, ind, sq;
	char inp;
	for (int y = 7 ; y >= 0 ; --y){
		x = 0;
		do {
			inp = fenBoard[index++];
			if (inp < 'A'){
				x += inp-'0';
			} else {
				ind = getPieceIndex(inp);
				if (ind == (KING | white)){
					++wk;
				} else if (ind == (KING | black)){
					++bk;
				}
				sq = index(x, y);
				updatePieces(sq, ind);
				++x;
			}
		} while (x <= 7);
		++index;
	}
	if (wk!=1) throw 1;
	if (bk!=1) throw 2;
	// 2. Active color
	if (fenPlaying=='b'){
		playing = black;
		zobr ^= zobrist::blackKey;
	} else {
		playing = white;
	}
	// 3. Castling availability
	ind = 0;
	while (fenCastling[ind] != '\0'){
		switch (fenCastling[ind++]){
		case 'K' :
			castling |= WhiteKingSideC;
			zobr ^= zobrist::White_King_Castling;
			break;
		case 'Q' :
			castling |= WhiteQueenSideC;
			zobr ^= zobrist::White_Queen_Castling;
			break;
		case 'k' :
			castling |= BlackKingSideC;
			zobr ^= zobrist::Black_King_Castling;
			break;
		case 'q' :
			castling |= BlackQueenSideC;
			zobr ^= zobrist::Black_Queen_Castling;
			break;
		}
	}
	// 4. En passant target square in algebraic notation
	if (fenEnPX == -1){
		enPassant = bitboard(0);
	} else {
		assert(fenEnPX>=0);
		assert(fenEnPX<=7);
		assert(fenEnPY>=0);
		assert(fenEnPY<=7);
		enPassant = filled::normal[index(fenEnPX, fenEnPY)];
		zobr ^= zobrist::enPassant[fenEnPX];
	}
	// 5. Halfmove clock
	halfmoves = fenHC;
	// 6. Fullmove number
	fullmoves = fenFM;
	//FEN processing ended
	addToHistory(zobr);
}

inline void Board::updatePieces(int sq, int ind){
	if ((sq < 64 && sq >= 0)==false) cout << sq << endl;
	assert(sq < 64 && sq >= 0);
	if ((ind >= 0 && ind < 12)==false) cout << ind << endl;
	assert(ind >= 0 && ind < 12);
	Pieces[ind] ^= filled::normal[sq];
	zobr ^= zobrist::keys[sq][ind];
	if ((ind&1)==black){
		Black_Pieces ^= filled::normal[sq];
	} else {
		White_Pieces ^= filled::normal[sq];
	}
}

move Board::getNullMove(){
	move n;
	n.fromX = -1;
	return n;
}

bool Board::moveIsNull(move m){
	if (m.fromX==-1) return true;
	return false;
}

short Board::convertPromotion(char a){
	return getWhitePieceIndex(a-'a'+'A');
}

void Board::printbb(bitboard bb){
	int i;
	for (int y = 7 ; y >= 0 ; --y){
		dbgstream << ndbgline;
		for (int x = 0 ; x < 8 ; ++x){
			i = index(x, y);
			dbgstream << ((bb & filled::normal[i]) >> i);
		}
		dbgstream << '\n';
	}
}

void Board::print(){
	if (debugcc){
		dbgstream << ndbgline << "--------------------------------\n";
		dbgstream << ndbgline << getFEN() << '\n';
		for (int i = white ; i < PIECESMAX ; i+=2){
			dbgstream << ndbgline << "White " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		for (int i = black ; i < PIECESMAX ; i+=2){
			dbgstream << ndbgline << "Black " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		dbgstream << ndbgline << "Zobrist Key: " << zobr << '\n';
		dbgstream << ndbgline << "--------------------------------" << endl;
	}
}

void Board::deactivateCastlingRights(){
	zobr ^= zobrist::castling[(castling*castlingsmagic)>>59];
	castling &= deactcastlingrights[playing];
	zobr ^= zobrist::castling[(castling*castlingsmagic)>>59];
}

void Board::capture(int to){
	for (int i = playing^1 ; i < PIECESMAX ; i+=2){
		if ((Pieces[i] & filled::normal[to])!=0){
			updatePieces(to, i);
			return;
		}
	}
}
/**
 * ATTENTION!: move m has to be LEGAL (or null move)
 */
void Board::make(move m){
	assert(!moveIsNull(m));
	int from = index(m.fromX, m.fromY);
	int to = index(m.toX, m.toY);
	if ((Pieces[PAWN+playing] & filled::normal[from])!=0){
		//is Pawn
		bitboard enemy;
		int lastRank;
		if (playing==white){
			enemy = Black_Pieces;
			lastRank = lastRank_w;
		} else {
			enemy = White_Pieces;
			lastRank = lastRank_b;
		}
		if ((enemy & filled::normal[to])!=0){
			//capture
			updatePieces(from, playing+PAWN);
			if (rank(to)==lastRank){
				updatePieces(to, m.promoteTo+playing);
			} else {
				updatePieces(to, playing+PAWN);
			}
			capture(to);
			enPassant = bitboard(0);
		} else {
			if (from-to==index(0, 0)-index(0, 1) || to-from==index(0, 0)-index(0, 1)){
				//normal
				updatePieces(from, playing+PAWN);
				if (rank(to)==lastRank){
					updatePieces(to, m.promoteTo+playing);
				} else {
					updatePieces(to, playing+PAWN);
				}
				enPassant = bitboard(0);
			} else if (from-to==index(0, 0)-index(0, 2) || to-from==index(0, 0)-index(0, 2)){
				//double
				updatePieces(from, playing+PAWN);
				updatePieces(to, playing+PAWN);
				enPassant = filled::normal[(from+to)/2];
				zobr ^= zobrist::enPassant[7&square( enPassant )];
			} else {
				//en passant
				updatePieces(from, playing+PAWN);
				updatePieces(to, playing+PAWN);
				updatePieces(index(file(to), rank(from)), (playing^1)+PAWN);
				enPassant = bitboard(0);
			}
		}
		halfmoves = 0;
		if (playing==black) ++fullmoves;
		togglePlaying();
	} else if ((Pieces[KING+playing] & filled::normal[from])!=0){
		//is King
		bool castl = false;
		if (playing == white){
			if (from == index('e'-'a', '1'-'1') && to == index('g'-'a', '1'-'1')){
				castl = true;
			} else if (from == index('e'-'a', '1'-'1') && to == index('c'-'a', '1'-'1')){
				castl = true;
			}
		} else {
			if (from == index('e'-'a', '8'-'1') && to == index('g'-'a', '8'-'1')){
				castl = true;
			} else if (from == index('e'-'a', '8'-'1') && to == index('c'-'a', '8'-'1')){
				castl = true;
			}
		}
		if (castl) {
			updatePieces(from, ROOK+playing);
			updatePieces((from+to)/2, ROOK+playing);
		}
		updatePieces(from, KING+playing);
		updatePieces(to, KING+playing);
		capture(to);
		if (playing==black) ++fullmoves;
		deactivateCastlingRights();
		togglePlaying();
		enPassant = bitboard(0);
	} else {
		for (int i = playing ; i < PIECESMAX ; i+=2){
			if ((Pieces[i] & filled::normal[from])!=0){
				updatePieces(from, i);
				updatePieces(to, i);
				if (i-playing==ROOK && (filled::normal[from] & allcastlingrights) != 0){
						zobr ^= zobrist::castling[(castling*castlingsmagic)>>59];
						castling ^= filled::normal[from];
						zobr ^= zobrist::castling[(castling*castlingsmagic)>>59];
				}
				break;
			}
		}
		capture(to);
		if (playing==black) ++fullmoves;
		togglePlaying();
		enPassant = bitboard(0);
	}
	addToHistory(zobr);
}

/**Use with caution, slow method
 *	Only for use in debugging.
 **/
string Board::getFEN(){
	//A FEN record contains six fields.
	//The separator between fields is a space.
	//	The fields are:
	string fen = "";
	//	1. Piece placement (from white's perspective).
	//		Each rank is described, starting with rank 8 and
	//		ending with rank 1; within each rank, the contents
	//		of each square are described from file a through
	//		file h. Following the Standard Algebraic Notation (SAN),
	//		each piece is identified by a single letter taken
	//		from the standard English names (pawn = "P",
	//		knight = "N", bishop = "B", rook = "R", queen = "Q"
	//		and king = "K").[1] White pieces are designated
	//		using upper-case letters ("PNBRQK") while Black
	//		take lowercase ("pnbrqk"). Blank squares are noted
	//		using digits 1 through 8 (the number of blank
	//		squares), and "/" separate ranks.
	for (int y = 7 ; y >= 0 ; --y){
		int count = 0;
		for (int x = 0 ; x < 8 ; ++x){
			bitboard a = filled::normal[index(x, y)];
			if ((White_Pieces & a) !=0){
				if (count != 0){
					fen += count+'0';
					count = 0;
				}
				for (int i = white ; i < PIECESMAX ; i+=2){
					if ((Pieces[i] & a) != 0){
						fen += PiecesNameSort[i>>1];
					}
				}
			} else if ((Black_Pieces & a) !=0){
				if (count != 0){
						fen += count+'0';
						count = 0;
				}
				for (int i = black ; i < PIECESMAX ; i+=2){
					if ((Pieces[i] & a) != 0){
						fen += PiecesNameSort[i>>1]-'A'+'a';
					}
				}
			} else {
				++count;
			}
		}
		if (count != 0) fen += count+'0';
		if (y != 0) fen += '/';
	}
	fen += ' ';
	//	2. Active color. "w" means white moves next, "b" means black.
	fen += ( playing==white ? 'w' : 'b' );
	fen += ' ';
	//	3. Castling availability. If neither side can castle,
	//		this is "�". Otherwise, this has one or more letters:
	//		"K" (White can castle kingside),
	//		"Q" (White can castle queenside),
	//		"k" (Black can castle kingside) and/or
	//		"q" (Black can castle queenside).
	bool ncr = true;
	if ((castling & WhiteKingSideC) != 0) {
		fen += 'K';
		ncr = false;
	}
	if ((castling & WhiteQueenSideC) != 0){
		fen += 'Q';
		ncr = false;
	}
	if ((castling & BlackKingSideC) != 0) {
		fen += 'k';
		ncr = false;
	}
	if ((castling & BlackQueenSideC) != 0){
		fen += 'q';
		ncr = false;
	}
	if (ncr) fen += '-';
	fen += ' ';
	//	4. En passant target square in algebraic notation.
	if (enPassant==0ull){
		//	If there's no en passant target square, this is "�".
		fen += '-';
	} else {
		//	If a pawn has just made a 2-square move, this is the
		//	position "behind" the pawn. This is recorded
		//	regardless of whether there is a pawn in position
		//	to make an en passant capture.
		for (int i = 0 ; i < 8 ; ++i){
			if (enPassant == filled::normal[index(i, 5)]){
				fen += i+'a';
				fen += '6';
				break;
			}
			if (enPassant == filled::normal[index(i, 2)]){
				fen += i+'a';
				fen += '3';
				break;
			}
		}
	}
	fen += ' ';
	// 5. Halfmove clock: This is the number of halfmoves since
	//		the last pawn advance or capture. This is used to
	//		determine if a draw can be claimed under the fifty-move rule.
	char num[8];
	sprintf(num, "%d", halfmoves);
	fen += num;
	fen += ' ';
	// 6. Fullmove number: The number of the full move.
	//		It starts at 1, and is incremented after Black's move.
	sprintf(num, "%d", fullmoves);
	fen += num;
	return fen;
}

void Board::newGame(){}

bitboard Board::bishopAttacks(bitboard occ, const int &sq){
	occ &= BishopMask[sq];
	occ *= BishopMagic[sq];
#ifndef fixedShift
	occ >>= BishopShift[sq];
#else
	occ >>= 64-maxBishopBits;
#endif
	return BishopAttacks[sq][occ];
}

bitboard Board::bishopMovesTo(bitboard occ, const int &sq, bitboard &notFriendly){
	return bishopAttacks(occ, sq) & notFriendly;
}

bitboard Board::rookAttacks(bitboard occ, const int &sq){
	occ &= RookMask[sq];
	occ *= RookMagic[sq];
#ifndef fixedShift
	occ >>= RookShift[sq];
#else
	occ >>= 64-maxRookBits;
#endif
	return RookAttacks[sq][occ];
}

bitboard Board::rookMovesTo(bitboard occ, const int &sq, bitboard &notFriendly){
	return rookAttacks(occ, sq) & notFriendly;
}

bitboard Board::queenAttacks(bitboard occ, const int &sq){
	return rookAttacks(occ, sq) | bishopAttacks(occ, sq);
}

bitboard Board::queenMovesTo(bitboard occ, const int &sq, bitboard &notFriendly){
	return queenAttacks(occ, sq) & notFriendly;
}

inline void Board::continueCapturesPerft(const bitboard &cpt, const int &captured, const int &i,
		const int* p, const bitboard* from, const bitboard* att, const int* fromSq,
		bitboard* &ally, bitboard* &enemy, const int &depth, U64 &moves){
	bitboard temp = att[i] & cpt;
	bitboard to, ft;
	key toggle;
	int tosq;
	while (temp != 0){
		to = temp & -temp;
		tosq = square(to);
		ft = to | from[i];
		Pieces[captured] ^= to;
		Pieces[p[i]] ^= ft;
		*enemy ^= to;
		*ally ^= ft;
		toggle = zobrist::keys[tosq][captured] ^
				 zobrist::keys[tosq][p[i]] ^
				 zobrist::keys[fromSq[i]][p[i]];
		zobr ^= toggle;
		addToHistory(zobr);
		if (validPosition()) moves += perft(depth-1);
		Pieces[captured] ^= to;
		Pieces[p[i]] ^= ft;
		*enemy ^= to;
		*ally ^= ft;
		zobr ^= toggle;
		removeLastHistoryEntry();
		temp &= temp-1;
	}
}

inline void Board::continueNormalMPerft(const bitboard &empty, const int &i,
		const int* p, const bitboard* from, const bitboard* att, const int* fromSq,
		bitboard* &ally, const int &depth, U64 &moves){
	bitboard temp = att[i] & empty;
	bitboard to, ft;
	key toggle;
	int tosq;
	while (temp != 0){
		to = temp & -temp;
		tosq = square(to);
		ft = to | from[i];
		Pieces[p[i]] ^= ft;
		*ally ^= ft;
		toggle = zobrist::keys[tosq][p[i]] ^ zobrist::keys[fromSq[i]][p[i]];
		zobr ^= toggle;
		addToHistory(zobr);
		if (validPosition()) moves += perft(depth-1);
		Pieces[p[i]] ^= ft;
		*ally ^= ft;
		zobr ^= toggle;
		removeLastHistoryEntry();
		temp &= temp-1;
	}
}

U64 Board::perft(int depth){
	if (depth==0) {
		if (dividedepth == 0) cout << pre << getFEN() << endl;
		return 1;
	}
	int oldhm (halfmoves);
	halfmoves = 0;
	if (playing==black) ++fullmoves;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
	bitboard All_Pieces = White_Pieces | Black_Pieces;
	bitboard notAll_Pieces = ~All_Pieces;
	togglePlaying();
	bitboard tmpEnPassant (enPassant);
	enPassant = 0ull;
	U64 moves = movePawnsByAttOrProm(depth, notAll_Pieces, tmpEnPassant);
	togglePlaying();
	bitboard* enemy;
	bitboard* ally;
	if (playing==white){
		enemy = &Black_Pieces;
		ally  = &White_Pieces;
	} else {
		enemy = &White_Pieces;
		ally  = &Black_Pieces;
	}
	bitboard att[64], from[64];
	int p[64], fromSq[64], n(0), firstRook(0), firstQueen(0);
	bitboard knight = Pieces[KNIGHT+playing];
	while (knight!=0){
		from[n] = knight & -knight;
		fromSq[n] = square(from[n]);
		att[n] = KnightMoves[fromSq[n]];
		p[n] = KNIGHT+playing;
		++n;
		knight &= knight-1;
	}
	bitboard bishop = Pieces[BISHOP+playing];
	while (bishop!=0){
		from[n] = bishop & -bishop;
		fromSq[n] = square(from[n]);
		att[n] = bishopAttacks(All_Pieces, fromSq[n]);
		p[n] = BISHOP+playing;
		++n;
		bishop &= bishop-1;
	}
	bitboard rook = Pieces[ROOK+playing];
	firstRook = n;
	while (rook!=0){
		from[n] = rook & -rook;
		fromSq[n] = square(from[n]);
		att[n] = rookAttacks(All_Pieces, fromSq[n]);
		p[n] = ROOK+playing;
		++n;
		rook &= rook-1;
	}
	bitboard queen = Pieces[QUEEN+playing];
	firstQueen = n;
	while (queen!=0){
		from[n] = queen & -queen;
		fromSq[n] = square(from[n]);
		att[n] = queenAttacks(All_Pieces, fromSq[n]);
		p[n] = QUEEN+playing;
		++n;
		queen &= queen-1;
	}
	p[n] = KING+playing;
	from[n] = Pieces[p[n]];
	fromSq[n] = square(from[n]);
	att[n] = KingMoves[fromSq[n]];
	//n : position of last bitboard generated

	//captures
	bitboard cpt, oldcastling;
	Zobrist ct, ct2;
	int i;
	if ((castling & castlingrights[playing])==0){
		togglePlaying();
		for (int captured = QUEEN | playing ; captured >= 0 ; captured-=2){ //playing == playing^1
			cpt = Pieces[captured];
			for (i = 0 ; i <= n ; ++i) continueCapturesPerft(cpt, captured, i, p, from, att, fromSq, ally, enemy, depth, moves);
		}
		halfmoves = oldhm+1;
	} else {
		togglePlaying();
		oldcastling = castling;
		ct = zobrist::castling[(castling*castlingsmagic)>>59];
		for (int captured = QUEEN | playing ; captured >= 0 ; captured-=2){ //playing == playing^1
			cpt = Pieces[captured];
			for (i = 0 ; i < firstRook ; ++i){
				continueCapturesPerft(cpt, captured, i, p, from, att, fromSq, ally, enemy, depth, moves);
			}
			zobr ^= ct;
			for (; i < firstQueen ; ++i){
				castling &= ~from[i];
				ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
				zobr ^= ct2;
				continueCapturesPerft(cpt, captured, i, p, from, att, fromSq, ally, enemy, depth, moves);
				zobr ^= ct2;
				castling = oldcastling;
			}
			zobr ^= ct;
			for (; i < n ; ++i){
				continueCapturesPerft(cpt, captured, i, p, from, att, fromSq, ally, enemy, depth, moves);
			}
			castling &= deactcastlingrights[playing^1];
			ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
			zobr ^= ct ^ ct2;
			continueCapturesPerft(cpt, captured, n, p, from, att, fromSq, ally, enemy, depth, moves);
			castling = oldcastling;
			zobr ^= ct ^ ct2;
		}
		halfmoves = oldhm+1;
	//WARNING! : playing has the value of the player to play after this move right now.
		if (playing != white){
			if ((castling&WhiteKingSideC&Pieces[ROOK|white])!=0 && (WhiteKingSideCSpace&All_Pieces)==0 && notAttacked(WKSCPassing) && validPosition()){
				castling &= deactcastlingrights[playing^1];
				key toggle = zobrist::keys[0][ROOK | white] ^ zobrist::keys[2][ROOK | white];
				toggle ^= ct ^ zobrist::castling[(castling*castlingsmagic)>>59];
				toggle ^= zobrist::keys[3][KING | white] ^ zobrist::keys[1][KING | white];
				Pieces[ROOK | white] ^= WKSCRT;
				Pieces[KING | white] ^= WKSCKT;
				White_Pieces ^= WKSCFT;
				zobr ^= toggle;
				if (validPosition()) moves += perft(depth-1);
				Pieces[ROOK | white] ^= WKSCRT;
				Pieces[KING | white] ^= WKSCKT;
				White_Pieces ^= WKSCFT;
				zobr ^= toggle;
				castling = oldcastling;
			}
			if ((castling&WhiteQueenSideC&Pieces[ROOK|white])!=0 && (WhiteQueenSideCSpace&All_Pieces)==0 && notAttacked(WQSCPassing) && validPosition()){
				castling &= deactcastlingrights[playing^1];
				key toggle = zobrist::keys[7][ROOK | white] ^ zobrist::keys[4][ROOK | white];
				toggle ^= ct ^ zobrist::castling[(castling*castlingsmagic)>>59];
				toggle ^= zobrist::keys[3][KING | white] ^ zobrist::keys[5][KING | white];
				Pieces[ROOK | white] ^= WQSCRT;
				Pieces[KING | white] ^= WQSCKT;
				White_Pieces ^= WQSCFT;
				zobr ^= toggle;
				if (validPosition()) moves += perft(depth-1);
				Pieces[ROOK | white] ^= WQSCRT;
				Pieces[KING | white] ^= WQSCKT;
				White_Pieces ^= WQSCFT;
				zobr ^= toggle;
				castling = oldcastling;
			}
		} else {
			if ((castling&BlackKingSideC&Pieces[ROOK|black])!=0 && (BlackKingSideCSpace&All_Pieces)==0 && notAttacked(BKSCPassing) && validPosition()){
				castling &= deactcastlingrights[playing^1];
				key toggle = zobrist::keys[56][ROOK | black] ^ zobrist::keys[58][ROOK | black];
				toggle ^= ct ^ zobrist::castling[(castling*castlingsmagic)>>59];
				toggle ^= zobrist::keys[59][KING | black] ^ zobrist::keys[57][KING | black];
				Pieces[ROOK | black] ^= BKSCRT;
				Pieces[KING | black] ^= BKSCKT;
				Black_Pieces ^= BKSCFT;
				zobr ^= toggle;
				if (validPosition()) moves += perft(depth-1);
				Pieces[ROOK | black] ^= BKSCRT;
				Pieces[KING | black] ^= BKSCKT;
				Black_Pieces ^= BKSCFT;
				zobr ^= toggle;
				castling = oldcastling;
			}
			if ((castling&BlackQueenSideC&Pieces[ROOK|black])!=0 && (BlackQueenSideCSpace&All_Pieces)==0 && notAttacked(BQSCPassing) && validPosition()){
				castling &= deactcastlingrights[playing^1];
				key toggle = zobrist::keys[63][ROOK | black] ^ zobrist::keys[60][ROOK | black];
				toggle ^= ct ^ zobrist::castling[(castling*castlingsmagic)>>59];
				toggle ^= zobrist::keys[59][KING | black] ^ zobrist::keys[61][KING | black];
				Pieces[ROOK | black] ^= BQSCRT;
				Pieces[KING | black] ^= BQSCKT;
				Black_Pieces ^= BQSCFT;
				zobr ^= toggle;
				if (validPosition()) moves += perft(depth-1);
				Pieces[ROOK | black] ^= BQSCRT;
				Pieces[KING | black] ^= BQSCKT;
				Black_Pieces ^= BQSCFT;
				zobr ^= toggle;
				castling = oldcastling;
			}
		}
	}
	//normal moves of non-Pawns
	bitboard empty = ~All_Pieces;
	if ((castling & castlingrights[playing^1])==0){
		for (i = 0 ; i <= n ; ++i) {
			continueNormalMPerft(empty, i, p, from, att, fromSq, ally, depth, moves);
		}
	} else {
		oldcastling = castling;
		ct = zobrist::castling[(castling*castlingsmagic)>>59];
		for (i = 0 ; i < firstRook ; ++i)continueNormalMPerft(empty, i, p, from, att, fromSq, ally, depth, moves);
		zobr ^= ct;
		for (; i < firstQueen ; ++i){
			castling &= ~from[i];
			ct2 = zobrist::castling[(castling*castlingsmagic)>>59];
			zobr ^= ct2;
			continueNormalMPerft(empty, i, p, from, att, fromSq, ally, depth, moves);
			zobr ^= ct2;
			castling = oldcastling;
		}
		zobr ^= ct;
		for (; i < n ; ++i) continueNormalMPerft(empty, i, p, from, att, fromSq, ally, depth, moves);
		castling &= deactcastlingrights[playing^1];
		ct ^= zobrist::castling[(castling*castlingsmagic)>>59];
		zobr ^= ct;
		continueNormalMPerft(empty, i, p, from, att, fromSq, ally, depth, moves);//(i==n)
		castling = oldcastling;
		zobr ^= ct;
	}
	//normal move pawns
	halfmoves = 0;
	moves += movePawnsForward(depth, notAll_Pieces);
	halfmoves = oldhm;
	togglePlaying();
	enPassant = tmpEnPassant;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
	if (playing==black) --fullmoves;
	if (depth == dividedepth) {
		cout << pre << getFEN() << '\t' << moves;
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
		int a = 0;
		sscanf(buffer, "Nodes: %d,", &a);
		if (a == moves) {
			cout << "\tOK" << endl;
		} else {
			cout << "\tFailed!\t" << a << endl;
			if (depth )
			cout << "-----------------------------------------" << endl;
			string oldpre = pre;
			pre += "\t";
			dividedepth = depth-1;
			perft(depth);
			cout << "-----------------------------------------" << endl;
			pre = oldpre;
			dividedepth = depth;
		}
#else
		cout << endl;
#endif
	}
	return moves;
}

void Board::addToHistory(Zobrist position){
	history[++lastHistoryEntry] = position;
}

void Board::removeLastHistoryEntry(){
	--lastHistoryEntry;
}

bool Board::notAttacked(const bitboard &target){
	assert((target & (target-1))==0);
	bitboard enemyP;
	if (playing == black){
		if ( ( (Pieces[PAWN | black] >> 7) & target & notfile7) != 0) return false;
		if ( ( (Pieces[PAWN | black] >> 9) & target & notfile0) != 0) return false;
		enemyP = Black_Pieces;
	} else {
		if ( ( (Pieces[PAWN | white] << 7) & target & notfile0) != 0) return false;
		if ( ( (Pieces[PAWN | white] << 9) & target & notfile7) != 0) return false;
		enemyP = White_Pieces;
	}
	int sq = square(target);
	if ((Pieces[KNIGHT | playing] & KnightMoves[sq])!=0) return false;
	if ((Pieces[KING | playing] & KingMoves[sq])!=0) return false;
	bitboard occ = White_Pieces | Black_Pieces;
	bitboard att = rookAttacks(occ, sq);
	if ((att & enemyP) != 0 && ((att & Pieces[ROOK | playing]) != 0 || (att & Pieces[QUEEN | playing]) != 0)) return false;
	att = bishopAttacks(occ, sq);
	if ((att & enemyP) != 0 && ((att & Pieces[BISHOP | playing]) != 0 || (att & Pieces[QUEEN | playing]) != 0)) return false;
	return true;
}

bool Board::validPosition() {
	return notAttacked(Pieces[KING + (playing^1)]);
}
