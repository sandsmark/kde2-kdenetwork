#include <kdebug.h>

#include "ksirclistbox.h"
#include "../config.h"
#include "../KSircListBox/irclistitem.h"

PObject *createWidget(CreateArgs &ca) 
{
  PKSircListBox *plb = new PKSircListBox(ca.parent);
  KSircListBox *lb;
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("KSircListBox") == TRUE){
    lb = (KSircListBox *) ca.fetchedObj;
    plb->setDeleteAble(FALSE);
  }
  else if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    lb = new KSircListBox((QWidget *) ca.parent->widget());
  else
    lb = new KSircListBox();
  plb->setWidget(lb);
  plb->setWidgetId(ca.pwI);
  plb->setPukeController(ca.pc);
  return plb;
}


PKSircListBox::PKSircListBox(PObject *parent) 
  : PListBox(parent)
{
  //  kdDebug() << "PListBox PListBox called" << endl;
  lb = 0;
  setWidget(lb);
}

PKSircListBox::~PKSircListBox() 
{
  //  kdDebug() << "PListBox: in destructor" << endl;
  /*
  delete widget();     // Delete the frame
  lb=0;          // Set it to 0
  setWidget(lb); // Now set all widget() calls to 0.
  */
}

void PKSircListBox::messageHandler(int fd, PukeMessage *pm) 
{
  PukeMessage pmRet;
  switch(pm->iCommand){
  case PUKE_KSIRCLISTBOX_TOBOTTOM:
    if(!checkWidget())
      return;
    
    pmRet.iArg = widget()->scrollToBottom((bool) pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_INSERT: // Overriden since Insert is not virtual
    if(!checkWidget())
      return;

    widget()->insertItem(new ircListItem(pm->cArg, &(widget()->colorGroup().text()), widget()), pm->iArg);
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = widget()->count();
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  case PUKE_LISTBOX_REMOVE:
    if(!checkWidget())
      return;
    
    widget()->removeItem(pm->iArg);
    
    pmRet.iCommand = - pm->iCommand;
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
    break;
  default:
    PListBox::messageHandler(fd, pm);
  }
}

void PKSircListBox::setWidget(QObject *_lb) 
{
  if(_lb != 0 && _lb->inherits("KSircListBox") == FALSE)
  {
    errorInvalidSet(_lb);
    return;
  }

  lb = (KSircListBox *) _lb;
  PListBox::setWidget(lb);

}


KSircListBox *PKSircListBox::widget() 
{
  return lb;
}

bool PKSircListBox::checkWidget(){ 
  if(widget() == 0){
    kdDebug() << "PKSircListBox: No Widget set" << endl;
    return FALSE;
  }
  return TRUE;
}

#include "ksirclistbox.moc"

