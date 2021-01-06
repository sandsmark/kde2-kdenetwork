/* -------------------------------------------------------------

   main.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias Hölzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

 ------------------------------------------------------------- */

#include <config.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "application.h"


static KCmdLineOptions knoptions[] =
{
  { "c", 0, 0 },
  { "clipboard", I18N_NOOP("define X11-clipboard content"), 0 },
  { "+[word/phrase]", I18N_NOOP("lookup the given word/phrase"), 0 },
  { 0, 0, 0 }
};


int main(int argc, char* argv[])
{
  KAboutData aboutData("kdict",
                        I18N_NOOP("Kdict"),
                        KDICT_VERSION,
                        I18N_NOOP("The KDE Dictionary Client"),
                        KAboutData:: License_Artistic,
                        "Copyright (c) 1999-2001, Christian Gebauer\nCopyright (c) 1998, Matthias Hoelzer",
                        0,
                        "http://www-user.rhrk.uni-kl.de/~gebauerc/kdict/");

  aboutData.addAuthor("Christian Gebauer",I18N_NOOP("Maintainer"),"gebauer@kde.org");
  aboutData.addAuthor("Matthias Hoelzer",I18N_NOOP("Original Author"),"hoelzer@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( knoptions );
  KUniqueApplication::addCmdLineOptions();

  if (!Application::start())
    return 0;

  Application app;

  return app.exec();
}
