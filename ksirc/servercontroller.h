/**********************************************************************

	--- Qt Architect generated file ---

	File: servercontroller.h
	Last generated: Sat Nov 29 08:50:19 1997
	
 Now Under CVS control.

 $$Id: servercontroller.h 103315 2001-06-21 15:28:58Z hausmann $$

 *********************************************************************/

#ifndef servercontroller_included
#define servercontroller_included

class servercontroller;
class ServMessage;
class ProcCommand;

#include <qdict.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qheader.h>

#include <klistview.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <ksystemtray.h>

#include "ksircprocess.h"

#include "puke/controller.h"

#include "kapopupmenu.h"

class ProcCommand // ServerController message
{
 public:
  static enum {
    addTopLevel,
    deleteTopLevel,
    procClose,
    newChannel,
    changeChannel,
    nickOnline,
    nickOffline,
    turnOffAutoCreate,
    turnOnAutoCreate
  } command;
};


class ServCommand // ServerController message
{
 public:
  static enum {
    updateFilters,
    updatePrefs
  } command;
};

class scInside : QFrame
{
  Q_OBJECT
  friend class servercontroller;
 public:
  scInside ( QWidget *parent = 0L, const char * name = 0, WFlags f=0,
	     bool allowLines = TRUE );
  ~scInside();

 protected:
  virtual void resizeEvent ( QResizeEvent * );

 private:
  KListView *ConnectionTree;
  QLabel *ASConn;

};

class KSircColour;

class dockServerController : public KSystemTray
{
  Q_OBJECT
    friend class servercontroller;
public:
  dockServerController(servercontroller *_sc, const char *_name);
  ~dockServerController();

private:
  servercontroller *sc;
};

class servercontroller : public KMainWindow
{
    Q_OBJECT
    friend class dockServerController;
public:

    servercontroller ( QWidget* parent = 0L, const char* name = NULL );
    virtual ~servercontroller();

signals:
    /**
      * Filter rules have changed, need to re-read and update.
      */
    virtual void filters_update();

    void ServMessage(QString server, int command, QString args);

public slots:
    // All slots are described in servercontroll.cpp file
    /**
      * Creates popup asking for new connection
      */
    virtual void new_connection();
    /**
      *  Args:
      *    QString: new server name or IP to connect to.
      *  Action:
      *	 Creates a new sirc process and window !default connected to the
      *	 server.  Does nothing if a server connection already exists.
      */
    virtual void new_ksircprocess(QString);
    /**
      * Creates popup asking for new channel name
      */
    virtual void new_channel();
    /**
      *  Args:
      *    str: name of the new channel to be created
      *  Action:
      *    Sends a signal to the currently selected server in the tree
      *    list and join the requested channel.  Does nothing if nothing
      *    is selected in the tree list.
      */
    virtual void new_toplevel(QString str);
    /**
      * Action:
      *     Notify all ksircprocess' to update filters
      */
    virtual void slot_filters_update();
    virtual void ToggleAutoCreate();
    virtual void colour_prefs();
    virtual void font_prefs();
    /**
     * Action: Popup a general preferences window which allows various
     * settings, etc.
     */
    virtual void general_prefs();
    virtual void font_update(const QFont&);
    virtual void filter_rule_editor();
    virtual void configChange();
    virtual void help_general();
    virtual void help_colours();
    virtual void help_filters();
    virtual void help_keys();
    virtual void about_ksirc();

    virtual void ProcMessage(QString server, int command, QString args);
    /**
     * On quit we sync the config to disk and exit
     */
    virtual void endksirc();

    QListViewItem * findChild( QListViewItem *parent, const QString& text );

protected slots:
  void WindowSelected(QListViewItem *);
  virtual void colour_prefsClosed();


protected:

  virtual void showEvent( QShowEvent *e );
  virtual void hideEvent( QHideEvent *e );
  virtual void closeEvent( QCloseEvent * );
  void saveDockingStatus();

  void saveGlobalProperties(KConfig *);
  void readGlobalProperties(KConfig *);

private:
    void saveSessionConfig();

    // La raison d'etre.  We don't run ConnectionTree outr selves, but
    // we get it from out helper class scInside.
    KListView *ConnectionTree;

    scInside *sci;

    // Menubar for the top.
    KMenuBar *MenuBar;

    // Hold a list of all KSircProcess's for access latter.  Index by server
    // name
    QDict<KSircProcess> proc_list;
    KAPopupMenu *options, *connections;
    int join_id, server_id;

    int open_toplevels;

    QPixmap *pic_icon;
    QPixmap *pic_server;
    QPixmap *pic_gf;
    QPixmap *pic_run;
    QPixmap *pic_ppl;
    QPixmap *pic_dock;

    PukeController *PukeC;
    KSircColour *kc;

    // Holds dockable widget
    dockServerController *dockWidget;
    bool we_are_exiting;

    QMap<QString,QStringList> m_sessionConfig;
};
#endif // servercontroller_included
