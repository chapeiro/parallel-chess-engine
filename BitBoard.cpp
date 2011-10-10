/*
 * Board.cpp
 *	BitBoards implemented using magic bitboards.
 *  Created on: 29 ��� 2011
 *      Author: Chrysogelos Periklis
 */
#include "Board.h"
#include <string>
#include <stdio.h>
#include <iostream>
//#define NDEBUG
#include <assert.h>
using namespace std;

const string PiecesName[] = {"Pawns", "Knights", "Bishops",
							 "Rooks", "Queens", "Kings"};
const char PiecesNameSort[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

inline int Board::getPieceIndex(char p){
	if (p > 'a') return getWhitePieceIndex(p-'a'+'A')+BlackPiecesOffset;
	return getWhitePieceIndex(p);
}

inline bool Board::isPieceBlack(int pieceIndex){
	if (pieceIndex >= BlackPiecesOffset) return true;
	return false;
}

int Board::getWhitePieceIndex(char p){
	for (int i = 0 ; i < BlackPiecesOffset ; ++i){
		if (p == PiecesNameSort[i]) return i;
	}
	return WRONG_PIECE;
}

Board::Board(char fenBoard[], char fenPlaying, char fenCastling[], int fenEnPX, int fenEnPY, int fenHC, int fenFM){
	//General initialize
	for (int i = 0 ; i < PIECESMAX ; ++i) Pieces[i] = bitboard(0);
	White_Pieces = bitboard(0);
	Black_Pieces = bitboard(0);
	WhiteKingSideCastling  = false;
	WhiteQueenSideCastling = false;
	BlackKingSideCastling  = false;
	BlackQueenSideCastling = false;
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
				if (ind == KING){
					++wk;
				} else if (ind == KING+BlackPiecesOffset){
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
		playingInt = blackInt;
		zobr ^= zobrist::blackKey;
	} else {
		playing = white;
		playingInt = whiteInt;
	}
	// 3. Castling availability
	ind = 0;
	while (fenCastling[ind] != '\0'){
		switch (fenCastling[ind++]){
		case 'K' :
			WhiteKingSideCastling = true;
			zobr ^= zobrist::White_King_Castling;
			break;
		case 'Q' :
			WhiteQueenSideCastling = true;
			zobr ^= zobrist::White_Queen_Castling;
			break;
		case 'k' :
			BlackKingSideCastling = true;
			zobr ^= zobrist::Black_King_Castling;
			break;
		case 'q' :
			BlackQueenSideCastling = true;
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
	assert(sq < 64 && sq >= 0);
	assert(ind >= 0 && ind <= 12);
	Pieces[ind] ^= filled::normal[sq];
	zobr ^= zobrist::keys[sq][ind];
	if (ind >= BlackPiecesOffset){
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
		for (int i = 0 ; i < BlackPiecesOffset ; ++i){
			dbgstream << ndbgline << "White " << PiecesName[i] << ": \n";
			printbb(Pieces[i]);
		}
		for (int i = 0 ; i < BlackPiecesOffset ; ++i){
			dbgstream << ndbgline << "Black " << PiecesName[i] << ": \n";
			printbb(Pieces[i+BlackPiecesOffset]);
		}
		dbgstream << ndbgline << "Zobrist Key: " << zobr << '\n';
		dbgstream << ndbgline << "--------------------------------" << endl;
	}
}

void Board::deactivateCastlingRights(){
	if (playing == white){
		if (WhiteKingSideCastling){
			WhiteKingSideCastling = false;
			zobr ^= zobrist::White_King_Castling;
		}
		if (WhiteQueenSideCastling){
			WhiteQueenSideCastling = false;
			zobr ^= zobrist::White_Queen_Castling;
		}
	} else {
		if (BlackKingSideCastling){
			BlackKingSideCastling = false;
			zobr ^= zobrist::Black_King_Castling;
		}
		if (BlackQueenSideCastling){
			BlackQueenSideCastling = false;
			zobr ^= zobrist::Black_Queen_Castling;
		}
	}
}

void Board::capture(int to){
	playingInt ^= BlackPiecesOffset;
	for (int i = playingInt ; i < playingInt+BlackPiecesOffset ; ++i){
		if ((Pieces[i] & filled::normal[to])!=0){
			updatePieces(to, i);
			playingInt ^= BlackPiecesOffset;
			return;
		}
	}
	playingInt ^= BlackPiecesOffset;
}
/**
 * ATTENTION!: move m has to be LEGAL (or null move)
 */
void Board::make(move m){
	assert(!moveIsNull(m));
	int from = index(m.fromX, m.fromY);
	int to = index(m.toX, m.toY);
	if ((Pieces[PAWN+playingInt] & filled::normal[from])!=0){
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
			updatePieces(from, playingInt+PAWN);
			if (rank(to)==lastRank){
				updatePieces(to, m.promoteTo+playingInt);
			} else {
				updatePieces(to, playingInt+PAWN);
			}
			capture(to);
			enPassant = bitboard(0);
		} else {
			if (from-to==index(0, 0)-index(0, 1) || to-from==index(0, 0)-index(0, 1)){
				//normal
				updatePieces(from, playingInt+PAWN);
				if (rank(to)==lastRank){
					updatePieces(to, m.promoteTo+playingInt);
				} else {
					updatePieces(to, playingInt+PAWN);
				}
				enPassant = bitboard(0);
			} else if (from-to==index(0, 0)-index(0, 2) || to-from==index(0, 0)-index(0, 2)){
				//double
				updatePieces(from, playingInt+PAWN);
				updatePieces(to, playingInt+PAWN);
				enPassant = filled::normal[(from+to)/2];
				zobr ^= zobrist::enPassant[7&square( enPassant )];
			} else {
				//en passant
				updatePieces(from, playingInt+PAWN);
				updatePieces(to, playingInt+PAWN);
				updatePieces(index(file(to), rank(from)), (playingInt^BlackPiecesOffset)+PAWN);
				enPassant = bitboard(0);
			}
		}
		halfmoves = 0;
		if (playing==black) ++fullmoves;
		togglePlaying();
	} else if ((Pieces[KING+playingInt] & filled::normal[from])!=0){
		//is King
		bool castl = false;
		if (playing == white){
			if (from == index('e'-'a', '1'-'1') && to == index('g'-'a', '1'-'1')){
				updatePieces(index(7, 0), ROOK);
				castl = true;
			} else if (from == index('e'-'a', '1'-'1') && to == index('c'-'a', '1'-'1')){
				updatePieces(index(0, 0), ROOK);
				castl = true;
			}
		} else {
			if (from == index('e'-'a', '8'-'1') && to == index('g'-'a', '8'-'1')){
				updatePieces(index(7, 7), ROOK+BlackPiecesOffset);
				castl = true;
			} else if (from == index('e'-'a', '8'-'1') && to == index('c'-'a', '8'-'1')){
				updatePieces(index(0, 7), ROOK+BlackPiecesOffset);
				castl = true;
			}
		}
		if (castl) {
			updatePieces((from+to)/2, ROOK+playingInt);
		}
		updatePieces(from, KING+playingInt);
		updatePieces(to, KING+playingInt);
		capture(to);
		if (playing==black) ++fullmoves;
		deactivateCastlingRights();
		togglePlaying();
		enPassant = bitboard(0);
	} else {
		for (int i = playingInt ; i < playingInt+BlackPiecesOffset ; ++i){
			if ((Pieces[i] & filled::normal[from])!=0){
				updatePieces(from, i);
				updatePieces(to, i);
				if (i-playingInt==ROOK){
					if (playing == white){
						if (from == index(7, 0)){
							if (WhiteKingSideCastling){
								WhiteKingSideCastling = false;
								zobr ^= zobrist::White_King_Castling;
							}
						} else if (from == index(0, 0)){
							if (WhiteQueenSideCastling){
								WhiteQueenSideCastling = false;
								zobr ^= zobrist::White_Queen_Castling;
							}
						}
					} else {
						if (from == index(7, 7)){
							if (BlackKingSideCastling){
								BlackKingSideCastling = false;
								zobr ^= zobrist::Black_King_Castling;
							}
						} else if (from == index(0, 7)){
							if (BlackQueenSideCastling){
								BlackQueenSideCastling = false;
								zobr ^= zobrist::Black_Queen_Castling;
							}
						}
					}
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
				for (int i = 0 ; i < BlackPiecesOffset ; ++i){
					if ((Pieces[i] & a) != 0){
						fen += PiecesNameSort[i];
					}
				}
			} else if ((Black_Pieces & a) !=0){
				if (count != 0){
						fen += count+'0';
						count = 0;
				}
				for (int i = 0 ; i < BlackPiecesOffset ; ++i){
					if ((Pieces[i+BlackPiecesOffset] & a) != 0){
						fen += PiecesNameSort[i]-'A'+'a';
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
	if (WhiteKingSideCastling) {
		fen += 'K';
		ncr = false;
	}
	if (WhiteQueenSideCastling){
		fen += 'Q';
		ncr = false;
	}
	if (BlackKingSideCastling) {
		fen += 'k';
		ncr = false;
	}
	if (BlackQueenSideCastling){
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
	fen += halfmoves + '0';
	fen += ' ';
	// 6. Fullmove number: The number of the full move.
	//		It starts at 1, and is incremented after Black's move.
	fen += fullmoves + '0';
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

int Board::perft(int depth){
	if (depth==0) return 1;
	zobr ^= zobrist::blackKey;
	int oldhm (halfmoves);
	halfmoves = 0;
	if (playing==black) ++fullmoves;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
	bitboard All_Pieces = White_Pieces | Black_Pieces;
	bitboard notAll_Pieces = ~All_Pieces;
	int moves = movePawnsByAttOrProm(depth, notAll_Pieces);
	bitboard tmpEnPassant (enPassant);
	enPassant = 0ull;
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
	int p[64], fromSq[64], n(0);
	bitboard knight = Pieces[KNIGHT+playingInt];
	while (knight!=0){
		from[n] = knight & -knight;
		fromSq[n] = square(from[n]);
		att[n] = KnightMoves[fromSq[n]];
		p[n] = KNIGHT+playingInt;
		++n;
		knight &= knight-1;
	}
	bitboard bishop = Pieces[BISHOP+playingInt];
	while (bishop!=0){
		from[n] = bishop & -bishop;
		fromSq[n] = square(from[n]);
		att[n] = bishopAttacks(All_Pieces, fromSq[n]);
		p[n] = BISHOP+playingInt;
		++n;
		bishop &= bishop-1;
	}
	bitboard rook = Pieces[ROOK+playingInt];
	while (rook!=0){
		from[n] = rook & -rook;
		fromSq[n] = square(from[n]);
		att[n] = rookAttacks(All_Pieces, fromSq[n]);
		p[n] = ROOK+playingInt;
		++n;
		rook &= rook-1;
	}
	bitboard queen = Pieces[QUEEN+playingInt];
	while (queen!=0){
		from[n] = queen & -queen;
		fromSq[n] = square(from[n]);
		att[n] = queenAttacks(All_Pieces, fromSq[n]);
		p[n] = QUEEN+playingInt;
		++n;
		queen &= queen-1;
	}
	p[n] = KING+playingInt;
	from[n] = Pieces[p[n]];
	fromSq[n] = square(from[n]);
	att[n] = KingMoves[fromSq[n]];
	//n : position of last bitboard generated

	//captures
	const int enQ = QUEEN + (playingInt^BlackPiecesOffset);
	const int enP = PAWN + (playingInt^BlackPiecesOffset);
	bitboard temp, cpt, to, ft;
	Zobrist toggle;
	int tosq;
	togglePlaying();
	for (int captured = enQ ; captured >= enP ; --captured){
		cpt = Pieces[captured];
		for (int i = 0 ; i <= n ; ++i){
			temp = att[i] & cpt;
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
						 zobrist::keys[from[i]][p[i]];
				zobr ^= toggle;
				addToHistory(zobr);
				if (validPosition()) moves += perft(depth-1);
				Pieces[captured] ^= to;
				Pieces[p[i]] ^= ft;
				*enemy ^= to;
				*ally ^= ft;
				toggle = zobrist::keys[tosq][captured] ^
						 zobrist::keys[tosq][p[i]] ^
						 zobrist::keys[from[i]][p[i]];
				zobr ^= toggle;
				removeLastHistoryEntry();
				temp &= temp-1;
			}
		}
	}
	halfmoves = oldhm+1;
	//TODO castling
//WARNING! : playing has the value of the player to play after this move right now.

	//normal moves of non-Pawns
	bitboard empty = ~All_Pieces;
	for (int i = 0 ; i <= n ; ++i){
		temp = att[i] & empty;
		while (temp != 0){
			to = temp & -temp;
			tosq = square(to);
			ft = to | from[i];
			Pieces[p[i]] ^= ft;
			*ally ^= ft;
			toggle = zobrist::keys[tosq][p[i]] ^ zobrist::keys[from[i]][p[i]];
			zobr ^= toggle;
			addToHistory(zobr);
			if (validPosition()) moves += perft(depth-1);
			Pieces[p[i]] ^= ft;
			*ally ^= ft;
			toggle = zobrist::keys[tosq][p[i]] ^ zobrist::keys[from[i]][p[i]];
			zobr ^= toggle;
			removeLastHistoryEntry();
			temp &= temp-1;
		}
	}
	//normal move pawns
	halfmoves = 0;
	moves += movePawnsForward(depth, notAll_Pieces);
	halfmoves = oldhm;
	togglePlaying();
	enPassant = tmpEnPassant;
	if (enPassant != 0) zobr ^= zobrist::enPassant[7&square( enPassant )];
	if (playing==black) --fullmoves;
	return moves;
}

void Board::addToHistory(Zobrist position){
	history[++lastHistoryEntry] = position;
}

void Board::removeLastHistoryEntry(){
	--lastHistoryEntry;
}

bool Board::validPosition() {
	//TODO check if the position is valid (king is not left to be captured)
	return true;
}
