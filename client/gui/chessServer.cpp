#include "chessGui.h"

#include "messages.h"

#include <boost/bind.hpp>

#include <SFML/Graphics.hpp>
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/CEGUIOpenGLRenderer.h>

#include <iostream>

#include <cstring>

bool refreshConnect(t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   t_message message;

   message.id = WANT_REFRESH_CONNECTION;

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }

   sharedData.gameCondition.notify_one();

   return true;
}

bool selectedPlay(CEGUI::MultiColumnList *listOfStuff, t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   t_message message;

   message.id = WANT_TO_PLAY_WITH;

   strcpy(message.wantToPlayWith.name,listOfStuff->getFirstSelectedItem()->getText().c_str());

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }

   sharedData.gameCondition.notify_one();

   std::cout<<"The play button was pressed"<<std::endl;
   std::cout<<"The first data was "<<listOfStuff->getFirstSelectedItem()->getText()<<std::endl;

   return true;
}

bool openServer(CEGUI::FrameWindow *server,t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   refreshConnect(sharedData,e);
   server->show();
   return true;
}

bool closeServer(CEGUI::FrameWindow *server, const CEGUI::EventArgs &e)
{
   server->hide();
   return true;
}

bool openConnect(CEGUI::FrameWindow *connect, const CEGUI::EventArgs &e)
{
   connect->show();
   connect->setModalState(true);
   return true;
}

bool closeConnect(CEGUI::FrameWindow *connect, const CEGUI::EventArgs &e)
{
   connect->hide();
   connect->setModalState(false);
   return true;
}

bool openMessage(CEGUI::FrameWindow *connect, const CEGUI::EventArgs &e)
{
   connect->show();
   connect->setModalState(true);
   return true;
}

bool closeMessage(CEGUI::FrameWindow *connect, const CEGUI::EventArgs &e)
{
   connect->hide();
   connect->setModalState(false);
   return true;
}


bool connectToServer(CEGUI::FrameWindow *connect, CEGUI::Editbox *name, CEGUI::Editbox *ip, t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   t_message message;

   message.id = JOIN_SERVER;

   strcpy(message.joinServer.name,name->getText().c_str());
   std::cout<<message.joinServer.name<<std::endl;

   strcpy(message.joinServer.address,ip->getText().c_str());
   std::cout<<message.joinServer.address<<std::endl;

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }

   sharedData.gameCondition.notify_one();

   connect->hide();
   connect->setModalState(false);

   return true;
}

bool disconnectFromServer(t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   t_message message;
   message.id = DISCONNECT_MESSAGE;

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }
   sharedData.gameCondition.notify_one();
   return true;
}

bool acceptRequest(CEGUI::FrameWindow *request,t_sharedData &sharedData, const CEGUI::EventArgs &e)
{
   t_message message;
   message.id = PLAY_RESPONSE;

   message.playResponse.response = 1;
   strcpy(message.playResponse.name,request->getUserString("ResponseName").c_str());

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }
   sharedData.gameCondition.notify_one();

   request->hide();
   request->setModalState(false);
   return true;
}

bool refuseRequest(CEGUI::FrameWindow *request, t_sharedData &sharedData,const CEGUI::EventArgs &e)
{
   t_message message;
   message.id = PLAY_RESPONSE;

   message.playResponse.response = 0;
   strcpy(message.playResponse.name,request->getUserString("ResponseName").c_str());

   {
      boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

      sharedData.gameBuffer.push_front(message);
   }
   sharedData.gameCondition.notify_one();

   request->hide();
   request->setModalState(false);
   return true;
}

void t_chessGui::initServer()
{
   CEGUI::FrameWindow *server = static_cast<CEGUI::FrameWindow *>(wmgr->loadWindowLayout("server.layout"));
   myRoot->addChildWindow(server);
   server->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,CEGUI::Event::Subscriber(boost::bind(closeServer,server,_1)));

   CEGUI::MenuItem *serverItem = static_cast<CEGUI::MenuItem *>(wmgr->getWindow("Root/FrameWindow/Menubar/File/New"));
   serverItem->subscribeEvent(CEGUI::MenuItem::EventClicked,CEGUI::Event::Subscriber(boost::bind(openServer,server,boost::ref(sharedData),_1)));

   CEGUI::PushButton *refreshButton = static_cast<CEGUI::PushButton *>(wmgr->getWindow("Lols5"));
   refreshButton->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(refreshConnect,boost::ref(sharedData),_1)));

   CEGUI::PushButton *disconnectButton = static_cast<CEGUI::PushButton *>(wmgr->getWindow("Lols6"));
   disconnectButton->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(disconnectFromServer,boost::ref(sharedData),_1)));


   CEGUI::MultiColumnList *testing = static_cast<CEGUI::MultiColumnList *>(wmgr->getWindow("Lols3"));
   testing->addColumn("Names",0,CEGUI::UDim(.25,0));
   testing->addColumn("Action",1,CEGUI::UDim(.25,0));
   testing->addColumn("Wins",2,CEGUI::UDim(.25,0));
   testing->addColumn("Losses",3,CEGUI::UDim(.25,0));

   testing->addRow();
   testing->addRow();

   testing->setItem(new CEGUI::ListboxTextItem("What, wow,"),0u,0u);

   CEGUI::PushButton *playButton = static_cast<CEGUI::PushButton *>(wmgr->getWindow("Lols4"));
   playButton->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(selectedPlay,testing,boost::ref(sharedData),_1)));

   server->hide();
}

void t_chessGui::initConnect()
{
   CEGUI::FrameWindow *connect = static_cast<CEGUI::FrameWindow *>(wmgr->loadWindowLayout("connect.layout"));

   myRoot->addChildWindow(connect);
   connect->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,CEGUI::Event::Subscriber(boost::bind(closeConnect,connect,_1)));

   CEGUI::MenuItem *connectItem = static_cast<CEGUI::MenuItem *>(wmgr->getWindow("Root/FrameWindow/Menubar/File/Open"));
   connectItem->subscribeEvent(CEGUI::MenuItem::EventClicked,CEGUI::Event::Subscriber(boost::bind(openConnect,connect,_1)));

   CEGUI::MenuItem *newConnectItem = static_cast<CEGUI::MenuItem *>(wmgr->getWindow("Lols2"));
   newConnectItem->subscribeEvent(CEGUI::MenuItem::EventClicked,CEGUI::Event::Subscriber(boost::bind(openConnect,connect,_1)));



   CEGUI::PushButton *cancelConnect = static_cast<CEGUI::PushButton *>(wmgr->getWindow("1Lols6"));
   cancelConnect->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(closeConnect,connect,_1)));


   CEGUI::Editbox *name = static_cast<CEGUI::Editbox *>(wmgr->getWindow("1Lols2"));
   CEGUI::Editbox *ip = static_cast<CEGUI::Editbox *>(wmgr->getWindow("1Lols7"));


   CEGUI::PushButton *startConnection = static_cast<CEGUI::PushButton *>(wmgr->getWindow("1Lols4"));
   startConnection->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(connectToServer,connect,name,ip,boost::ref(sharedData),_1)));

   connect->hide();
}

void t_chessGui::refreshServer(t_message message)
{

   CEGUI::MultiColumnList *testing = static_cast<CEGUI::MultiColumnList *>(wmgr->getWindow("Lols3"));
   testing->resetList();

   t_dataPacket data;
   char temp[40];

   for (unsigned int i = 0; i< message.dataPackets.size(); i++)
   {
      std::cout<<"I am adding the row: "<<i<<std::endl;
      std::cout<<data.name<<" "<<data.status<<" "<<data.wins<<" "<<std::endl;
      unsigned int b = testing->addRow();

      data = message.dataPackets[i];
      testing->setItem(new MyListItem(data.name),0u,b);

      snprintf(temp,sizeof(temp),"%d",data.status);
      testing->setItem(new MyListItem(temp),1u,b);

      snprintf(temp,sizeof(temp),"%d",data.wins);
      testing->setItem(new MyListItem(temp),2u,b);

      snprintf(temp,sizeof(temp),"%d",data.losses);
      testing->setItem(new MyListItem(temp),3u,b);
   }

   CEGUI::Window *text = static_cast<CEGUI::Window *>(wmgr->getWindow("Lols"));
   snprintf(temp,sizeof(temp),"You are connected to: %s",message.refreshConnection.server);
   text->setText(temp);
}

void t_chessGui::initMessage()
{
   CEGUI::FrameWindow *message = static_cast<CEGUI::FrameWindow *>(wmgr->loadWindowLayout("message.layout"));

   myRoot->addChildWindow(message);
   message->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,CEGUI::Event::Subscriber(boost::bind(closeMessage,message,_1)));

   CEGUI::MenuItem *messageItem = static_cast<CEGUI::MenuItem *>(wmgr->getWindow("Root/FrameWindow/Menubar/File/Close"));
   messageItem->subscribeEvent(CEGUI::MenuItem::EventClicked,CEGUI::Event::Subscriber(boost::bind(openMessage,message,_1)));

   CEGUI::PushButton *okMessage = static_cast<CEGUI::PushButton *>(wmgr->getWindow("messageWindow/button"));
   okMessage->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(closeMessage,message,_1)));
   message->hide();
}

void t_chessGui::showMessage(std::string textStuff)
{
   CEGUI::FrameWindow *message = static_cast<CEGUI::FrameWindow *>(wmgr->getWindow("messageWindow"));

   CEGUI::Window *text = static_cast<CEGUI::Window *>(wmgr->getWindow("messageWindow/text"));
   //snprintf(temp,sizeof(temp),"You are connected to: %s",message.refreshConnection.server);
   text->setText(textStuff.c_str());

   message->show();
   message->setModalState(true);
}


void t_chessGui::initRequest()
{
   CEGUI::FrameWindow *message = static_cast<CEGUI::FrameWindow *>(wmgr->loadWindowLayout("request.layout"));

   myRoot->addChildWindow(message);
   message->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,CEGUI::Event::Subscriber(boost::bind(closeMessage,message,_1)));

   CEGUI::MenuItem *messageItem = static_cast<CEGUI::MenuItem *>(wmgr->getWindow("Root/FrameWindow/Menubar/File/Save"));
   messageItem->subscribeEvent(CEGUI::MenuItem::EventClicked,CEGUI::Event::Subscriber(boost::bind(openMessage,message,_1)));

   CEGUI::PushButton *okMessage = static_cast<CEGUI::PushButton *>(wmgr->getWindow("requestWindow/yes"));
   okMessage->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(acceptRequest,message,boost::ref(sharedData),_1)));

   CEGUI::PushButton *noMessage = static_cast<CEGUI::PushButton *>(wmgr->getWindow("requestWindow/no"));
   noMessage->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(boost::bind(refuseRequest,message,boost::ref(sharedData),_1)));
   message->hide();
}

void t_chessGui::showRequest(std::string textStuff)
{
   CEGUI::FrameWindow *message = static_cast<CEGUI::FrameWindow *>(wmgr->getWindow("requestWindow"));

   if (message->isVisible())
   {
      t_message newMessage;
      newMessage.id = PLAY_RESPONSE;

      newMessage.playResponse.response = 0;
      strcpy(newMessage.playResponse.name,textStuff.c_str());

      {
         boost::unique_lock<boost::mutex> lock(sharedData.gameMutex);

         sharedData.gameBuffer.push_front(newMessage);
      }
      sharedData.gameCondition.notify_one();
      return;
   }

   CEGUI::Window *text = static_cast<CEGUI::Window *>(wmgr->getWindow("requestWindow/text"));

   char temp[100];
   snprintf(temp,sizeof(temp),"Do you want to play with %s?",textStuff.c_str());
   text->setText(temp);

   message->setUserString("ResponseName",textStuff);

   message->show();
   message->setModalState(true);
}