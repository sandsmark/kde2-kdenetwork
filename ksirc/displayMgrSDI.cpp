#include "displayMgrSDI.h"
#include "config.h"

#include "displayMgr.h"
#include <qwidget.h>

DisplayMgrSDI::DisplayMgrSDI()
{
}

DisplayMgrSDI::~DisplayMgrSDI(){
}

void DisplayMgrSDI::newTopLevel(QWidget *w, bool show){
  if(show == TRUE)
    w->show();
}

void DisplayMgrSDI::removeTopLevel(QWidget *){
}

void DisplayMgrSDI::show(QWidget *w){
  w->show();
}

void DisplayMgrSDI::raise(QWidget *w){
  w->show();
  w->raise();
}


void DisplayMgrSDI::setCaption(QWidget *w, const QString& cap){
  w->setCaption(cap);
}

