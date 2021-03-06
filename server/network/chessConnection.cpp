#include "chessConnection.h"

#include "messages.h"
#include "netMessages.h"
#include "myDataBase.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include <iostream>

#include <boost/thread.hpp>

#include "../game/chessGame.h"

void createGame(boost::shared_ptr<t_sharedGame> sharedGame)
{
   t_chessGame chessGame(*sharedGame);

   chessGame.run();
}

using namespace boost::asio::ip;

t_chessConnection::t_chessConnection(boost::shared_ptr<t_connectionData> theSharedData, const boost::shared_ptr<tcp::socket> &theSocket, tcp::endpoint theEnd) : connectionData(theSharedData), socket(theSocket), end(theEnd), color(0), playing(0)
{
   boost::asio::socket_base::keep_alive option(true);
   socket->set_option(option);

   std::cout<<"Connection made !!"<<std::endl;
}

void t_chessConnection::run()
{

   for (;;)
   {
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      boost::system::error_code ecf;

      for (;;)
      {
         t_message message;
         {
            boost::unique_lock<boost::mutex> lock(connectionData->connMutex);

            if (connectionData->connBuffer.empty())
            {
               break;
            }

            message = connectionData->connBuffer.back();
            connectionData->connBuffer.pop_back();
         }
         std::cout<<"I have recieved a message"<<std::endl;

         switch (message.id)
         {
         case JOIN_SERVER:
         {
            std::cout<<"The name was: "<<message.joinServer.name<<" at a port of: "<<message.joinServer.address<<std::endl;
            break;
         }

         case PLAY_REQUEST:
         {
            std::cout<<"Someone wants to play with me"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_PLAY_REQUEST;

            strcpy(newNetMessage.netPlayRequest.name,message.playRequest.name);

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case PLAY_RESPONSE:
         {
            std::cout<<"I have recieved the response from MY request"<<std::endl;
            std::cout<<message.playResponse.name<<" responded with "<<message.playResponse.response<<std::endl;

            t_netMessage newNetMessage;

            if (message.playResponse.response == 0)
            {
               newNetMessage.id = NET_PLAY_REJECTED;
               boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

               if (ecf)
               {
                  needToQuit(ecf);
                  return;
               }

               break;
            }

            newNetMessage.id = NET_PLAY_ACCEPTED;
            newNetMessage.netPlayAccepted.side = !message.playResponse.side;
            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            int found = 0;
            boost::shared_ptr<t_connectionData> tempConnData;

            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               BOOST_FOREACH(auto myDataTemp,  connectionData->myDataInfo->myDataBase)
               {
                  if (!myDataTemp.second)
                  {
                     continue;
                  }

                  if (myDataTemp.second->name == std::string(message.playResponse.name))
                  {
                     tempConnData = myDataTemp.second->connectionData.lock();
                     found = 1;
                     break;
                  }
               }

            }

            if (found == 0)
            {
               exit(1);

               return;
            }


            color = !message.playResponse.side;
            playing = 1;

            sharedGame = boost::make_shared<t_sharedGame>(tempConnData,connectionData);

            boost::thread gameThread(boost::bind(createGame,sharedGame));

            t_message newMessage;
            newMessage.id = PASS_GAME;
            newMessage.gamePass.sharedGame = &sharedGame;
            newMessage.gamePass.side = message.playResponse.side;

            {
               boost::unique_lock<boost::mutex> lock(tempConnData->connMutex);

               tempConnData->connBuffer.push_front(newMessage);
            }

            break;
         }

         case PASS_GAME:
         {
            color = message.gamePass.side;
            playing = 1;
            sharedGame = *message.gamePass.sharedGame;
            break;
         }

         case CHANGE_ICON:
         {
            std::cout<<"piece changed"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_CHANGE_ICON;
            newNetMessage.netChangeIcon.pawnPos = message.changeIcon.pawnPos;
            newNetMessage.netChangeIcon.type = message.changeIcon.type;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case SHOW_PAWN_PROMOTE:
         {
            std::cout<<"piece changed"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_SHOW_PAWN_PROMOTE;
            newNetMessage.netShowPawnPromote.color = message.showPawnPromote.color;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case HIGHLIGHT_SPACE:
         {
            std::cout<<"piece highlighted"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_HIGHLIGHT_SPACE;
            newNetMessage.netHighlightSpace.pos = message.highlightSpace.pos;
            newNetMessage.netHighlightSpace.color = message.highlightSpace.color;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case MOVE_PIECE:
         {
            std::cout<<"piece moved"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_MOVE_PIECE;
            newNetMessage.netMovePiece.pos = message.movePiece.pos;
            newNetMessage.netMovePiece.oldPos = message.movePiece.oldPos;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case CAPTURE_PIECE:
         {
            std::cout<<"Piece captured"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_CAPTURE_PIECE;
            newNetMessage.netMovePiece.pos = message.movePiece.pos;
            newNetMessage.netMovePiece.oldPos = message.movePiece.oldPos;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);


            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case CHECK_MATE:
         {
            std::cout<<"Check Mate"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_CHECK_MATE;
            newNetMessage.netCheckMate.winner = message.checkMate.winner;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case IN_CHECK:
         {
            std::cout<<"In check"<<std::endl;

            t_netMessage newNetMessage;
            newNetMessage.id = NET_IN_CHECK;
            newNetMessage.netInCheck.attackingPiece = message.inCheck.attackingPiece;;

            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
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

      for (;;)
      {


         boost::system::error_code ec;

         if (!socket->available(ec))
         {
            if (!ec)
            {
               break;
            }

            else
            {
               needToQuit(ec);
               break;
            }
         }

         std::cout<<"About to read the the connection that \"should\" be there...   "<<ec<<std::endl;

         t_netMessage netMessage;

         socket->read_some(boost::asio::buffer(&netMessage,sizeof(netMessage)));

         switch (netMessage.id)
         {
         case NET_JOIN_SERVER:
         {
            std::cout<<"The name was: "<<netMessage.netJoinServer.name<<std::endl;

            int found = 0;

            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               BOOST_FOREACH(auto myDataTemp,  connectionData->myDataInfo->myDataBase)
               {
                  if (!myDataTemp.second)
                  {
                     continue;
                  }

                  if (myDataTemp.second->name == std::string(netMessage.netJoinServer.name))
                  {
                     found = 1;
                     break;
                  }
               }

            }

            if (found == 1)
            {
               t_netMessage newNetMessage;

               newNetMessage.id = NET_CONNECTION_BAD_NAME;
               boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

               if (ecf)
               {
                  needToQuit(ecf);
                  return;
               }

               std::cout<<"But the name was bad..."<<std::endl;
               return;
            }


            myData = boost::make_shared<t_myData>();

            myData->name = netMessage.netJoinServer.name;
            myData->wins = 0;
            myData->losses = 0;
            myData->status = NOTHING;
            myData->connectionData = connectionData;

            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               connectionData->myDataInfo->myDataBase.insert(std::make_pair(end,myData));
            }

            t_netMessage newNetMessage;

            newNetMessage.id = NET_CONNECTION_SUCCESS;
            boost::asio::write(*socket,boost::asio::buffer(&newNetMessage,sizeof(newNetMessage)),boost::asio::transfer_all(),ecf);

            if (ecf)
            {
               needToQuit(ecf);
               return;
            }

            break;
         }

         case NET_REFRESH_CONNECTION:
         {
            std::cout<<"Now I have to send the stupid data over the internet, stupid net"<<std::endl;

            {
               boost::shared_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               netMessage.netRefreshConnection.numOfPackets = connectionData->myDataInfo->myDataBase.size();

               boost::asio::write(*socket,boost::asio::buffer(&netMessage,sizeof(netMessage)),boost::asio::transfer_all(),ecf);

               if (ecf)
               {
                  needToQuit(ecf);
                  return;
               }

               t_dataPacket dataPacket;

               for (auto iter = connectionData->myDataInfo->myDataBase.begin(); iter != connectionData->myDataInfo->myDataBase.end(); iter++)
               {
                  strcpy(dataPacket.name,iter->second->name.c_str());
                  dataPacket.wins = iter->second->wins;
                  dataPacket.losses = iter->second->losses;
                  dataPacket.status = iter->second->status;

                  boost::asio::write(*socket,boost::asio::buffer(&dataPacket,sizeof(dataPacket)),boost::asio::transfer_all(),ecf);


                  if (ecf)
                  {
                     needToQuit(ecf);
                     return;
                  }

               }

            }

            break;
         }

         case NET_BOARD_CLICKED:
         {
            std::cout<<"I have recieved news of a board clicked"<<std::endl;

            t_message newMessage;
            newMessage.id = BOARD_CLICKED;
            newMessage.boardClicked.pos = netMessage.netBoardClicked.pos;

            sharedGame->pushToGame(newMessage,color);
            break;
         }

         case NET_RECIEVE_PAWN_PROMOTE:
         {
            std::cout<<"The person has chosen the promotion"<<std::endl;

            t_message newMessage;
            newMessage.id = RECIEVE_PAWN_PROMOTE;
            newMessage.recievePawnPromote.type = netMessage.netRecievePawnPromote.type;

            sharedGame->pushToGame(newMessage,color);
            break;
         }

         case NET_WANT_TO_PLAY_WITH:
         {
            std::cout<<"This network is not a loner and wants to play with someone"<<std::endl;


            int found = 0;
            boost::shared_ptr<t_connectionData> tempConnData;

            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               BOOST_FOREACH(auto myDataTemp,  connectionData->myDataInfo->myDataBase)
               {
                  if (!myDataTemp.second)
                  {
                     continue;
                  }

                  if (myDataTemp.second->name == std::string(netMessage.netWantToPlayWith.name))
                  {
                     tempConnData = myDataTemp.second->connectionData.lock();
                     found = 1;
                     break;
                  }
               }

            }

            if (found == 0)
            {
               exit(1);

               return;
            }

            t_message newMessage;

            newMessage.id = PLAY_REQUEST;
            strcpy(newMessage.playRequest.name,connectionData->myDataInfo->myDataBase[end]->name.c_str());

            {
               boost::unique_lock<boost::mutex> lock(tempConnData->connMutex);

               tempConnData->connBuffer.push_front(newMessage);
            }

            break;
         }

         case NET_PLAY_RESPONSE:
         {
            std::cout<<"Telling the requesti about the response that I got "<<std::endl;

            int found = 0;
            boost::shared_ptr<t_connectionData> tempConnData;

            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);

               BOOST_FOREACH(auto myDataTemp,  connectionData->myDataInfo->myDataBase)
               {
                  if (!myDataTemp.second)
                  {
                     continue;
                  }

                  if (myDataTemp.second->name == std::string(netMessage.netPlayResponse.name))
                  {
                     tempConnData = myDataTemp.second->connectionData.lock();
                     found = 1;
                     break;
                  }
               }

            }

            if (found == 0)
            {
               exit(1);

               return;
            }

            t_message newMessage;

            newMessage.id = PLAY_RESPONSE;
            newMessage.playResponse.response =  netMessage.netPlayResponse.response;
            newMessage.playResponse.side =  netMessage.netPlayResponse.side;
            strcpy(newMessage.playResponse.name,connectionData->myDataInfo->myDataBase[end]->name.c_str());

            {
               boost::unique_lock<boost::mutex> lock(tempConnData->connMutex);

               tempConnData->connBuffer.push_front(newMessage);
            }

            break;
         }


         case NET_QUIT_MESSAGE:
         {
            std::cout<<"It told me to quit"<<std::endl;
            {
               boost::unique_lock<boost::shared_mutex> lock(connectionData->myDataInfo->myDataMutex);
               connectionData->myDataInfo->myDataBase.erase(end);
            }
         }
         return;


         default:
            std::cout<<"And I do not know what it was"<<std::endl;
         }
      }
   }

   std::cout<<"The thread started, then quit"<<std::endl;


}

void t_chessConnection::needToQuit(boost::system::error_code ecf)
{
   std::cout<<"The send failed with "<<ecf.message()<<" and "<<ecf<<std::endl;

}
