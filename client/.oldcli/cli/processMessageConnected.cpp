#include <iostream>

#include <boost/thread.hpp>

#include <cstdio>
#include <cstdlib>

#include "chessCli.h"
#include "messages.h"

bool t_chessCli::processMessageConnected(const t_message &message)
{
   switch (message.id)
   {

   case BOARD_CLICKED:
   {
      std::cout<<"Telling net I clicked"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }
      
      break;
   }

   case HIGHLIGHT_SPACE:
   {
      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }

   case MOVE_PIECE:
   {
      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }
   case CAPTURE_PIECE:
   {
      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }
   case JOIN_SERVER:
   {
      std::cout<<"Telling net to join the server"<<std::endl;

      t_message newMessage;
      newMessage.id = DISCONNECT_MESSAGE;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(newMessage);
         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }
      connected = 0;

      break;
   }

   case WANT_TO_PLAY_WITH:
   {
      std::cout<<"Telling net I want to play"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }
      
      break;
   }

   case PLAY_REQUEST:
   {
      std::cout<<"Someone is asking me to play"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }

   case PLAY_REJECTED:
   {
      std::cout<<"play rejected"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }

   case PLAY_ACCEPTED:
   {
      std::cout<<"play accepted"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }

   case PLAY_RESPONSE:
   {
      std::cout<<"I have responded to their asking"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }

      break;
   }

   case DISCONNECT_MESSAGE:
   {
      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }

      connected = 0;

      break;
   }

   case WANT_REFRESH_CONNECTION:
   {
      std::cout<<"Telling net to refresh my data"<<std::endl;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }

      break;
   }

   case REFRESH_CONNECTION:
   {
      std::cout<<"Now to shuttle this refresh data to gui"<<std::endl;

      for (auto iter = message.dataPackets.begin(); iter != message.dataPackets.end(); iter++)
      {
         std::cout<<iter->name<<std::endl;
      }

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }

      break;
   }

   case CONNECTION_FAILED:
   {
      std::cout<<"The connection has quit/failed"<<std::endl;
      connected = 0;

      {
         boost::unique_lock<boost::mutex> lock(sharedData.clientMutex);

         sharedData.clientBuffer.push_front(message);
         sharedData.clientCondition.notify_one();
      }
      break;
   }

   case QUIT_MESSAGE:
   {
      std::cout<<"It told me to quit"<<std::endl;
      {
         boost::unique_lock<boost::mutex> lock(sharedData.netMutex);

         sharedData.netBuffer.push_front(message);
         sharedData.netCondition.notify_one();
      }
   }
   return 1;

   default:
      std::cout<<"And I do not know what it was"<<std::endl;

   }

   return 0;
}
