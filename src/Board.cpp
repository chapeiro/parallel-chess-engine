/*
 * Board.cpp
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#include "Board.h"
#include <string>
#include "SquareMapping.h"
#include "MoveEncoding.h"
#include "Values.h"

const int Value::piece[12] = {100,
							 -100,
							  320,
							 -320,
							  325,
							 -325,
							  500,
							 -500,
							  975,
							 -975,
							  MAT,
							 -MAT};
int Board::hashSize = 1;

Board* Board::createBoard(const char FEN[]) {
	char fenBoard[71], fenEnP[3];
	char fenCastling[] = { '-', '-', '-', '-', '\0'};
	int fenHC, fenFM, fenEnPX, fenEnPY;
	char fenPlaying;
	sscanf(FEN, "%71s %c %4s %2s %d %d", fenBoard, &fenPlaying, fenCastling, fenEnP, &fenHC, &fenFM);
	if (fenEnP[0]=='-'){
		fenEnPX = -1;
		fenEnPY = -1;
	} else {
		fenEnPX = fenEnP[0]-'a';
		fenEnPY = fenEnP[1]-'1';
	}
	return new Board(fenBoard, fenPlaying, fenCastling, fenEnPX, fenEnPY, fenHC, fenFM);
}

Board::Board(char fenBoard[], char fenPlaying, char fenCastling[], int fenEnPX, int fenEnPY, int fenHC, int fenFM){
	//General initialize
	for (int i = 0 ; i < PIECESMAX ; ++i) Pieces[i] = bitboard(0);
	castling = 0;
	zobr = 0;
	lastHistoryEntry = 0;
	pieceScore = 0;
	int wk (0), bk(0);
	//FEN processing
	//	according to :
	//	http://en.wikipedia.org/w/index.php?title=Forsyth%E2%80%93Edwards_Notation&oldid=396200404
	// 1. Piece Placement
	int index1 = 0;
	int x, ind, sq;
	char inp;
	for (int y = 7 ; y >= 0 ; --y){
		x = 0;
		do {
			inp = fenBoard[index1++];
			if (inp < 'A'){
				x += inp-'0';
			} else {
				ind = getPieceIndex(inp);
				if ((ind & ~colormask) == WRONG_PIECE) throw MalformedFEN();
				if (ind == (KING | white)){
					++wk;
				} else if (ind == (KING | black)){
					++bk;
				} else {
					pieceScore += Value::piece[ind];
				}
				sq = index(x, y);
				updatePieces(sq, ind);
				++x;
			}
		} while (x <= 7);
		++index1;
	}
	if (wk!=1) throw KingException(white, wk);
	if (bk!=1) throw KingException(black, bk);
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
			castling |= castlingc<white>::KingSide;
			zobr ^= zobrist::White_King_Castling;
			break;
		case 'Q' :
			castling |= castlingc<white>::QueenSide;
			zobr ^= zobrist::White_Queen_Castling;
			break;
		case 'k' :
			castling |= castlingc<black>::KingSide;
			zobr ^= zobrist::Black_King_Castling;
			break;
		case 'q' :
			castling |= castlingc<black>::QueenSide;
			zobr ^= zobrist::Black_Queen_Castling;
			break;
		}
	}
	// 4. En passant target square in algebraic notation
	if (fenEnPX < 0 || fenEnPX > 7 || fenEnPY < 0 || fenEnPY > 7){
		enPassant = bitboard(0);
	} else {
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

int Board::getWhitePieceIndex(char p){
	const char PiecesNameSort[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	for (int i = 0 ; i < (LASTPIECE >> 1) ; ++i){
		if (PiecesNameSort[i] == p) return (i << 1) | white;
	}
	return WRONG_PIECE;
}

void Board::capture(int to){
	for (int i = playing^1 ; i < PIECESMAX ; i+=2){
		if ((Pieces[i] & filled::normal[to])!=0){
			pieceScore -= Value::piece[i];
			updatePieces(to, i);
			return;
		}
	}
}

/**
 * ATTENTION!: move m has to be LEGAL (or null move)
 */
void Board::make(move m){
	if (moveIsNull(m)){
		togglePlaying();
		return;
	}
	int from = index(m.fromX, m.fromY);
	int to = index(m.toX, m.toY);
	if ((Pieces[PAWN | playing] & filled::normal[from])!=0){
		//is Pawn
		int lastRank;
		if (playing==white){
			lastRank = lastRank_w;
		} else {
			lastRank = lastRank_b;
		}
		if ((Pieces[CPIECES | playing] & filled::normal[to])!=0){
			//capture
			updatePieces(from, playing | PAWN);
			if (rank(to)==lastRank){
				int a = getWhitePieceIndex(m.promoteTo);
				if (a == WRONG_PIECE) a = QUEEN;
				pieceScore += Value::piece[a | playing];
				pieceScore -= Value::piece[PAWN | playing];
				updatePieces(to, a | playing);
			} else {
				updatePieces(to, playing | PAWN);
			}
			capture(to);
			enPassant = bitboard(0);
		} else {
			if (from-to==index(0, 0)-index(0, 1) || to-from==index(0, 0)-index(0, 1)){
				//normal
				updatePieces(from, playing | PAWN);
				if (rank(to)==lastRank){
					int a = getWhitePieceIndex(m.promoteTo);
					if (a == WRONG_PIECE) a = QUEEN;
					pieceScore += Value::piece[a | playing];
					pieceScore -= Value::piece[PAWN | playing];
					updatePieces(to, a | playing);
				} else {
					updatePieces(to, playing | PAWN);
				}
				enPassant = bitboard(0);
			} else if (from-to==index(0, 0)-index(0, 2) || to-from==index(0, 0)-index(0, 2)){
				//double
				updatePieces(from, playing | PAWN);
				updatePieces(to, playing | PAWN);
				enPassant = filled::normal[(from+to)/2];
				zobr ^= zobrist::enPassant[7&square( enPassant )];
			} else {
				//en passant
				updatePieces(from, playing | PAWN);
				updatePieces(to, playing | PAWN);
				pieceScore -= Value::piece[PAWN | (playing^1)];
				updatePieces(index(file(to), rank(from)), (playing^1) | PAWN);
				enPassant = bitboard(0);
			}
		}
		halfmoves = 0;
		if (playing==black) ++fullmoves;
		togglePlaying();
	} else if ((Pieces[KING | playing] & filled::normal[from])!=0){
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
			updatePieces(from, ROOK | playing);
			updatePieces((from+to)/2, ROOK | playing);
		}
		updatePieces(from, KING | playing);
		updatePieces(to, KING | playing);
		capture(to);
		if (playing==black) ++fullmoves;
		if (playing==white){
			deactivateCastlingRights<white>();
		} else {
			deactivateCastlingRights<black>();
		}
		togglePlaying();
		enPassant = bitboard(0);
	} else {
		for (int i = playing ; i < LASTPIECE ; i+=2){
			if ((Pieces[i] & filled::normal[from])!=0){
				updatePieces(from, i);
				updatePieces(to, i);
				if ((i^playing)==ROOK && (filled::normal[from] & allcastlingrights) != 0){
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

/**
 * Use with caution, slow method
 *	Only for use in debugging.
 **/
std::string Board::getFEN(){
	//A FEN record contains six fields.
	//The separator between fields is a space.
	//	The fields are:
	std::string fen = "";
	const char PiecesNameSort[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
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
			if ((Pieces[CPIECES | white] & a) !=0){
				if (count != 0){
					fen += count+'0';
					count = 0;
				}
				for (int i = white ; i < LASTPIECE ; i+=2){
					if ((Pieces[i] & a) != 0){
						fen += PiecesNameSort[i>>1];
					}
				}
			} else if ((Pieces[CPIECES | black] & a) !=0){
				if (count != 0){
						fen += count+'0';
						count = 0;
				}
				for (int i = black ; i < LASTPIECE ; i+=2){
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
	if ((castling & castlingc<white>::KingSide) != 0) {
		fen += 'K';
		ncr = false;
	}
	if ((castling & castlingc<white>::QueenSide) != 0){
		fen += 'Q';
		ncr = false;
	}
	if ((castling & castlingc<black>::KingSide) != 0) {
		fen += 'k';
		ncr = false;
	}
	if ((castling & castlingc<black>::QueenSide) != 0){
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

/**
 * Use with caution, slow method (if debugcc==true)
 **/
void Board::print(){
	if (debugcc){
		std::cout << ndbgline << "--------------------------------\n";
		std::cout << ndbgline << getFEN() << '\n';
		for (int i = white ; i < LASTPIECE ; i+=2){
			std::cout << ndbgline << "White " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		for (int i = black ; i < LASTPIECE ; i+=2){
			std::cout << ndbgline << "Black " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		std::cout << ndbgline << "Zobrist Key: " << zobr << '\n';
		std::cout << ndbgline << "--------------------------------" << std::endl;
	}
}

void Board::printbb(bitboard bb){
	int i;
	for (int y = 7 ; y >= 0 ; --y){
		std::cout << ndbgline;
		for (int x = 0 ; x < 8 ; ++x){
			i = index(x, y);
			std::cout << ((bb & filled::normal[i]) >> i);
		}
		std::cout << '\n';
	}
}

U64 Board::perft(int depth){
	horizonNodes = 0;
	nodes = 0;
	key oldZobr = zobr;
	bitboard oldep = enPassant;
	int oldhm = halfmoves;
	int oldfm = fullmoves;
	int oldpl = playing;
	bitboard oldcstl = castling;
	int oldlhe = lastHistoryEntry;
	int psc = pieceScore;
	bool failed = false;
	if (playing == white){
		search<Perft, white>(-inf, +inf, depth);
	} else {
		search<Perft, black>(-inf, +inf, depth);
	}
	if (oldZobr != zobr) {
		std::cout << oldZobr << "|z" << zobr << std::endl;
		failed = true;
		key k = zobr ^ oldZobr;
		if (k == zobrist::blackKey) std::cout << "blackKey\n" << std::endl;
		for (int i = 0 ; i < 16 ; ++i){
			if ( k == zobrist::castling[i]) std::cout << "castling[" << i << ']' << std::endl;
		}
		for (int i = 0 ; i < 8 ; ++i){
			if ( k == zobrist::enPassant[i]) std::cout << "enPassant[" << i << ']' << std::endl;
		}
		for (int i = 0 ; i < 64 ; ++i){
			for (int j = 0 ; j < 12 ; ++j){
				if ( k == zobrist::keys[i][j]) std::cout << "keys[" << i << "][" << j << ']' << std::endl;
			}
		}
	}
	if (oldep != enPassant) {std::cout << oldep << "|ep" << enPassant << std::endl; failed = true;}
	if (oldhm != halfmoves) {std::cout << oldhm << "|hm" << halfmoves << std::endl; failed = true;}
	if (oldfm != fullmoves) {std::cout << oldfm << "|fm" << fullmoves << std::endl; failed = true;}
	if (oldpl != playing) {std::cout << oldpl << "|p" << playing << std::endl; failed = true;}
	if (oldcstl != castling) {std::cout << oldcstl << "|c" << castling << std::endl; failed = true;}
	if (oldlhe != lastHistoryEntry) {std::cout << oldlhe << "|lhe" << lastHistoryEntry << std::endl; failed = true;}
	if (psc != pieceScore) {std::cout << psc << "|pS" << pieceScore << std::endl; failed = true;}
	if (failed) return 0;
	return horizonNodes;
}

int Board::test(int depth){
	horizonNodes = 0;
	nodes = 0;
	key oldZobr = zobr;
	bitboard oldep = enPassant;
	int oldhm = halfmoves;
	int oldfm = fullmoves;
	int oldpl = playing;
	bitboard oldcstl = castling;
	int oldlhe = lastHistoryEntry;
	int psc = pieceScore;
	bool failed = false;
	int score;
	if (playing == white){
		score = search<PV, white>(-inf, +inf, depth);
	} else {
		score = -search<PV, black>(-inf, +inf, depth);
	}
	if (oldZobr != zobr) {
		std::cout << oldZobr << "|z" << zobr << std::endl;
		failed = true;
		key k = zobr ^ oldZobr;
		if (k == zobrist::blackKey) std::cout << "blackKey\n" << std::endl;
		for (int i = 0 ; i < 16 ; ++i){
			if ( k == zobrist::castling[i]) std::cout << "castling[" << i << ']' << std::endl;
		}
		for (int i = 0 ; i < 8 ; ++i){
			if ( k == zobrist::enPassant[i]) std::cout << "enPassant[" << i << ']' << std::endl;
		}
		for (int i = 0 ; i < 64 ; ++i){
			for (int j = 0 ; j < 12 ; ++j){
				if ( k == zobrist::keys[i][j]) std::cout << "keys[" << i << "][" << j << ']' << std::endl;
			}
		}
	}
	if (oldep != enPassant) {std::cout << oldep << "|ep" << enPassant << std::endl; failed = true;}
	if (oldhm != halfmoves) {std::cout << oldhm << "|hm" << halfmoves << std::endl; failed = true;}
	if (oldfm != fullmoves) {std::cout << oldfm << "|fm" << fullmoves << std::endl; failed = true;}
	if (oldpl != playing) {std::cout << oldpl << "|p" << playing << std::endl; failed = true;}
	if (oldcstl != castling) {std::cout << oldcstl << "|c" << castling << std::endl; failed = true;}
	if (oldlhe != lastHistoryEntry) {std::cout << oldlhe << "|lhe" << lastHistoryEntry << std::endl; failed = true;}
	if (psc != pieceScore) {std::cout << psc << "|pS" << pieceScore << std::endl; failed = true;}
	if (failed) return 0;
	return score;
}

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

bitboard Board::queenAttacks(bitboard occ, const int &sq){
	return rookAttacks(occ, sq) | bishopAttacks(occ, sq);
}
