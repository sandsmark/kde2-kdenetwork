/* -------------------------------------------------------------

   application.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

 ------------------------------------------------------------- */

#include <kwin.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kurl.h>

#include "application.h"
#include "toplevel.h"


Application::Application()
 : KUniqueApplication()
{
}


Application::~Application()
{
}


int Application::newInstance()
{
  kdDebug(5004) << "Application::newInstance()" << endl;

  if (mainWidget()) {
    KWin::setOnDesktop(mainWidget()->winId(),KWin::currentDesktop());
    KWin::setActiveWindow(mainWidget()->winId());
  } else {
    if (isRestored()) {
      RESTORE(TopLevel);
    } else {
      TopLevel* top = new TopLevel();
      top->show();
    }
  }

  // process parameters...
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if (args->isSet("clipboard"))
    static_cast<TopLevel*>(mainWidget())->defineClipboard();
  else {
    if (args->count()>0) {
      QString phrase;
      for (int i=0;i<args->count();i++) {
        phrase += QString::fromLocal8Bit(args->arg(i));
        if (i+1 < args->count())
          phrase += " ";
      }
      static_cast<TopLevel*>(mainWidget())->define(phrase);
    } else
      static_cast<TopLevel*>(mainWidget())->normalStartup();
  }

  kdDebug(5004) << "Application::newInstance() done" << endl;
  return 0;
}

//--------------------------------

#include "application.moc"
