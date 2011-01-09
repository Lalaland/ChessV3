#include <iostream>

#include <boost/thread.hpp>

#include <cstdio>

#include "chessCli.h"
#include "../messages.h"

t_chessCli::t_chessCli(t_sharedData &theSharedData) : sharedData(theSharedData), selected(0)
{
   std::cout<<"Hello world"<<std::endl;

   board =
   {
      {10, 11, 12, 14, 13, 12, 11, 10},
      { 9,  9,  9,  9,  9,  9,  9,  9},
      { 0,  0,  0,  0,  0,  0,  0,  0},
      { 0,  0,  0,  0,  0,  0,  0,  0},
      { 0,  0,  0,  0,  0,  0,  0,  0},
      { 0,  0,  0,  0,  0,  0,  0,  0},
      { 1,  1,  1,  1,  1,  1,  1,  1},
      { 2,  3,  4,  6,  5,  4,  3,  2}
   };

   whiteCanCastleLeft = 1;
   whiteCanCastleRight = 1;

   blackCanCastleLeft = 1;
   blackCanCastleRight = 1;

   turn = 0;

   whiteKingPos.x = 4;
   blackKingPos.x = 4;

   whiteKingPos.y = 7;
   blackKingPos.y = 0;


   t_myVector2 pos;

   for (int y = 6;y < 8;y++)
   {
      pos.y = y;
      for (int x = 0; x<8;x++)
      {
         pos.x = x;
         whitePieces.insert(pos);
      }
   }

   for (int y = 0;y < 2;y++)
   {
      pos.y = y;
      for (int x = 0; x<8;x++)
      {
         pos.x = x;
         blackPieces.insert(pos);
      }
   }
}

void t_chessCli::run()
{
   for (;;)
   {
      t_message message;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

         if (sharedData.gameBuffer.empty())
         {
            sharedData.gameCondition.wait(lock);
         }

         message = sharedData.gameBuffer.back();
         sharedData.gameBuffer.pop_back();

      }

      std::cout<<"I have recieved a message"<<std::endl;

      switch (message.id)
      {

      case BOARD_CLICKED:
      {
         t_myVector2 pos = message.boardClicked.pos;
         std::cout<<"It was clicked at "<<pos<<std::endl;

         t_message newMessage;
         newMessage.id = HIGHLIGHT_SPACE;


         if (selected)
         {
            if (pos == selectedPos)
            {
               selected = 0;
               newMessage.highlightSpace.pos = selectedPos;
               newMessage.highlightSpace.color = 0;
               {
                  boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

                  sharedData.clientBuffer.push_front(newMessage);

                  for (auto iter = move.begin(); iter != move.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = hit.begin(); iter != hit.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = castle.begin(); iter != castle.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }
               }

               move.clear();
               hit.clear();
               castle.clear();
            }

            else if (std::find(move.begin(), move.end(), pos) != move.end())
            {
               if (checkCheck(pos,selectedPos))
               {
                  std::cout<<"I am in check"<<std::endl;
                  break;
               }

               if (turn == 0)
               {
                  whitePieces.erase(selectedPos);
                  whitePieces.insert(pos);
               }

               else
               {
                  blackPieces.erase(selectedPos);
                  blackPieces.insert(pos);
               }

               removeCastle(pos,selectedPos);

               selected = 0;
               newMessage.highlightSpace.pos = selectedPos;
               newMessage.highlightSpace.color = 0;
               
               {
                  boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

                  sharedData.clientBuffer.push_front(newMessage);

                  for (auto iter = move.begin(); iter != move.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = hit.begin(); iter != hit.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = castle.begin(); iter != castle.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  newMessage.id = MOVE_PIECE;
                  newMessage.movePiece.pos = pos;
                  newMessage.movePiece.oldPos = selectedPos;

                  sharedData.clientBuffer.push_front(newMessage);
               }

               move.clear();
               hit.clear();
               castle.clear();

               board[pos.y][pos.x] = board[selectedPos.y][selectedPos.x];
               board[selectedPos.y][selectedPos.x] = 0;

               turn = !turn;

               if (checkCheckmate())
               {
                  std::cout<<"I win"<<std::endl;
               }
            }

            else if (std::find(hit.begin(), hit.end(), pos) != hit.end())
            {
               if (checkCheck(pos,selectedPos))
               {
                  std::cout<<"I am in check"<<std::endl;
                  break;
               }

               if (turn == 0)
               {
                  whitePieces.erase(selectedPos);
                  whitePieces.insert(pos);

                  blackPieces.erase(pos);
               }

               else
               {
                  blackPieces.erase(selectedPos);
                  blackPieces.insert(pos);

                  whitePieces.erase(pos);
               }

               removeCastle(pos,selectedPos);

               selected = 0;
               newMessage.highlightSpace.pos = selectedPos;
               newMessage.highlightSpace.color = 0;
               
               {
                  boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

                  sharedData.clientBuffer.push_front(newMessage);

                  for (auto iter = move.begin(); iter != move.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = hit.begin(); iter != hit.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = castle.begin(); iter != castle.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  newMessage.id = CAPTURE_PIECE;
                  newMessage.movePiece.pos = pos;
                  newMessage.movePiece.oldPos = selectedPos;

                  sharedData.clientBuffer.push_front(newMessage);
               }

               move.clear();
               hit.clear();
               castle.clear();

               board[pos.y][pos.x] = board[selectedPos.y][selectedPos.x];
               board[selectedPos.y][selectedPos.x] = 0;

               turn = !turn;

               if (checkCheckmate())
               {
                  std::cout<<"I win"<<std::endl;
               }
            }

            else if (std::find(castle.begin(), castle.end(), pos) != castle.end())
            {
               if (checkCheck(pos,selectedPos))
               {
                  std::cout<<"I am in check"<<std::endl;
                  break;
               }

               if (turn == 0)
               {
                  whitePieces.erase(selectedPos);
                  whitePieces.insert(pos);
               }

               else
               {
                  blackPieces.erase(selectedPos);
                  blackPieces.insert(pos);
               }

               removeCastle(pos,selectedPos);

               selected = 0;
               newMessage.highlightSpace.pos = selectedPos;
               newMessage.highlightSpace.color = 0;
               
               {
                  boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

                  sharedData.clientBuffer.push_front(newMessage);

                  for (auto iter = move.begin(); iter != move.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = hit.begin(); iter != hit.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  for (auto iter = castle.begin(); iter != castle.end(); iter++)
                  {
                     newMessage.highlightSpace.pos = *iter;
                     newMessage.highlightSpace.color = 0;

                     sharedData.clientBuffer.push_front(newMessage);
                  }

                  newMessage.id = MOVE_PIECE;
                  newMessage.movePiece.pos = pos;
                  newMessage.movePiece.oldPos = selectedPos;

                  sharedData.clientBuffer.push_front(newMessage);

                  if (pos.x == 2) //left castle
                  { 
                     newMessage.movePiece.pos.x = 3;
                     newMessage.movePiece.oldPos.x = 0;
                  }

                  else
                  {
                     newMessage.movePiece.pos.x = 5;
                     newMessage.movePiece.oldPos.x = 7;
                  }
                     sharedData.clientBuffer.push_front(newMessage);
               }

               move.clear();
               hit.clear();
               castle.clear();

               board[pos.y][pos.x] = board[selectedPos.y][selectedPos.x];
               board[selectedPos.y][selectedPos.x] = 0;

               if (pos.x == 2) //left
               {
                  pos.x = 3;
                  selectedPos.x = 0;

                  board[pos.y][3] = board[selectedPos.y][0];
                  board[selectedPos.y][0] = 0;
               }
               
               else 
               {
                  pos.x = 5;
                  selectedPos.x = 7;

                  board[pos.y][5] = board[selectedPos.y][7];
                  board[selectedPos.y][7] = 0;
               }

               if (turn == 0)
               {
                  whitePieces.erase(selectedPos);
                  whitePieces.insert(pos);
               }

               else
               {
                  blackPieces.erase(selectedPos);
                  blackPieces.insert(pos);
               }

               turn = !turn;


               if (checkCheckmate())
               {
                  std::cout<<"I win"<<std::endl;
               }
            }



         }

         else if (board[pos.y][pos.x] && board[pos.y][pos.x]/8 == turn)
         {
            selected = 1;
            selectedPos = pos;
            newMessage.highlightSpace.pos = pos;
            newMessage.highlightSpace.color = 1;

            generateMoves(pos);

            {
               boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

               sharedData.clientBuffer.push_front(newMessage);

               for (auto iter = move.begin(); iter != move.end(); iter++)
               {
                  newMessage.highlightSpace.pos = *iter;
                  newMessage.highlightSpace.color = 2;

                  sharedData.clientBuffer.push_front(newMessage);
               }

               for (auto iter = hit.begin(); iter != hit.end(); iter++)
               {
                  newMessage.highlightSpace.pos = *iter;
                  newMessage.highlightSpace.color = 3;

                  sharedData.clientBuffer.push_front(newMessage);
               }

               for (auto iter = castle.begin(); iter != castle.end(); iter++)
               {
                  newMessage.highlightSpace.pos = *iter;
                  newMessage.highlightSpace.color = 4;

                  sharedData.clientBuffer.push_front(newMessage);
               }

            }


         }



         break;
      }

      case QUIT_MESSAGE:
         std::cout<<"It told me to quit"<<std::endl;
         return;

      default:
         std::cout<<"And I do not know what it was"<<std::endl;

      }
   }
}




