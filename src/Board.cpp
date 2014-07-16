/*
 * Board.cpp
 *
 *  Created on: 2 ��� 2011
 *      Author: Chrysogelos Periklis
 */

#include "Board.hpp"
#include <cstring>
#include <string>
#include "SquareMapping.hpp"
#include "MoveEncoding.hpp"
#include "Values.hpp"
#include <ctime>
#include <atomic>

Board Board::bmem[MAX_BOARDS];
std::mutex Board::bmem_m;

node_statistics gstats;

unsigned int Board::bmem_unused[MAX_BOARDS];
unsigned int Board::bmem_unused_last = MAX_BOARDS << 1;

std::atomic<bool> interruption_requested;

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

Board::Board(Board * __restrict b){
	for (int i = 0 ; i < PIECESMAX ; ++i) Pieces[i] = b->Pieces[i];
	White_Pieces = b->White_Pieces;
	Black_Pieces = b->Black_Pieces;
	castling = b->castling;
	zobr = b->zobr;
	lastHistoryEntry = b->lastHistoryEntry;
	pieceScore = b->pieceScore;
	playing = b->playing;
	//memcpy(kingSq, b->kingSq, (colormask + 1)*sizeof(*kingSq));
	enPassant = b->enPassant;
	halfmoves = b->halfmoves;
	fullmoves = b->fullmoves;
	stats.reset();
	//Memory
	memcpy(history, b->history, sizeof(history));
}

Board::Board(char fenBoard[], char fenPlaying, char fenCastling[], int fenEnPX, int fenEnPY, int fenHC, int fenFM){
	//General initialize
	for (int i = 0 ; i < PIECESMAX ; ++i) Pieces[i] = bitboard(0);
	All_Pieces(white) = All_Pieces(black) = bitboard(0);
	castling = 0;
	zobr = 0;
	lastHistoryEntry = -1;
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
				sq = index(x, y);
				if (ind == (KING | white)){
					++wk;
					//kingSq[white] = sq;
				} else if (ind == (KING | black)){
					++bk;
					//kingSq[black] = sq;
				} else {
					pieceScore += Value::piece[ind];
				}
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
			zobr     ^= zobrist::White_King_Castling;
			break;
		case 'Q' :
			castling |= castlingc<white>::QueenSide;
			zobr     ^= zobrist::White_Queen_Castling;
			break;
		case 'k' :
			castling |= castlingc<black>::KingSide;
			zobr     ^= zobrist::Black_King_Castling;
			break;
		case 'q' :
			castling |= castlingc<black>::QueenSide;
			zobr     ^= zobrist::Black_Queen_Castling;
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

int Board::getWhitePieceIndex(char p) __restrict{
	const char PiecesNameSort[] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	for (int i = 0 ; i < (LASTPIECE >> 1) ; ++i){
		if (PiecesNameSort[i] == p) return (i << 1) | white;
	}
	return WRONG_PIECE;
}

void Board::capture(int to) __restrict{
	for (int i = playing^1 ; i < PIECESMAX ; i+=2){
		if ((Pieces[i] & filled::normal[to])!=0){
			pieceScore -= Value::piece[i];
			updatePieces(to, i);
			return;
		}
	}
	++halfmoves;
}

/**
 * ATTENTION!: move m has to be LEGAL (or null move)
 */
void Board::make(chapeiro::move m){
	//FIXME fic
	if (chapeiro::moveIsNull(m)){
		togglePlaying();
		return;
	}

	if (enPassant) zobr ^= zobrist::enPassant[7&square(enPassant)];

	int from = index(m.fromX, m.fromY);
	int to = index(m.toX, m.toY);
	if ((Pieces[PAWN | playing] & filled::normal[from])!=0){
		//is Pawn
		int lastRank;
		if (playing==white){
			lastRank = 7;
		} else {
			lastRank = 0;
		}
		if ((All_Pieces(playing^1) & filled::normal[to])!=0){
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
				unsigned long int tmpSq = square(enPassant);
				zobr ^= zobrist::enPassant[7&tmpSq];
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
			int rookFrom = (to > from) ? index('a'-'a', m.fromY) : index('h'-'a', m.fromY);
			updatePieces(rookFrom, ROOK | playing);
			updatePieces((from+to)/2, ROOK | playing);
		}
		updatePieces(from, KING | playing);
		updatePieces(to, KING | playing);
		capture(to);
		if (playing==black) ++fullmoves;
		if (playing==white){
			deactivateCastlingRights<white>();
			//kingSq[white] = square(to);
		} else {
			deactivateCastlingRights<black>();
			//kingSq[black] = square(to);
		}
		togglePlaying();
		enPassant = bitboard(0);
	} else {
		for (int i = playing ; i < LASTPIECE ; i+=2){
			if ((Pieces[i] & filled::normal[from])!=0){
				updatePieces(from, i);
				updatePieces(to, i);
				if ((i&(~colormask))==ROOK && (filled::normal[from] & castling) != 0){
						zobr ^= zobrist::castling[(castling*castlingsmagic)>>60];
						castling ^= filled::normal[from];
						zobr ^= zobrist::castling[(castling*castlingsmagic)>>60];
				}/** else if ((i^(~colormask)) == KING){
					if (playing==white){
						kingSq[white] = square(to);
					} else {
						kingSq[black] = square(to);
					}
				}**/
				break;
			}
		}
		capture(to);
		if (playing==black) ++fullmoves;
		togglePlaying();
		enPassant = bitboard(0);
	}
	addToHistory(zobr);
	forgetOldHistory();
}

/**
 * Use with caution, slow method
 *	Only for use in debugging.
 **/
std::string Board::getFEN() __restrict{ return getFEN(playing); }

/**
 * Use with caution, slow method
 *	Only for use in debugging.
 **/
std::string Board::getFEN(int playingl) __restrict{
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
			if ((All_Pieces(white) & a) !=0){
				if (count != 0){
					fen += count+'0';
					count = 0;
				}
				for (int i = white ; i < LASTPIECE ; i+=2){
					if ((Pieces[i] & a) != 0){
						fen += PiecesNameSort[i>>1];
					}
				}
			} else if ((All_Pieces(black) & a) !=0){
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
	fen += ( playingl==white ? 'w' : 'b' );
	fen += ' ';
	//	3. Castling availability. If neither side can castle,
	//		this is "-". Otherwise, this has one or more letters:
	//		"K" (White can castle kingside),
	//		"Q" (White can castle queenside),
	//		"k" (Black can castle kingside) and/or
	//		"q" (Black can castle queenside).
	bool ncr = true;
	//TODO reconsider & Pieces[ROOK | color]
	if ((castling & castlingc<white>::KingSide & Pieces[ROOK | white]) != 0) {
		fen += 'K';
		ncr = false;
	}
	if ((castling & castlingc<white>::QueenSide & Pieces[ROOK | white]) != 0){
		fen += 'Q';
		ncr = false;
	}
	if ((castling & castlingc<black>::KingSide & Pieces[ROOK | black]) != 0) {
		fen += 'k';
		ncr = false;
	}
	if ((castling & castlingc<black>::QueenSide & Pieces[ROOK | black]) != 0){
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
		std::cerr << ndbgline << "--------------------------------\n";
		std::cerr << ndbgline << getFEN(playing) << '\n';
		for (int i = white ; i < LASTPIECE ; i+=2){
			std::cerr << ndbgline << "White " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		for (int i = black ; i < LASTPIECE ; i+=2){
			std::cerr << ndbgline << "Black " << PiecesName[i>>1] << ": \n";
			printbb(Pieces[i]);
		}
		unsigned long int kingSqW = square(Pieces[KING | white]);
		unsigned long int kingSqB = square(Pieces[KING | black]);
		std::cerr << ndbgline << "White King square : " << kingSqW << "\n";
		std::cerr << ndbgline << "Black King square : " << kingSqB << "\n";
		const bitboard occ = All_Pieces(white) | All_Pieces(black);
		printbb(kingIsAttackedBy<white>(occ, kingSqW));
		//assert((1ull << kingSq[white]) == Pieces[KING | white]);
		printbb(kingIsAttackedBy<black>(occ, kingSqB));
		//assert((1ull << kingSq[black]) == Pieces[KING | black]);
		std::cerr << ndbgline << "Black checking Pieces : \n";
		printbb(kingIsAttackedBy<white>(occ, kingSqW));
		std::cerr << ndbgline << "White checking Pieces : \n";
		printbb(kingIsAttackedBy<black>(occ, kingSqB));
		std::cerr << ndbgline << "White Pieces : \n";
		printbb(All_Pieces(white));
		std::cerr << ndbgline << "Black Pieces : \n";
		printbb(All_Pieces(black));
		std::cerr << ndbgline << "Zobrist Key: " << zobr << '\n';
		std::cerr << ndbgline << "static score : " << pieceScore << '\n';
		//std::cerr << ndbgline << "History : " << '\n';
		//for (int i = 0 ; i < 256 ; ++i) std::cerr << ndbgline << "history[" << std::dec << std::setw(3) << i << "] : " << std::hex << std::setw(16) << history[i] << '\n';
		printHistory();
		std::cerr << ndbgline << "--------------------------------" << std::endl;
	}
}

void Board::printHistory(){
	std::cerr << ndbgline << "Half Moves Clock : " << halfmoves << '\n';
	std::cerr << ndbgline << "History : " << '\n';
	for (int i = 0 ; i <= lastHistoryEntry ; ++i) std::cerr << ndbgline << "history[" << std::dec << std::setw(3) << i << "] : " << std::hex << std::setw(16) << history[i] << '\n';
}

U64 Board::perft(int depth){
	interruption_requested = false;
	gstats.reset();
	stats.reset();
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
		search<Perft, white, true>(-inf, +inf, depth);
	} else {
		search<Perft, black, true>(-inf, +inf, depth);
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
		for (int j = 0 ; j < 12 ; ++j){
			for (int i = 0 ; i < 64 ; ++i){
				if ( k == zobrist::keys[j][i]) std::cout << "keys[" << j << "][" << i << ']' << std::endl;
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
	return gstats.horizonNodes + stats.horizonNodes;
}

int Board::test(int depth){
	gstats.horizonNodes = 0;
	gstats.qNodes = 0;
	gstats.nodes = 0;
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
		score = search<PV, white, true>(-inf, +inf, depth);
	} else {
		score = -search<PV, black, true>(-inf, +inf, depth);
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
		for (int j = 0 ; j < 12 ; ++j){
			for (int i = 0 ; i < 64 ; ++i){
				if ( k == zobrist::keys[j][i]) std::cout << "keys[" << j << "][" << i << ']' << std::endl;
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

// void Board::go(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch){
// 	searchThread = new thread(&Board::startSearch, this, maxDepth, wTime, bTime, wInc, bInc, movesUntilTimeControl, searchForXMsec, infinitiveSearch);
// }

// void Board::go(int maxDepth, time_control tc){
// 	searchThread = new thread(&Board::startSearchTM, this, maxDepth, tc);
// }

// void Board::stop(){
// 	if (searchThread == NULL) return;
// 	interruption_requested = true;
// 	searchThread->join();
// 	delete searchThread;
// 	searchThread = NULL;
// }


void Board::go(int maxDepth, time_control tc){
	interruption_requested = false;
	gstats.reset();
	stats.reset();
	TimeManager<> tm(tc, (playing == white) ? WHITE : BLACK, &interruption_requested);

	int depth = (STARTING_DEPTH < maxDepth) ? STARTING_DEPTH : 1; //STARTING_DEPTH
	int alpha = -inf;
	int beta = inf;
	int move = 0;
	int score = 0;
	int matdcycles = 0;
	U64 stNodes = gstats.nodes;
	if (debugcc) std::cerr << ndbgline << "0x" << std::hex << std::setw(16) << zobr << std::dec << std::endl;
	while ((depth <= maxDepth && (!move || tm.continue_search(ELAPSED_TIME_FACTOR)) && matdcycles < 3)){
		rootDepth = depth;
		ttPreparePVS(zobr);
		if (playing == white){
			score = search<PV, white, true>(alpha, beta, depth);
		} else {
			score = search<PV, black, true>(alpha, beta, depth);
		}
		if (interruption_requested) {
			//ttNewGame(); //FIXME if this is not used TT will have invalid entries!!! But this is bad for later searches in the same game!
			break; //DO NOT USE THE SCORE RETURNED BY SEARCH!!! IT IS NOT VALID!!!
		}
		int tmove = getBestMove(zobr);
		if (tmove | move) tm.search_lap_tick();
		move = tmove;
		
		std::chrono::milliseconds etime(tm.getElapsedTime());
		//Sending Infos
		std::cout << "info";
		std::cout << " depth " << depth;
		std::cout << " time " << etime.count();
		std::cout << " nodes " << (gstats.nodes+stats.nodes)-stNodes;
		std::chrono::seconds secs(std::chrono::duration_cast<std::chrono::seconds>(etime));
		if (secs.count() > 0) std::cout << " nps " << (U64) (((gstats.nodes+stats.nodes)-stNodes) / secs.count());

		Board * extrPv = new Board(this);
		std::cout << " pv " << extrPv->extractPV(depth);
		if (isMat(score)) {
			std::cout << " score mate ";
			if (score < 0) std::cout << '-';
			int matmoves = ((Value::MAT - abs(score) - 1)/2) + 1;
			std::cout << matmoves;
			++matdcycles;
		} else {
			matdcycles = 0;
			std::cout << " score cp " << score;
		}
		std::cout << " hashfull " << (1000*ttUsed/TRANSPOSITION_TABLE_SIZE);
		delete extrPv;
		std::cout << std::endl;
		std::cout.flush();
		if (debugcc) std::cerr << ndbgline << "Score : " << score << std::endl;
		++depth;
	}
	if (move){
		char m[6];
		std::cout << "bestmove " << moveToString(move, m) << std::endl;
		if (depth > maxDepth) std::cerr << ndbgline << "Maximum Depth reached!" << std::endl;
	} else {
		std::cerr << ndbgline << "Move not found in hashtable !!!" << std::endl;
	}
}

int rootDepth = 0;

// void Board::startSearch(int maxDepth, U64 wTime, U64 bTime, U64 wInc, U64 bInc, int movesUntilTimeControl, U64 searchForXMsec, bool infinitiveSearch){
// 	time_td startTime = get_current_time();
// 	interruption_requested = false;
// 	if (movesUntilTimeControl == NO_NEXT_TIME_CONTROL) movesUntilTimeControl = 40;
// 	U64 timeToSearch = ((playing == white) ? wTime : bTime) / (movesUntilTimeControl);
// 	time_td searchEndTime = startTime + milli_to_time(timeToSearch < searchForXMsec ? timeToSearch : searchForXMsec);
// 	time_duration elapsedTime = get_zero_time();
// 	time_td currentTime;
// 	int depth = (STARTING_DEPTH < maxDepth) ? STARTING_DEPTH : 1; //STARTING_DEPTH
// 	int alpha = -inf;
// 	int beta = inf;
// 	int move = 0;
// 	int score = 0;
// 	int matdcycles = 0;
// 	U64 stNodes = gstats.nodes;
// 	if (debugcc) std::cerr << ndbgline << "0x" << std::hex << std::setw(16) << zobr << std::dec << std::endl;
// 	Board * extrPv = NULL;
// 	while (depth <= maxDepth && (infinitiveSearch || ((searchEndTime - get_current_time()) > elapsedTime*ELAPSED_TIME_FACTOR)) && matdcycles < 3){
// 		rootDepth = depth;
// 		if (playing == white){
// 			score = search<PV, white, true>(alpha, beta, depth);
// 		} else {
// 			score = search<PV, black, true>(alpha, beta, depth);
// 		}
// 		elapsedTime = get_current_time() - startTime;
// 		if (interruption_requested) {
// 			ttNewGame(); //FIXME if this is not used TT will have invalid entries!!! But this is bad for later searches in the same game!
// 			break; //DO NOT USE THE SCORE RETURNED BY SEARCH!!! IT IS NOT VALID!!!
// 		}
// 		move = getBestMove(zobr);
// 		//Sending Infos
// 		std::cout << "info";
// 		std::cout << " depth " << depth;
// #if defined _MSC_VER && _MSC_VER <= 1600
// 		std::cout << " time " << elapsedTime.total_milliseconds();
// #else
// 		std::cout << " time " << (elapsedTime.count()/1000);
// #endif
// 		std::cout << " nodes " << gstats.nodes-stNodes;
// #if defined _MSC_VER && _MSC_VER <= 1600
// 		if (elapsedTime.total_milliseconds() != 0ull) std::cout << " nps " << ((gstats.nodes-stNodes)*1000ull) / (elapsedTime.total_milliseconds());
// #else
// 		if (elapsedTime.count() >= 1000) std::cout << " nps " << ((gstats.nodes-stNodes)*1000ull) / (elapsedTime.count() / 1000.0);
// #endif
// 		extrPv = new Board(this);
// 		std::cout << " pv " << extrPv->extractPV(depth);
// 		if (isMat(score)) {
// 			std::cout << " score mate ";
// 			if (score < 0) std::cout << '-';
// 			int matmoves = ((Value::MAT - abs(score) - 1)/2) + 1;
// 			std::cout << matmoves;
// 			++matdcycles;
// 		} else {
// 			matdcycles = 0;
// 			std::cout << " score cp " << score;
// 		}
// 		std::cout << " hashfull " << (1000*ttUsed/TRANSPOSITION_TABLE_SIZE);
// 		delete extrPv;
// 		std::cout << std::endl;
// 		std::cout << bmem_unused_last << std::endl;
// 		if (debugcc) std::cerr << ndbgline << "Score : " << score << std::endl;
// 		++depth;
// 	}
// 	if (move != 0){
// 		char m[6];
// 		std::cout << "bestmove " << moveToString(move, m) << std::endl;
// 		if (depth > maxDepth) std::cerr << ndbgline << "Maximum Depth reached!" << std::endl;
// 	} else {
// 		std::cerr << ndbgline << "Move not found in hashtable !!!" << std::endl;
// 	}
// }


std::string Board::extractPV(int depth){
	int move;
	if (depth == 0 || (move = getBestMove(zobr)) == 0) return "";
	std::string pv = "";
	char m[6];
	moveToString(move, m);
	make(chapeiro::convertUCImove(m));
	pv += m;
	pv += " " + extractPV(depth - 1);
	return pv;
}

char * Board::moveToString(int move, char* m) const __restrict{
	int fromSq = getTTMove_From(move);
	int toSq = getTTMove_To(move);
	int prom = getTTMove_Prom_spec(move);
	m[0] = (char) ('a'+file(fromSq));
	m[1] = (char) ('1'+rank(fromSq));
	m[2] = (char) ('a'+file(toSq));
	m[3] = (char) ('1'+rank(toSq));
	if (((bitboard(1) << fromSq) & Pieces[PAWN | playing]) && (prom & (~TTMove_EnPassantFlag & ~colormask)) != PAWN){
		const char PiecesNameSort[] = {'p', 'n', 'b', 'r', 'q', 'k'};
		m[4] = PiecesNameSort[prom >> 1];
		m[5] = '\0';
	} else {
		m[4] = '\0';
	}
	return m;
}

void Board::forgetOldHistory() __restrict{
	if (lastHistoryEntry < 50 || halfmoves >= lastHistoryEntry || halfmoves == 0) return;
	int offset = lastHistoryEntry - halfmoves;
	for (int i = 0 ; i <= halfmoves ; ++i) history[i] = history[i+offset];
	lastHistoryEntry = halfmoves;
}


time_td get_current_time(){
#if defined _MSC_VER && _MSC_VER <= 1600
	return boost::posix_time::microsec_clock::universal_time();
#else
	return clock_ns::now();
#endif
}

time_duration milli_to_time(U64 milli){
#if defined _MSC_VER && _MSC_VER <= 1600
	return boost::posix_time::milliseconds(milli);
#else
	return std::chrono::milliseconds(milli);
#endif
}

time_duration get_zero_time(){
#if defined _MSC_VER && _MSC_VER <= 1600
	return boost::posix_time::milliseconds(0);
#else
	return std::chrono::nanoseconds(0);
#endif
}

int Board::search(int depth, int alpha, int beta) __restrict{
	int score;
	if (playing == white){
		score = -search<ZW, white, true>(-1-alpha, -alpha, depth - 1);
		if ( score > alpha ) score = -search<PV, white, true>(-beta, -alpha, depth - 1);
	} else {
		score = -search<ZW, black, true>(-1-alpha, -alpha, depth - 1);
		if ( score > alpha ) score = -search<PV, black, true>(-beta, -alpha, depth - 1);
	}
	return score;
}

int Board::searchT(int depth, int alpha, int beta) __restrict{
	return search(depth, alpha, beta);
}

void Board::setThreadID(unsigned int thrd_id){
	this->thread_id = thrd_id;
}

void Board::setWindow(Window * w){
	this->w = w;
}


void * Board::operator new(size_t size){
	std::lock_guard<std::mutex> lk(bmem_m);
	if (bmem_unused_last > MAX_BOARDS) {
		bmem_unused_last = MAX_BOARDS;
		for (unsigned int i = 0 ; i < MAX_BOARDS ; ++i) bmem_unused[i] = i;
	}
	if (bmem_unused_last == 0) throw std::bad_alloc();
	return &(bmem[bmem_unused[--bmem_unused_last]]);
}

void Board::operator delete(void * p){
	std::lock_guard<std::mutex> lk(bmem_m);
	bmem_unused[bmem_unused_last++] = (((Board *) p) - bmem);
	assert((((Board *) p) - bmem) < (MAX_BOARDS));
	assert(bmem_unused_last <= MAX_BOARDS);
	gstats += ((Board *) p)->stats;
	((Board *) p)->stats.reset();
}




