/*************************************************************************

 Main KSirc start

 $$Id: ksirc.cpp 99420 2001-05-30 08:56:38Z pfeiffer $$

 Main start file that defines 3 global vars, etc

*************************************************************************/

/*
 * Needed items
 * 4. Send a /quit and/or kill dsirc on exit
 * */


#include "servercontroller.h"
#include "welcomeWin.h"

#include <iostream>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include <qfont.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kaboutdata.h>

#include "config.h"
#include "version.h"
#include "cdate.h"
#include <klocale.h>
#include <kcmdlineargs.h>

static const char *description =
        I18N_NOOP("KDE Irc client");


KApplication *kApp;
KConfig *kConfig;
global_config *kSircConfig;
//QDict<KSircTopLevel> TopList;
//QDict<KSircMessageReceiver> TopList;

class KCmdLineOptions options[] =
{
    { "nick <nickname>", I18N_NOOP( "Nickname to use." ), 0 } ,
    { "server <server>", I18N_NOOP( "Server to connect to on startup." ), 0 },
    { 0, 0, 0 }
};

int main( int argc, char ** argv )
{
  KAboutData aboutData( "ksirc", I18N_NOOP("KSirc"),
    KSIRC_VERSION, description, KAboutData::License_Artistic,
    "(c) 1997-1999, Andrew Stanley-Jones");
  aboutData.addAuthor("Andrew Stanley-Jones",0, "asj@chowtown.cban.com");
  aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");
  aboutData.addAuthor("Carsten Pfeiffer",0, "pfeiffer@kde.org");
  aboutData.addAuthor("Malte Starostik",0, "malte.starostik@t-online.de");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );


  // Start the KDE application
  kApp = new KApplication;

  kConfig = kApp->config();

  // Get config, and setup internal structure.

  kSircConfig = new global_config;

  kConfig->setGroup("Colours");
  kSircConfig->colour_text = new QColor(kConfig->readColorEntry("text", &Qt::black));
  kSircConfig->colour_info = new QColor(kConfig->readColorEntry("info", &Qt::blue));
  kSircConfig->colour_chan = new QColor(kConfig->readColorEntry("chan", &Qt::green));
  kSircConfig->colour_error = new QColor(kConfig->readColorEntry("error", &Qt::red));
  if(kSircConfig->colour_text == 0x0)
      kSircConfig->colour_text = new QColor("black");
  if(kSircConfig->colour_info == 0x0)
      kSircConfig->colour_info = new QColor("blue");
  if(kSircConfig->colour_chan == 0x0)
      kSircConfig->colour_chan = new QColor("green");
  if(kSircConfig->colour_error == 0x0)
      kSircConfig->colour_error = new QColor("red");


  QWidget *w = new QWidget();
  kSircConfig->colour_background = new QColor(kConfig->readColorEntry("Background", new QColor(w->colorGroup().mid())));
  delete w;

  kSircConfig->filterKColour = kConfig->readBoolEntry("kcolour", true);
  kSircConfig->filterMColour = kConfig->readBoolEntry("mcolour", false);

  kSircConfig->nickFHighlight = kConfig->readNumEntry("nickfcolour", -1);
  kSircConfig->nickBHighlight = kConfig->readNumEntry("nickbcolour", -1);
  kSircConfig->usHighlight = kConfig->readNumEntry("uscolour", -1);

  /*
  QString ld_path = getenv("LD_LIBRARY_PATH");
  ld_path += ":" + locate( "appdata", "" ) + ":"; // this returns QString::null
  ld_path.prepend("LD_LIBRARY_PATH=");
  putenv((char *)ld_path.data());
  */

  kConfig->setGroup("GlobalOptions");
  kSircConfig->defaultfont = kConfig->readFontEntry("MainFont", new QFont("fixed"));
  kConfig->setGroup("General");
  kSircConfig->DisplayMode = kConfig->readNumEntry("DisplayMode", 0);
  kSircConfig->WindowLength = kConfig->readNumEntry("WindowLength", 200);
  kSircConfig->transparent = kConfig->readBoolEntry("transparent", false);
  kSircConfig->BackgroundPix = kConfig->readBoolEntry("BackgroundPix", false);
  kSircConfig->BackgroundFile = kConfig->readEntry("BackgroundFile", "");
  kSircConfig->timestamp = kConfig->readBoolEntry("TimeStamp", false);
  kConfig->setGroup( "StartUp" );
  kSircConfig->nickName = kConfig->readEntry( "Nick" );

#if 0
  kConfig->setGroup("ReleaseNotes");
  if(kConfig->readNumEntry("LastRunRelease", 0) < COMPILE_DATE){
    welcomeWin ww;
    if(ww.exec())
      kConfig->writeEntry("LastRunRelease", COMPILE_DATE);
  }
#endif

  servercontroller *sc = 0x0;
  sc = new servercontroller(0, "servercontroller");
  kApp->setMainWidget(sc);

  if (KMainWindow::canBeRestored(1)) {
      sc->restore(1, false);
  }

  else { // no Session management -> care about docking, geometry, etc.
      kConfig->setGroup("ServerController");
      bool docked = kConfig->readBoolEntry("Docked", FALSE);
      if ( !docked )
        sc->show();

      const QRect geom = kConfig->readRectEntry("Size");
      if(! geom.isEmpty())
          sc->setGeometry(geom);

      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      QCString nickName = args->getOption( "nick" );
      QCString server = args->getOption( "server" );

      if ( !nickName.isEmpty() )
          kSircConfig->nickName = nickName;

      if ( !server.isEmpty() )
          sc->new_ksircprocess( QString::fromLocal8Bit( server ) );

      args->clear();
  }

  try {
      kApp->exec();
  }
  catch(...){
      std::cerr << "Caught Unkown Exception, uhoh!!!\n";
      std::cerr << "Dying!!!\n";
      exit(10);
  }

  kConfig->sync();
}
