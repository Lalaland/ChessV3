#ifndef CHESS_ENGINE_H_INCLUDED
#define CHESS_ENGINE_H_INCLUDED

#include "myvector2.h"

#include <boost/utility.hpp>
#include <vector>
#include <set>
#include <bitset>

#include <boost/optional.hpp>

class t_message;

class t_chessEngine : boost::noncopyable
{
public:
   t_chessEngine();

   void reset();
   bool getTurn();
   bool inPromotion();
   std::vector<t_message> boardClickedSingle(const t_message &message);
   std::vector<t_message> insertMove(const t_myVector2 &oldPos, const t_myVector2 &newPos);
   std::vector<t_message> insertMove(const t_myVector2 &oldPos, const t_myVector2 &newPos, int type);


private:
   void generateMoves(const t_myVector2 &pos);
   void removeCastle(const t_myVector2 &pos, const t_myVector2 &oldPos);

   boost::optional<t_myVector2> checkCheck(const t_myVector2 &pos, const t_myVector2 &oldPos);
   bool checkCheckmate();

   void addPieceMoves(const t_myVector2 &pos, std::bitset<64> &defendingPieces);

   void castleMove(t_myVector2 pos, std::vector<t_message> &buffer);
   void attackMove( t_myVector2 pos, std::vector<t_message> &buffer);
   void moveMove(t_myVector2 pos, std::vector<t_message> &buffer);
   void promoteMove(t_myVector2 pos, std::vector<t_message> &buffer);
   void pessantMove(t_myVector2 pos, std::vector<t_message> &buffer);
   void clearHighlights(std::vector<t_message> &buffer);


   void finishPromote(int type,std::vector<t_message> &buffer);

   std::vector<t_myVector2> move;
   std::vector<t_myVector2> hit;
   std::vector<t_myVector2> castle;
   std::vector<t_myVector2> pawnPromote;
   std::vector<t_myVector2> pawnMove;
   std::vector<t_myVector2> pawnPessant;

   int enPessantPossible;
   t_myVector2 enPessantPos;
   t_myVector2 enPessantPawn;

   char board[8][8];

   t_myVector2 selectedPos;
   bool selected;

   bool whiteCanCastleLeft;
   bool whiteCanCastleRight;

   bool blackCanCastleLeft;
   bool blackCanCastleRight;

   bool turn;

   bool inPawnPromotion;
   t_myVector2 pawnPos;

   t_myVector2 whiteKingPos;
   t_myVector2 blackKingPos;

   std::set<t_myVector2> whitePieces;
   std::set<t_myVector2> blackPieces;
};

#endif
