#include "displayMgr.h"
#include "config.h"

#include <qobject.h>


DisplayMgr::DisplayMgr()
{
}

DisplayMgr::~DisplayMgr()
{
}

void DisplayMgr::newTopLevel(QWidget *, bool )
{
  qWarning("Display Manger: newTopLevel called");
}

void DisplayMgr::removeTopLevel(QWidget *)
{
  qWarning("Display Manger: removeTopLevel called");
}

void DisplayMgr::show(QWidget *)
{
  qWarning("Display Manger: show called");
}

void DisplayMgr::raise(QWidget *)
{
  qWarning("Display Manger: raise called");
}

void DisplayMgr::setCaption(QWidget *, const QString&)
{
  qWarning("Display Manger: setCaption called");
}

