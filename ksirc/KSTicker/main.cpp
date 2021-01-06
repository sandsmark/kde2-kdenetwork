#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <qapplication.h> 
#include <qsocketnotifier.h>
#include <qregexp.h>

#include <unistd.h> 

#include "ksticker.h"
#include "main.h"
#include "../config.h"

static const char *description = 
	I18N_NOOP("Displays STDIN as a scrolling stock ticker");

static const char *version = "v0.0.1";


KConfig *kConfig;

StdInTicker::StdInTicker()
  : KSTicker()
{
  kConfig->setGroup("defaults");
  QFont font;
  font = kConfig->readFontEntry("font");
  font.setFixedPitch(TRUE);
  setFont(font);
  setSpeed(kConfig->readNumEntry("tick", 30), 
	   kConfig->readNumEntry("step", 3));
}

StdInTicker::~StdInTicker()
{
  int tick, step;
  speed(&tick, &step);
  kConfig->setGroup("defaults");
  kConfig->writeEntry("font", KSTicker::font());
  kConfig->writeEntry("tick", tick);
  kConfig->writeEntry("step", step);
  kConfig->writeEntry("text", colorGroup().text() ); 
  kConfig->writeEntry("background", colorGroup().background() );
  kConfig->sync();
}

void StdInTicker::readsocket(int socket)
{
  char buf[1024];
  int bytes = read(socket, buf, 1024);
  buf[ bytes ] = '\0'; // David : null-terminate the string and build ...
  QString str(buf);    // a QString (the (char*,int) constructor disappeared)
  str.replace(QRegExp("\n"), " // ");
  mergeString(str);
}

void StdInTicker::closeEvent ( QCloseEvent *e )
{
  KSTicker::closeEvent(e);
  delete this;
}


int main(int argc, char **argv){
  KLocale::setMainCatalogue("ksirc");
  KCmdLineArgs::init(argc, argv, "ksticker", description, version );

  KApplication a;

  kConfig = a.config();

  KSTicker *kst = new StdInTicker();
  QSocketNotifier *sn = new QSocketNotifier(0, QSocketNotifier::Read);
  QObject::connect(sn, SIGNAL(activated(int)), 
	  kst, SLOT(readsocket(int)));
  a.setMainWidget(kst);
  kst->show();
  return a.exec();
}

#include "main.moc"
