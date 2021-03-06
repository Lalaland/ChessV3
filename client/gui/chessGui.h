#ifndef CHESS_GUI_H_INCLUDED
#define CHESS_GUI_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <gtkmm.h>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>

#include "messages.h"

class ModelColumns : public Gtk::TreeModelColumnRecord
{
public:

   ModelColumns()
   {
      add(m_Name);
      add(m_Status);
      add(m_Wins);
      add(m_Losses);
   }

   Gtk::TreeModelColumn<Glib::ustring> m_Name;
   Gtk::TreeModelColumn<int> m_Status;
   Gtk::TreeModelColumn<int> m_Wins;
   Gtk::TreeModelColumn<int> m_Losses;

};

class PromoteColumns : public Gtk::TreeModelColumnRecord
{
public:

   PromoteColumns()
   {
      add(m_Picture);
      add(m_Name);
      add(m_Type);
   }

   Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_Picture;
   Gtk::TreeModelColumn<Glib::ustring> m_Name;
   Gtk::TreeModelColumn<int> m_Type;

};

class t_sharedData;

class t_chessGui : boost::noncopyable
{
public:
   t_chessGui(t_sharedData &theSharedData);

   void run();

private:
   void initGtkmm();
   void initSfml();
   void initPromoteLists();
   void initAList(Glib::RefPtr<Gtk::ListStore> ,bool);
   void reset();
   void set();

   void loadImages();
   void loadSprites();

   bool checkBuffer();

   void refreshServer(t_message message);
   void showMessage(std::string message);
   void showRequest(std::string message);
   void resetWarningConnected();
   void resetWarningSingle();
   int showPawnPromote(bool color);

   sf::RenderWindow App;

   sf::Image images[12];

   sf::Sprite sprites[32];

   sf::Shape BlackBox;
   sf::Shape RedBox;
   sf::Shape BlueBox;
   sf::Shape BrownBox;
   sf::Shape PurpleBox;
   sf::Shape PinkBox;
   sf::Shape GreenBox;

   Glib::RefPtr<Gtk::Builder> builder;
   Glib::RefPtr<Gtk::ListStore> list;
   Glib::RefPtr<Gtk::ListStore> blackList;
   Glib::RefPtr<Gtk::ListStore> whiteList;
   ModelColumns columns;
   PromoteColumns promoteColumns;

   boost::scoped_ptr<Gtk::Window> mainWindow;
   boost::scoped_ptr<Gtk::Window> serverWindow;
   boost::scoped_ptr<Gtk::Dialog> connectDialog;
   boost::scoped_ptr<Gtk::Dialog> singleSettings;
   boost::scoped_ptr<Gtk::Dialog> newGameDialog;
   boost::scoped_ptr<Gtk::Dialog> pawnPromotionDialog;
   boost::scoped_ptr<Gtk::Dialog> chooseSideDialog;

   Gtk::DrawingArea *myArea;
   Gtk::Entry *nameEntry;
   Gtk::Entry *serverEntry;
   Gtk::Label *label;
   Gtk::TreeView *view;
   Gtk::RadioButton *Single, *Two;
   Gtk::RadioButton *customUci;
   Gtk::FileChooserButton *uciButton;
   Gtk::ComboBox *promoteSelect;
   Gtk::RadioButton *whiteButton, *whiteSingle;

   

   t_sharedData &sharedData;

   int boardColors[64];

   int boardPieces[8][8];

   const int width;
   const int height;

   int newWidth;
   int newHeight;

   bool showingCheck;
   t_myVector2 checkPos;

   //Gui Callbacks
   void customUciToggled();
   void openSingleSettings();
   void openNewGame();
   void newButton();
   bool drawBoard(GdkEventExpose *event);
   bool mouseButtonPressedEvent(GdkEventButton* event);
   void sizeAllocated(Gtk::Allocation &);
   void disconnectFromServer();
   void selectedPlay();
   void realizeFunc();

   void openServer();
   void openConnect();
   void refreshConnect();
   void connectToServer();

};

#endif
