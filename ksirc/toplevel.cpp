/**********************************************************************

 TopLevel IRC Channel/query Window

 $$Id: toplevel.cpp 107242 2001-07-23 01:20:18Z pfeiffer $$

 This is the main window with with the user interacts.  It handles
 both normal channel converstations and private conversations.

 2 classes are defined, the UserControlMenu and KSircToplevel.  The
 user control menu is used as alist of user defineable menus used by
 KSircToplevel.

 KSircTopLevel:

 Signals:

 outputLine(QString &):
 output_toplevel(QString):

 closing(KSircTopLevel *, char *):

 changeChannel(QString old, QString new):

 currentWindow(KSircTopLevel *):

 changeSize():

 Slots:



 *********************************************************************/

#include "toplevel.h"
#include "iocontroller.h"
#include "NewWindowDialog.h"
#include "control_message.h"
#include "config.h"
#include "ssfeprompt.h"
#include "displayMgr.h"
#include "KSTicker/ksticker.h"
#include "ahistlineedit.h"
#include "alistbox.h"

#include <stdlib.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include <qclipboard.h>
#include <qtooltip.h>
#include <qsplitter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kio/netaccess.h>
#include <ktempfile.h>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>

#include <kdebug.h>

extern KConfig *kConfig;
extern KApplication *kApp;
extern DisplayMgr *displayMgr;
//QPopupMenu *KSircTopLevel::user_controls = 0L;
QList<UserControlMenu> *KSircTopLevel::user_menu = 0L;
QPixmap *KSircTopLevel::pix_info = 0L;
QPixmap *KSircTopLevel::pix_star = 0L;
QPixmap *KSircTopLevel::pix_bball = 0L;
QPixmap *KSircTopLevel::pix_greenp = 0L;
QPixmap *KSircTopLevel::pix_bluep = 0L;
QPixmap *KSircTopLevel::pix_madsmile = 0L;

#define KSB_MAIN_LINEE 10
#define KSB_MAIN_LAG 20

void
KSircTopLevel::initColors()
{
  if(kSircConfig->colour_background == 0){
    kConfig->setGroup("Colours");
    kSircConfig->colour_background = new QColor(kConfig->readColorEntry("Background", new QColor(colorGroup().mid())));
  }

  ircListItem::flushCache();
  QColorGroup cg_mainw = kapp->palette().active();
  cg_mainw.setColor(QColorGroup::Base, *kSircConfig->colour_background);
  cg_mainw.setColor(QColorGroup::Text, *kSircConfig->colour_text);
  mainw->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  nicks->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
  linee->setPalette(QPalette(cg_mainw,cg_mainw, cg_mainw));
}

KSircTopLevel::KSircTopLevel(KSircProcess *_proc, const char *cname, const char * name)
    : KMainWindow(0, name, 0 /* no WDestructiveClose! */),
      KSircMessageReceiver(_proc)

{
    // prevent us from being quitted when closing a channel-window. Only
    // closing the servercontroller shall quit.
    // KMainWindow will deref() us in closeEvent
    kapp->ref();

  /*
   * Make sure we tell others when we are destroyed
   */
  connect(this, SIGNAL(destroyed()),
          this, SLOT(iamDestroyed()));

  connect(kapp, SIGNAL(kdisplayPaletteChanged()),
          this, SLOT(initColors()));

  proc = _proc;

  QString kstl_name = QString(QObject::name()) + "_" + "toplevel";
  setName(kstl_name);

  channel_name = qstrdup(cname);
  if(channel_name){
  //    QString s = channel_name;
  //    int pos2 = s.find(' ', 0);
  //    if(pos2 > 0)
  //      channel_name = qstrdup(s.mid(0, pos2).data());

    setCaption(channel_name);
    caption = channel_name;
  }
  else
    caption = "";

  Buffer = FALSE;

  have_focus = 0;
  ticker = 0; // Set the ticker to NULL while it doesn't exist.
  tab_pressed = 0; // Tab (nick completion not pressed yet)
  tab_start = -1;
  tab_end = -1;

  KickWinOpen = false;
  current_size = size();


  file = new KAPopupMenu(0x0, QString(QObject::name()) + "_popup_file");
  file->setCheckable(true);

  file->insertItem(i18n("&New..."), this, SLOT(newWindow()), CTRL + Key_N);
  file->insertItem(i18n("&Ticker Mode"), this, SLOT(showTicker()), CTRL + Key_T);
  //  file->insertItem("&Root Window Mode", this, SLOT(toggleRootWindow()), CTRL + Key_Z);
  file->insertSeparator();
  file->insertItem(i18n("&Save to logfile..."), this, SLOT(saveCurrLog()), CTRL + Key_S);

  tsitem = file->insertItem(i18n("Time St&amp"), this, SLOT(toggleTimestamp()), CTRL + Key_A);
  file->setItemChecked(tsitem, kSircConfig->timestamp);
  file->insertSeparator();

  file->insertItem(i18n("&Close"), this, SLOT(terminate()), CTRL + Key_W );

  kmenu = menuBar();
  kmenu->insertItem(i18n("&Channel"), file, 2, -1);
  kmenu->setAccel(Key_F, 2);

  edit = new KAPopupMenu();
  edit->insertItem(i18n("&Paste"), this, SLOT(pasteToWindow()), CTRL + Key_V);
  edit->insertItem(i18n("&Clear Window"), this, SLOT(clearWindow()), CTRL + Key_L);
  kmenu->insertItem(i18n("&Edit"), edit, -1, -1);

  ksb_main = new KStatusBar(this, QString(QObject::name()) + "_" + "KStatusBar");

  /*
   * Ok, let's look at the basic widget "layout"
   * Everything belongs to q QFrame F, this is use so we
   * can give the KApplication a single main client widget, which is needs.
   *
   * A QVbox and a QHbox is used to ctronl the 3 sub widget
   * The Modified QListBox is then added side by side with the User list box.
   * The SLE is then fit bello.
   */

  // kstInside does not setup fonts, etc, it simply handles sizing

  f = new kstInside(this, QString(QObject::name()) + "_" + "kstIFrame");
  setCentralWidget(f);  // Tell the KApplication what the main widget is.


  // get basic variable

  mainw = f->mainw;
  nicks = f->nicks;
  pan = f->pan;


  linee = new aHistLineEdit(ksb_main, "");

  initColors();

  ksb_main->addWidget(linee, mainw->width());
  ksb_main->insertItem("Lag: Wait", KSB_MAIN_LAG, true);

  // don't show the nick lists in a private chat or the default window
  if (isPrivateChat() || strncmp(channel_name, "!no_channel", 11) == 0)
      nicks->hide();
  else
      nicks->show();

  connect(mainw, SIGNAL(updateSize()),
          this, SIGNAL(changeSize()));
  connect(mainw, SIGNAL(pasteReq()),
          this, SLOT(pasteToWindow()));

  mainw->setFont(kSircConfig->defaultfont);
  nicks->setFont(kSircConfig->defaultfont);

  // setup line editor

  linee->setFocusPolicy(QWidget::StrongFocus);
  linee->setFont(kSircConfig->defaultfont);
  connect(linee, SIGNAL(gotFocus()),
          this, SLOT(gotFocus()));
  connect(linee, SIGNAL(lostFocus()),
          this, SLOT(lostFocus()));
  connect(linee, SIGNAL(pasteText()),
          this, SLOT(pasteToWindow()));
  connect(linee, SIGNAL(notTab()),
          this, SLOT(lineeNotTab()));

  connect(linee, SIGNAL(returnPressed()), // Connect return in sle to send
          this, SLOT(sirc_line_return()));// the line to dsirc

  linee->setFocus();  // Give SLE focus

  lines = 0;          // Set internal line counter to 0

  /*
   * Set generic run time variables
   *
   */

  opami = FALSE;
  continued_line = FALSE;
  on_root = FALSE;

  /*
   * Load basic pics and images
   * This should use on of the KDE finder commands
   */

  if(pix_info == 0){
    pix_info = new QPixmap(UserIcon("info"));
    pix_star = new QPixmap(UserIcon("star"));
    pix_bball = new QPixmap(UserIcon("blueball"));
    pix_greenp = new QPixmap(UserIcon("greenpin"));
    pix_bluep = new QPixmap(UserIcon("bluepin"));
    pix_madsmile = new QPixmap(UserIcon("madsmiley"));
  }
  KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());

  /*
   * Create our basic parser object
   */

  ChanParser = new ChannelParser(this);


  /*
   * Create the user Controls popup menu, and connect it with the
   * nicks list on the lefthand side (highlighted()).
   *
   */

  if(user_menu == 0)
    user_menu = UserControlMenu::parseKConfig();

  user_controls = new QPopupMenu();
  kmenu->insertItem(i18n("&Users"), user_controls);
  kmenu->insertItem(i18n("&Help"), helpMenu( QString::null, false ));

  connect(user_controls, SIGNAL(activated(int)),
          this, SLOT(UserParseMenu(int)));

  connect(nicks, SIGNAL(rightButtonPress(int)), this,
          SLOT(UserSelected(int)));
  connect(nicks, SIGNAL(selectedNick(const QString &)),
          this, SLOT(openQueryFromNick(const QString &)));



  UserUpdateMenu();  // Must call to update Popup.

  accel = new QAccel(this, "accel");

  accel->connectItem(accel->insertItem(SHIFT + Key_PageUp),
                     this,
                     SLOT(AccelScrollUpPage()));
  accel->connectItem(accel->insertItem(SHIFT + Key_PageDown),
                     this,
                     SLOT(AccelScrollDownPage()));

  /*
   * Pageup/dn
   * Added for stupid wheel mice
   */

  accel->connectItem(accel->insertItem(Key_PageUp),
                     this,
                     SLOT(AccelScrollUpPage()));
  accel->connectItem(accel->insertItem(Key_PageDown),
                     this,
                     SLOT(AccelScrollDownPage()));

  accel->connectItem(accel->insertItem(CTRL + Key_Enter),
                     this,
                     SLOT(AccelPriorMsgNick()));

  accel->connectItem(accel->insertItem(CTRL + SHIFT + Key_Enter),
                     this,
                     SLOT(AccelNextMsgNick()));

  accel->connectItem(accel->insertItem(CTRL + Key_Return),
                     this,
                     SLOT(AccelPriorMsgNick()));

  accel->connectItem(accel->insertItem(CTRL + SHIFT + Key_Return),
                     this,
                     SLOT(AccelNextMsgNick()));

  accel->connectItem(accel->insertItem(Key_Tab), // adds TAB accelerator
                     this,                         // connected to the main
                     SLOT(TabNickCompletion()));  // TabNickCompletion() slot
  accel->connectItem(accel->insertItem(CTRL + Key_N),
                     this, SLOT(newWindow()));
  accel->connectItem(accel->insertItem(CTRL + Key_T),
                     this, SLOT(showTicker()));
  accel->connectItem(accel->insertItem(CTRL + Key_S),
                     this, SLOT(toggleTimestamp()));

  // Drag & Drop
  connect( mainw, SIGNAL( textDropped(const QString&) ),
           SLOT( slotTextDropped(const QString&) ));
  connect( mainw, SIGNAL( urlsDropped(const QStringList&) ),
           SLOT( slotDropURLs(const QStringList&) ));
  connect( nicks, SIGNAL( urlsDropped( const QStringList&, const QString& )),
           SLOT( slotDccURLs( const QStringList&, const QString& )));

  f->mainw->setAcceptFiles( isPrivateChat() );
  resize(600, 360);
}


KSircTopLevel::~KSircTopLevel()
{
  // Cleanup and shutdown
  //  if(this == proc->getWindowList()["default"])
  //    write(sirc_stdin, "/quit\n", 7); // tell dsirc to close
  //
  //  if(proc->getWindowList()[channel_name])
  //    proc->getWindowList().remove(channel_name);

  //  if((channel_name[0] == '#') || (channel_name[0] == '&')){
  //    QString str = QString("/part ") + channel_name + "\n";
  //    emit outputLine(str);
  //  }

    kdDebug() << "**** ~KSircTopLevel!!!" << endl;

  if ( isPublicChat() ) {
      kdDebug() << "*** parting channel: " << channel_name << endl;
      QString str = QString("/part ") + channel_name + "\n";
      emit outputLine(str);
  }

  delete ticker;
  ticker = 0;
  delete user_controls;
  delete ChanParser;
  free( channel_name );
  channel_name = 0;
}

void KSircTopLevel::show()
{
  if(ticker){
    ticker->show();
    ticker->raise();
  }
  else{
    KMainWindow::show();
  }
}

//void KSircTopLevel::sirc_stop(bool STOP = FALSE)
//{
//  if(STOP == TRUE){
//    Buffer = TRUE;
//  }
//  else{
//    Buffer = FALSE;
//    if(LineBuffer->isEmpty() == FALSE)
//      sirc_receive(QString(""));
//  }
//}

void KSircTopLevel::TabNickCompletion()
{
  /*
   * Gets current text from lined find the last item and try and perform
   * a nick completion, then return and reset the line.
   */

  int start, end;
  QString s;

  if(tab_pressed > 0){
    s = tab_saved;
    start = tab_start;
    end = tab_end;
  }
  else{
    s = linee->text();
    tab_saved = s;
    end = linee->cursorPosition() - 1;
    start = s.findRev(" ", end, FALSE);
    tab_start = start;
    tab_end = end;

  }

  if(s.length() == 0){
    QString line = tab_nick + ": "; // tab_nick holds the last night since we haven't overritten it yet.
    linee->setText(line);
    linee->setCursorPosition(line.length());
    return;
  }

  if (start == -1) {
    tab_nick = findNick(s.mid(0, end+1), tab_pressed);
    if(tab_nick.isNull() == TRUE){
      tab_pressed = 0;
      tab_nick = findNick(s.mid(0, end+1), tab_pressed);
    }
    s.replace(0, end + 1, tab_nick);
  }
  else {
    tab_nick = findNick(s.mid(start + 1, end - start), tab_pressed);
    if(tab_nick.isNull() == TRUE){
      tab_pressed = 0;
      tab_nick = findNick(s.mid(start + 1, end - start), tab_pressed);
    }
    s.replace(start + 1, end - start, tab_nick);
  }

  int tab = tab_pressed + 1;

  linee->setText(s);

  linee->setCursorPosition(start + tab_nick.length() + 1);

  tab_pressed = tab; // setText causes lineeTextChanged to get called and erase tab_pressed

  connect(linee, SIGNAL(notTab()),
          this, SLOT(lineeNotTab()));

}

void KSircTopLevel::sirc_receive(QString str, bool broadcast)
{

  /*
   * read and parse output from dsirc.
   * call reader, split the read input into lines, parse the lines
   * then print line by line into the main text area.
   *
   * PROBLEMS: if a line terminates in mid line, it will get borken oddly
   *
   */

  /*
   * If we have too many lines, nuke the top 100, leave us with 100
   */
  int lines = 0;

  if(Buffer == FALSE){
    if(LineBuffer.count() >= 2){
      //mainw->setAutoUpdate(FALSE);
    }

    if( !str.isEmpty() ){
        LineBuffer.append( BufferedLine( str, broadcast ) );
    }

    ircListItem *item = 0;
    QString string;
    bool update = FALSE;
    BufferedLine line;

    // be careful not to use a QValueList iterator here, as it is possible
    // that we enter a local event loop (think of the ssfeprompt dialog!)
    // which might trigger a socketnotifier activation which in turn
    // might result in the execution of code that modifies the LineBuffer,
    // which would invalidate iterators (Simon)
    while ( LineBuffer.begin() != LineBuffer.end() )
    {
        line = *LineBuffer.begin();
        LineBuffer.remove( LineBuffer.begin() );

        // Get the need list box item, with colour, etc all set
        string = line.message;
        item = parse_input(string);
        // If we shuold add anything, add it.
        // Item might be null, if we shuoold ingore the line

        if(item){
            // Insert line to the end
            connect(this, SIGNAL(changeSize()),
                    item, SLOT(updateSize()));
            connect(this, SIGNAL(freezeUpdates(bool)),
                    item, SLOT(freeze(bool)));
            mainw->insertItem(item, -1);
            if(ticker){
                QString text;
                int colour = KSPainter::colour2num(*(item->defcolour()));
                if(colour >= 0){
                    text.setNum(colour);
                    text.prepend("~");
                }
                text.append(item->getText());
                ticker->mergeString(text + "~C // ");
            }
            lines++; // Mode up lin counter
            update = TRUE;
            // Don't announce server messages as they are
            // spread through all channels anyway
            if ( !line.wasBroadcast )
                emit changed();
        }
    }
    LineBuffer.clear(); // Clear it since it's been added

    if(mainw->count() > (1.25*kSircConfig->WindowLength) &&
       kSircConfig->WindowLength > 0){
        //mainw->setAutoUpdate(FALSE);
        update = TRUE;
        while(mainw->count() > (uint) kSircConfig->WindowLength)
           mainw->removeItem(0);
    }

    //mainw->setAutoUpdate(TRUE);

    mainw->scrollToBottom();
    mainw->repaint(FALSE); // Repaint, but not need to erase and cause fliker.
  }
  else{
    LineBuffer.append( BufferedLine( str, broadcast ) );
  }

//  bitBlt(kApp->desktop(), 0, 0, mainw);
}

void KSircTopLevel::sirc_line_return()
{

  /* Take line from SLE, and output if to dsirc */

  QString s = linee->text();

  if(s.length() == 0)
    return;

  tab_pressed = 0; // new line, zero the counter.

  //
  // Lookup the nick completion
  // Do this before we append the linefeed!!
  //

  int pos2;
  if(kSircConfig->NickCompletion == TRUE){
    if (!tab_nick.isEmpty())
	{
      addCompleteNick(tab_nick);
      tab_nick = QString::null;
	}
    if(s.find(QRegExp("^[^ :]+: "), 0) != -1){
      pos2 = s.find(": ", 0);
      if(pos2 < 1){
        kdDebug() << "Evil string: " << s << endl;
      }
      else
        s.replace(0, pos2, findNick(s.mid(0, pos2)));
    }
  }

  s += '\n'; // Append a need carriage return :)

  if((uint) nick_ring.at() < (nick_ring.count() - 1))
    nick_ring.next();
  else
    nick_ring.last();

  sirc_write(s);

  linee->setText("");

}

void KSircTopLevel::sirc_write(QString str)
{
  /*
   * Parse line forcommand we handle
   */
    //    kdDebug() << "*** got: " << str << endl;


  if((strncmp(str, "/join ", 6) == 0) ||
     (strncmp(str, "/j ", 3) == 0) ||
     (strncmp(str, "/query ", 7) == 0)){
    str = str.lower().simplifyWhiteSpace();
    int pos1 = str.find(' ') + 1;
    if(!pos1)
      return;
    int pos2 = str.find(' ', pos1);
    if(pos2 == -1)
      pos2 = str.length();
    if(pos1 > 2){
      QString name = str.mid(pos1, pos2 - pos1);
      // In case of a channel key, first join and then open
      // the toplevel, to avoid a "Could not join, wrong key"
      // when the new toplevel emits a /join on activation
      if(name[0] != '#'){
        emit open_toplevel(name);
        linee->setText("");
        return;
      }
      else {
        emit outputLine(str + "\n");
        emit open_toplevel(name);
      }
      // Finish sending /join
    }
    return;
  }
  else if(strncmp(str, "/server ", 6) == 0){
    QString command = "/eval &print(\"*E* Use The Server Controller\\n\");\n";
    sirc_write(command);
    linee->setText("");
    return;
  }
  else if((strncmp(str, "/part", 5) == 0) ||
          (strncmp(str, "/leave", 6) == 0) ||
          (strncmp(str, "/hop", 4) == 0) ||
          (strncmp(str, "/bye", 4) == 0) ||
          (strncmp(str, "/exit", 5) == 0) ||
          (strncmp(str, "/quit", 5) == 0)){
    QApplication::postEvent(this, new QCloseEvent()); // WE'RE DEAD
    linee->setText("");
    str.truncate(0);
    return;
  }

  //
  // Look at the command, if we're assigned a channel name, default
  // messages, etc to the right place.  This include /me, etc
  //

  if(!isSpecialWindow()) { // channel or private chat

    if(str[0] != '/'){
      str.prepend(QString("/msg ") + channel_name + QString(" "));
    }
    else if(qstrnicmp(str, "/me", 3) == 0){
      str.remove(0, 3);
      str.prepend(QString("/de ") + channel_name);
    }
  }

  // Write out line

  //  proc->stdin_write(str);
  mainw->scrollToBottom(TRUE);
  emit outputLine(str);
  //  kdDebug() << "*** wrote: " << str << endl;
}

ircListItem *KSircTopLevel::parse_input(QString string)
{
  ircListItem *result = 0;
  /*
   * Parsing routines are broken into 3 majour sections
   *
   * 1. Search and replace evil characters. The string is searched for
   * each character in turn (evil[i]), and the character string in
   * evil_rep[i] is replaced.
   *
   * 2. Parse control messages, add pixmaps, and colourize required
   * lines.  Caption is also set as required.
   *
   * 3. Create and return the ircListItem.
   *
   */

  /*
   * No-output get's set to 1 if we should ignore the line
   */

  /*
   * This is the second generation parsing code.
   * Each line type is determined by the first 3 characters on it.
   * Each line type is then sent to a parsing function.
   */
  parseResult *pResult = ChanParser->parse(string);

  parseSucc *item = dynamic_cast<parseSucc *>(pResult);
  parseError *err = dynamic_cast<parseError *>(pResult);
  if(item)
  {
    if(item->string.length() > 0)
      result = new ircListItem(item->string,item->colour,mainw,item->pm);
  }
  else if (err)
  {
    if(err->err.isEmpty() == FALSE)
    {
      kdWarning() << err->err << ": " << string << endl;
    }
    if (!err->str.isEmpty())
      result = new ircListItem(err->str, kSircConfig->colour_error, mainw, pix_madsmile);
  }
  else
  {
    result = new ircListItem(string, kSircConfig->colour_text, mainw, 0);
  }
  delete pResult;
  return result;
}

void KSircTopLevel::UserSelected(int index)
{
  if(index >= 0)
    user_controls->popup(this->cursor().pos());
}

void KSircTopLevel::UserParseMenu(int id)
{
  if(nicks->currentItem() < 0){
      KMessageBox::information(this, i18n("Warning! Dork at the helm, Captain!\nTry selecting a nick first!"), i18n("Warning, dork at the helm Captain!"));
      return;
  }
  QString s;
  s = QString("/eval $dest_nick='") +
      QString(nicks->text(nicks->currentItem())) +
      QString("';\n");
  sirc_write(s);
  // set $$dest_chan variable
  s = QString("/eval $dest_chan='") +
      QString( channel_name ) +
      QString("';\n");
  sirc_write(s);
  QString action = user_menu->at(id)->action;
  if (action.length() && action[0] == '/')
      action.remove(0, 1);
  s = QString("/eval &docommand(eval{\"") +
      action +
      QString("\"});\n");
  s.replace(QRegExp("\\$\\$"), "$");
  sirc_write(s);
  //  kdDebug() << "**** wrote: " << s << endl;
}

void KSircTopLevel::UserUpdateMenu()
{
  int i = 0;
  UserControlMenu *ucm;

  user_controls->clear();
  for(ucm = user_menu->first(); ucm != 0; ucm = user_menu->next(), i++){
    if(ucm->type == UserControlMenu::Seperator){
      user_controls->insertSeparator();
    }
    else{
      user_controls->insertItem(ucm->title, i);
      if(ucm->accel)
        user_controls->setAccel(i, ucm->accel);
      if((ucm->op_only == TRUE) && (opami == FALSE))
        user_controls->setItemEnabled(i, FALSE);
    }
  }
}

void KSircTopLevel::AccelScrollDownPage()
{
    mainw->pageDown();
}

void KSircTopLevel::AccelScrollUpPage()
{
    mainw->pageUp();
}
void KSircTopLevel::AccelPriorMsgNick()
{
  linee->setText(QString("/msg ") + nick_ring.current() + " ");

  if(nick_ring.at() > 0)
    nick_ring.prev();

}

void KSircTopLevel::AccelNextMsgNick()
{
  if(nick_ring.at() < ((int) nick_ring.count() - 1) )
    linee->setText(QString("/msg ") + nick_ring.next() + " ");
}

void KSircTopLevel::newWindow()
{
  NewWindowDialog w;
  connect(&w, SIGNAL(openTopLevel(QString)), SIGNAL(open_toplevel(QString)));
  w.exec();
}

void KSircTopLevel::closeEvent(QCloseEvent *e)
{
  KMainWindow::closeEvent( e );
  e->accept();

  // Let's not part the channel till we are acutally delete.
  // We should always get a close event, *BUT* we will always be deleted.
  //   if( isPublicChat() ) {
  //       QString str = QString("/part ") + channel_name + "\n";
  //       emit outputLine(str);
  //   }

  // Hide ourselves until we finally die
  hide();
  qApp->flushX();
  // Let's say we're closing, what ever connects to this should delete us.
  emit closing(this, channel_name);
}

void KSircTopLevel::resizeEvent(QResizeEvent *e)
{
  // bool update = mainw->autoUpdate();
  //mainw->setAutoUpdate(FALSE);
  KMainWindow::resizeEvent(e);
  //mainw->setAutoUpdate(update);
}

void KSircTopLevel::gotFocus()
{
  if(isVisible() == TRUE){
    if(have_focus == 0){
      if(channel_name[0] == '#'){
        QString str = "/join " + QString(channel_name) + "\n";
        emit outputLine(str);
		emit outputLine("/eval $query=''\n");
      }
	  else if (channel_name[0] != '!')
	  {
		  emit outputLine(QString("/eval $query='%1'\n").arg(channel_name));
	  }
      have_focus = 1;
      emit currentWindow(this);
      //    kdDebug() << channel_name << " got focusIn Event\n";
    }
  }
}

void KSircTopLevel::lostFocus()
{
  if(have_focus == 1){
    have_focus = 0;
    //    kdDebug() << channel_name << " got focusIn Event\n";
  }
}

void KSircTopLevel::control_message(int command, QString str)
{
  switch(command){
  case CHANGE_CHANNEL: // 001 is defined as changeChannel
    {
      QString server, chan;
      int bang;
      bang = str.find("!!!");
      if(bang < 0){
          chan = str;
          QString mname = name();
          int end = mname.find('_');
          if(end < 0){
              kdWarning() << "Change channel message was invalid: " << str << endl;
              break;
          }
          server = mname.mid(0, end);
      }
      else{
          server = str.mid(0, bang);
          chan = str.mid(bang + 3, str.length() - (bang + 3));
      }
      emit changeChannel(channel_name, chan);
      if(channel_name)
        delete channel_name;
      channel_name = qstrdup(chan.ascii());
      setName(server + "_" + QString(channel_name) + "_" + "toplevel");
      f->setName(QString(QString(QObject::name()) + "_" + "kstIFrame"));
      kmenu->setName(QString(QObject::name()) + "_ktoolframe");
      linee->setName(QString(QObject::name()) + "_" + "LineEnter");
      kmenu->show();
      have_focus = 0;
      setCaption(channel_name);
      mainw->scrollToBottom();
      emit currentWindow(this);

      bool isPrivate = isPrivateChat();
      f->mainw->setAcceptFiles( isPrivate );
      if ( isPrivate )
          f->nicks->hide();
      else
          f->nicks->show();
      break;
    }
  case STOP_UPDATES:
    Buffer = TRUE;
    break;
  case RESUME_UPDATES:
    Buffer = FALSE;
    if(LineBuffer.isEmpty() == FALSE)
      sirc_receive(QString::null);
    break;
  case REREAD_CONFIG:
    emit freezeUpdates(TRUE); // Stop the list boxes update
    mainw->setFont(kSircConfig->defaultfont);
    nicks->setFont(kSircConfig->defaultfont);
    linee->setFont(kSircConfig->defaultfont);
    UserUpdateMenu();  // Must call to update Popup.
    emit freezeUpdates(FALSE); // Stop the list boxes update
    emit changeSize(); // Have the ist box update correctly.
    initColors();
    mainw->scrollToBottom();
    update();
    break;
  case SET_LAG:
    if(str.isNull() == FALSE){
      bool ok = TRUE;

      str.truncate(6);
      double lag = str.toDouble(&ok);
      if(ok == TRUE){
        lag -= (lag*100.0 - int(lag*100.0))/100.0;
        QString s_lag;
                s_lag.sprintf("Lag: %.2f", lag);
        ksb_main->changeItem(s_lag, KSB_MAIN_LAG);
      }
      else{
        ksb_main->changeItem(str, KSB_MAIN_LAG);
      }
    }
    break;
  default:
    kdDebug() << "Unkown control message: " << str << endl;
  }
}

void KSircTopLevel::showTicker()
{
  myrect = geometry();
  mypoint = pos();
  ticker = new KSTicker(0, "ticker", WStyle_NormalBorder);
  ticker->setCaption(caption);
  kConfig->setGroup("TickerDefaults");
  ticker->setFont(kConfig->readFontEntry("font",
                                         new QFont( KGlobalSettings::fixedFont())));
  ticker->setSpeed(kConfig->readNumEntry("tick", 30),
                   kConfig->readNumEntry("step", 3));
  ticker->setPalette(mainw->palette());
  ticker->setBackgroundColor(*kSircConfig->colour_background); /* safe, set in init */

  connect(ticker, SIGNAL(doubleClick()),
          this, SLOT(unHide()));
  connect(ticker, SIGNAL(closing()),
          this, SLOT(terminate()));

  this->reparent(0, 0, QPoint(0,0));
  displayMgr->removeTopLevel(this);

  if(tickerrect.isEmpty() == FALSE){
    ticker->setGeometry(tickerrect);
    ticker->reparent(0, 0, tickerpoint, TRUE);
  }
  for(int i = 5; i > 0; i--)
    ticker->mergeString(QString(mainw->text(mainw->count()-i)) + " // ");

  ticker->show();
}

void KSircTopLevel::toggleTimestamp()
{
    kSircConfig->timestamp = !kSircConfig->timestamp;
    file->setItemChecked(tsitem,kSircConfig->timestamp);
    kConfig->setGroup("General");
    kConfig->writeEntry("TimeStamp", kSircConfig->timestamp);
    kConfig->sync();
    mainw->repaint(FALSE); // Repaint, but not need to erase and cause fliker.
}

void KSircTopLevel::unHide()
{
  tickerrect = ticker->geometry();
  tickerpoint = ticker->pos();
  int tick, step;
  ticker->speed(&tick, &step);
  kConfig->setGroup("TickerDefaults");
  kConfig->writeEntry("font", ticker->font());
  kConfig->writeEntry("tick", tick);
  kConfig->writeEntry("step", step);
  kConfig->sync();
  delete ticker;
  ticker = 0;
  displayMgr->newTopLevel(this, TRUE);
//  this->setGeometry(myrect);
//  this->recreate(0, getWFlags(), mypoint, TRUE);
//  this->show();
  linee->setFocus();  // Give SLE focus
}

QString KSircTopLevel::findNick(QString part, uint which)
{
  QStrList matches;
  for (QStringList::ConstIterator it = completeNicks.begin();
       it != completeNicks.end(); ++it)
    if ((*it).left(part.length()).lower() == part.lower() &&
        nicks->findNick(*it) >= 0)
      matches.append(*it);

  for(uint i=0; i < nicks->count(); i++){
    if (matches.contains(nicks->text(i)))
      continue;
    if(qstrlen(nicks->text(i)) >= part.length()){
      if(qstrnicmp(part, nicks->text(i), part.length()) == 0){
        QString qsNick = kSircConfig->nickName;
        if(qstrcmp(nicks->text(i), qsNick) != 0){ // Don't match your own nick
          matches.append(nicks->text(i));
        }
      }
    }
  }
  if(matches.count() > 0){
    if(which < matches.count())
      return matches.at(which);
    else
      return QString::null;
  }
  return part;

}

void KSircTopLevel::addCompleteNick(const QString &nick)
{
  QStringList::Iterator it = completeNicks.find(nick);
  if (it != completeNicks.end())
    completeNicks.remove(it);

  completeNicks.prepend(nick);
}

void KSircTopLevel::changeCompleteNick(const QString &oldNick, const QString &newNick)
{
  QStringList::Iterator it = completeNicks.find(oldNick);
  if (it != completeNicks.end())
    *it = newNick;
}

void KSircTopLevel::openQueryFromNick(const QString &nick)
{
    emit open_toplevel(nick.lower());
}

void KSircTopLevel::pasteToWindow()
{
  QString text = kApp->clipboard()->text();
  text += "\n";
  if((text.contains("\n") > 4) || (text.length() > 300)){
      switch( KMessageBox::questionYesNo(this, i18n("You are about to paste a very\nlarge number of lines.\nDo you really want to do this?"), i18n("Large Paste Requested"))) {
      case KMessageBox::Yes:
        break;
      default:
        linee->setText("");
        return;
      }
  }

  if(text.contains("\n") > 1){
    linee->setUpdatesEnabled(FALSE);

    QStringList lines = QStringList::split( '\n', text, true );
    QStringList::ConstIterator it = lines.begin();
    QStringList::ConstIterator end = lines.end();
    for (; it != end; ++it )
    {
        linee->setText( *it );
        sirc_line_return();
    }

    linee->setText("");
    linee->setUpdatesEnabled(TRUE);
    linee->update();
  }
  else{
    text.replace(QRegExp("\n"), "");
    QString line = linee->text();
    line += text;
    linee->setText(line);
  }
}

void KSircTopLevel::clearWindow()
{
  mainw->clear();
}

void KSircTopLevel::lineeNotTab()
{
  tab_pressed = 0;
  disconnect(linee, SIGNAL(notTab()),
             this, SLOT(lineeNotTab()));

}

void KSircTopLevel::toggleRootWindow()
{
}

void KSircTopLevel::saveCurrLog()
{

    KURL url = KFileDialog::getSaveFileName(QString::null,
                                            "*.log", 0L,
                                            i18n("Save chat / query logfile"));

    KTempFile temp;

    QTextStream *str = temp.textStream();
    unsigned int i=0;
    for ( i=0; i <= mainw->count(); i++ )
    {
        *str << mainw->text(i) << endl;
    }

    temp.close();
    KIO::NetAccess::upload(temp.name(), url);
}

void KSircTopLevel::iamDestroyed()
{
  emit objDestroyed(this);
}


void KSircTopLevel::slotTextDropped( const QString& text )
{
    if ( text.isEmpty() )
        return;

    if ( text.length() > 300 ) {
      if ( KMessageBox::questionYesNo(this,
                                      i18n("You dropped a very\nlarge number of lines.\nDo you really want me to go on?"),
                                      i18n("Large Drop"))
           == KMessageBox::No)
          return;
    }

    tab_pressed = 0;
    QString s = text + '\n';
    sirc_write(s);
}

void KSircTopLevel::slotDropURLs( const QStringList& urls )
{
    if ( !isPrivateChat() )
        return;

    slotDccURLs( urls, channel_name );
}

// sends the list of urls to $dest_nick
void KSircTopLevel::slotDccURLs( const QStringList& urls, const QString& nick )
{
    if ( urls.isEmpty() || nick.isEmpty() )
        return;

    QStringList::ConstIterator it = urls.begin();
    // QString s("/eval &docommand(eval{\"dcc send " + nick + " %1\"});\n");
    QString s("/dcc send " + nick + " %1\n");
    for ( ; it != urls.end(); ++it ) {
        QString file( *it );
        kdDebug() << "........ " << file << endl;
        if ( !file.isEmpty() )
            sirc_write(s.arg( file ));
    }
}

bool KSircTopLevel::isPrivateChat() const
{
    return ((channel_name[0] != '!') && (channel_name[0] != '&') &&
            (channel_name[0] != '#'));
}

bool KSircTopLevel::isPublicChat() const
{
    return ((channel_name[0] == '#') || (channel_name[0] == '&'));
}

bool KSircTopLevel::isSpecialWindow() const
{
    return (channel_name[0] == '!');
}

QWidget *KSircTopLevel::lineEdit() const
{
    return linee;
}

// ####################################################################



kstInside::kstInside ( QWidget * parent, const char * name, WFlags f )
    : QHBox(parent, name, f)
{
  pan = new QSplitter(QSplitter::Horizontal, this, "");

  mainw = new KSircListBox(pan, "");
  mainw->setFocusPolicy(QWidget::NoFocus);
  // mainw->setSmoothScrolling(TRUE);

  nicks = new aListBox(pan, "");
  nicks->setFocusPolicy(QWidget::NoFocus);
  nicks->hide(); // default = only the main widget

  QValueList<int> sizes;
  sizes << 85 << 15;
  pan->setSizes(sizes);
  pan->setResizeMode( mainw, QSplitter::Stretch );
  pan->setResizeMode( nicks, QSplitter::Stretch );

  setName(name);
}

kstInside::~kstInside()
{
  delete mainw;
  delete nicks;
  delete pan;
}


void kstInside::setName(const char *name)
{
  QObject::setName(name);
  my_name = name;
  panner_name = my_name + "_" + "Panner";
  mainw_name = my_name + "_" + "MainIrc";
  nicks_name = my_name + "_" + "NickListBox";
  linee_name = my_name + "_" + "LineEnter";

  pan->setName(panner_name);
  mainw->setName(mainw_name);
  nicks->setName(nicks_name);
//  linee->setName(linee_name);
}
#include "toplevel.moc"
