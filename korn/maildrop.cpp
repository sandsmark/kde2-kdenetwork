/*
* maildrop.cpp -- Implementation of class KMailDrop.
* Author:  Sirtaj Singh Kang
* Version:  $Id: maildrop.cpp 61109 2000-08-15 12:36:46Z hausmann $
* Generated:  Sat Nov 29 20:07:45 EST 1997
*/

#include<qstring.h>
#include<qcolor.h>
#include<assert.h>
#include<qapplication.h>

#include<kconfigbase.h>
#include<kdebug.h>

#include"utils.h"
#include"maildrop.h"
#include"gencfg.h"
#include"comcfg.h"
#include"dropdlg.h"

const char *KMailDrop::TypeConfigKey = "type";
const char *KMailDrop::CaptionConfigKey = "caption";
const char *KMailDrop::ClickConfigKey = "onclick";
const char *KMailDrop::NewMailConfigKey = "onnewmail";
const char *KMailDrop::BgColourConfigKey = "bgcolour";
const char *KMailDrop::FgColourConfigKey = "fgcolour";
const char *KMailDrop::NBgColourConfigKey = "newmailbgcolour";
const char *KMailDrop::NFgColourConfigKey = "newmailfgcolour";
const char *KMailDrop::IconConfigKey = "icon";
const char *KMailDrop::NewMailIconConfigKey = "newmailicon";
const char *KMailDrop::DisplayStyleConfigKey = "displaystyle";

KMailDrop::KMailDrop()
  : _style(Plain),
    _lastCount(0)
{
  connect(this, SIGNAL(changed(int)), SLOT(setCount(int)));
}

KMailDrop::~KMailDrop()
{
  // Empty.
}

void KMailDrop::setCount(int count)
{
  _lastCount = count;
}

void KMailDrop::notifyClients()
{
  emit(notifyDisconnect());
}

void KMailDrop::addConfigPage(KDropCfgDialog * dlg)
{
  dlg->addConfigPage(new KGeneralCfg(this));
  dlg->addConfigPage(new KCommandsCfg(this));
}

void KMailDrop::forceCountZero()
{
  emit changed(0);
}

bool KMailDrop::readConfigGroup(const KConfigBase & c)
{
  _caption    = c.readEntry(fu(CaptionConfigKey));
  _clickCmd   = c.readEntry(fu(ClickConfigKey));
  _nMailCmd   = c.readEntry(fu(NewMailConfigKey));
  _style      = Style(c.readUnsignedNumEntry(fu(DisplayStyleConfigKey), Plain));
  _bgColour   = c.readColorEntry(fu(BgColourConfigKey), &QApplication::palette().active().background());
  _fgColour   = c.readColorEntry(fu(FgColourConfigKey), &QApplication::palette().active().text());
  _nbgColour  = c.readColorEntry(fu(NBgColourConfigKey), &QApplication::palette().active().background());
  _nfgColour  = c.readColorEntry(fu(NFgColourConfigKey), &QApplication::palette().active().text());
  _icon       = c.readEntry(fu(IconConfigKey));
  _nIcon      = c.readEntry(fu(NewMailIconConfigKey));

  emit(configChanged());

  return true;
}

bool KMailDrop::writeConfigGroup(KConfigBase & c) const
{
  c.writeEntry(fu(TypeConfigKey),         type());
  c.writeEntry(fu(CaptionConfigKey),      caption());
  c.writeEntry(fu(ClickConfigKey),        clickCmd());
  c.writeEntry(fu(NewMailConfigKey),      newMailCmd());
  c.writeEntry(fu(DisplayStyleConfigKey), _style);
  c.writeEntry(fu(BgColourConfigKey),     _bgColour);
  c.writeEntry(fu(FgColourConfigKey),     _fgColour);
  c.writeEntry(fu(NBgColourConfigKey),    _nbgColour);
  c.writeEntry(fu(NFgColourConfigKey),    _nfgColour);
  c.writeEntry(fu(IconConfigKey),         _icon);
  c.writeEntry(fu(NewMailIconConfigKey),  _nIcon);

  return true;
}

void KMailDrop::setCaption(QString s)      
{
  _caption = s;
  emit(configChanged());
}

void KMailDrop::setClickCmd(QString s)     
{
  kdDebug() << "KMailDrop::setClickCmd(" << s << ")" << endl;
  _clickCmd = s;
  emit(configChanged());
}

void KMailDrop::setNewMailCmd(QString s)   
{
  _nMailCmd = s;
  emit(configChanged());
}

void KMailDrop::setDisplayStyle(Style s)   
{
  _style = s;
  emit(configChanged());
}

void KMailDrop::setBgColour(QColor c)      
{
  _bgColour = c;
  emit(configChanged());
}

void KMailDrop::setFgColour(QColor c)      
{
  _fgColour = c;
  emit(configChanged());
}

void KMailDrop::setNewBgColour(QColor c)   
{
  _nbgColour = c;
  emit(configChanged());
}

void KMailDrop::setNewFgColour(QColor c)   
{
  _nfgColour = c;
  emit(configChanged());
}

void KMailDrop::setIcon(QString s)         
{
  kdDebug() << "KMailDrop::setIcon(" << s << ")" << endl;
  _icon = s;
  emit(configChanged());
}

void KMailDrop::setNewIcon(QString s)      
{
  _nIcon = s;
  emit(configChanged());
}

#include "maildrop.moc"
