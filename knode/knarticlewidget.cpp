/*
    knarticlewidget.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qstring.h>
#include <qclipboard.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kprinter.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>

#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kprocess.h>
#include <kcharsets.h>
#include <kaction.h>

#include "resource.h"
#include "knmime.h"
#include "knarticlewidget.h"
#include "knglobals.h"
#include "knarticlemanager.h"
#include "knarticlewindow.h"
#include "knfoldermanager.h"
#include "knarticlefactory.h"
#include "knconfigmanager.h"
#include "kngroup.h"
#include "knfolder.h"
#include "knnntpaccount.h"
#include "knstringsplitter.h"
#include "utilities.h"
#include "knscoring.h"
#include "knpgp.h"
#include "knode.h"

#define PUP_OPEN    1000
#define PUP_SAVE    2000
#define PUP_COPYURL 3000
#define PUP_SELALL  4000
#define PUP_COPY    5000

#define HDR_COL   0
#define QCOL_1    1
#define QCOL_2    2
#define QCOL_3    3


KNSourceViewWindow::KNSourceViewWindow(const QString &htmlCode)
  : KTextBrowser(0)
{
  setWFlags(WType_TopLevel | WDestructiveClose);
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  setCaption(kapp->makeStdCaption(i18n("Article Source")));
  QColorGroup pcg(paperColorGroup());
  pcg.setColor(QColorGroup::Base, app->backgroundColor());
  pcg.setColor(QColorGroup::Text, app->textColor());
  setPaperColorGroup(pcg);
  setLinkColor(app->linkColor());
  setFont(knGlobals.cfgManager->appearance()->articleFixedFont());

  QStyleSheetItem *style;
  style=new QStyleSheetItem(styleSheet(), "txt");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);

  setText(QString("<qt><txt>%1</txt></qt>").arg(htmlCode));
  KNHelper::restoreWindowSize("sourceWindow", this, QSize(500,300));
  show();
}


KNSourceViewWindow::~KNSourceViewWindow()
{
  KNHelper::saveWindowSize("sourceWindow",size());
}


void KNSourceViewWindow::viewportMouseReleaseEvent(QMouseEvent *e)
{
  QTextBrowser::viewportMouseReleaseEvent(e);

  if (e->button()==LeftButton) {
    if(hasSelectedText() && !selectedText().isEmpty())
      copy();
  }
}


//=============================================================================================================


KNMimeSource::KNMimeSource(QByteArray data, QCString mimeType)
 : d_ata(data), m_imeType(mimeType)
{}


KNMimeSource::~KNMimeSource()
{}


const char* KNMimeSource::format(int n) const
{
  return (n<1)? m_imeType.data() : 0;
}


QByteArray KNMimeSource::encodedData(const char *) const
{
  return d_ata;
}


//=============================================================================================================


KNArticleWidget::KNArticleWidget(KActionCollection* actColl, QWidget *parent, const char *name )
    : KTextBrowser(parent, name), a_rticle(0), a_tt(0), h_tmlDone(false), emuKMail(false), a_ctions(actColl)
{
  i_nstances.append(this);
  setNotifyClick( true );

  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  setFocusPolicy(QWidget::WheelFocus);

  installEventFilter(this);

  //popups
  u_rlPopup=new KPopupMenu();
  u_rlPopup->insertItem(SmallIcon("fileopen"),i18n("&Open Link"), PUP_OPEN);
  u_rlPopup->insertItem(SmallIcon("editcopy"),i18n("&Copy Link Location"), PUP_COPYURL);
  a_ttPopup=new KPopupMenu();
  a_ttPopup->insertItem(SmallIcon("fileopen"),i18n("&Open Attachment"), PUP_OPEN);
  a_ttPopup->insertItem(SmallIcon("filesave"),i18n("&Save Attachment"), PUP_SAVE);

  //actions
  a_ctSave              = KStdAction::save(this, SLOT(slotSave()), a_ctions);
  a_ctSave->setText(i18n("&Save..."));
  a_ctPrint             = KStdAction::print(this, SLOT(slotPrint()), a_ctions);
  a_ctSelAll            = KStdAction::selectAll(this, SLOT(slotSelectAll()), a_ctions);
  a_ctCopy              = KStdAction::copy(this, SLOT(copy()), a_ctions);

  a_ctReply             = new KAction(i18n("&Followup to Newsgroup..."),"message_reply", Key_R , this,
                          SLOT(slotReply()), a_ctions, "article_postReply");
  a_ctRemail            = new KAction(i18n("Reply by E-&Mail..."),"mail_reply", Key_A , this,
                          SLOT(slotRemail()), a_ctions, "article_mailReply");
  a_ctForward           = new KAction(i18n("Forw&ard by E-Mail..."),"mail_forward", Key_F , this,
                          SLOT(slotForward()), a_ctions, "article_forward");
  a_ctCancel            = new KAction(i18n("article","&Cancel Article"), 0 , this,
                          SLOT(slotCancel()), a_ctions, "article_cancel");
  a_ctSupersede         = new KAction(i18n("S&upersede Article..."), 0 , this,
                          SLOT(slotSupersede()), a_ctions, "article_supersede");
  a_ctVerify            = new KAction(i18n("&Verify PGP Signature"), 0, this,
                          SLOT(slotVerify()), a_ctions, "article_verify");
  a_ctToggleFullHdrs    = new KToggleAction(i18n("Show &All Headers"), "text_block", 0 , this,
                          SLOT(slotToggleFullHdrs()), a_ctions, "view_showAllHdrs");
  a_ctToggleFullHdrs->setChecked(knGlobals.cfgManager->readNewsViewer()->showFullHdrs());
  a_ctToggleRot13       = new KToggleAction(i18n("&Unscramble (Rot 13)"), "decrypted", 0 , this,
                          SLOT(slotToggleRot13()), a_ctions, "view_rot13");
  a_ctToggleFixedFont   = new KToggleAction(i18n("U&se Fixed Font"),  Key_X , this,
                          SLOT(slotToggleFixedFont()), a_ctions, "view_useFixedFont");
  a_ctToggleFixedFont->setChecked(knGlobals.cfgManager->readNewsViewer()->useFixedFont());
  a_ctViewSource        = new KAction(i18n("&View Source"),  0 , this,
                          SLOT(slotViewSource()), a_ctions, "article_viewSource");

  a_ctSetCharset = new KSelectAction(i18n("Chars&et"), 0, a_ctions, "set_charset");
  QStringList cs=KGlobal::charsets()->availableEncodingNames();
  cs.prepend(i18n("Automatic"));
  a_ctSetCharset->setItems(cs);
  a_ctSetCharset->setCurrentItem(0);
  connect(a_ctSetCharset, SIGNAL(activated(const QString&)),
          this, SLOT(slotSetCharset(const QString&)));
  a_ctSetCharsetKeyb = new KAction(i18n("Charset"), Key_C, this,
                                   SLOT(slotSetCharsetKeyboard()), a_ctions, "set_charset_keyboard");


  o_verrideCS=KNHeaders::Latin1;
  f_orceCS=false;

  //timer
  t_imer=new QTimer(this);
  connect(t_imer, SIGNAL(timeout()), this, SLOT(slotTimeout()));

  //config
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);
  applyConfig();
}


KNArticleWidget::~KNArticleWidget()
{
  if(a_rticle && a_rticle->isOrphant())
    delete a_rticle; //don't leak orphant articles

  i_nstances.removeRef(this);
  delete a_tt;
  delete a_ttPopup;
  delete u_rlPopup;
  delete f_actory;
}


bool KNArticleWidget::scrollingDownPossible()
{
  return ((contentsY()+visibleHeight())<contentsHeight());
}


void KNArticleWidget::scrollDown()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, visibleHeight()-offs);
}


void KNArticleWidget::focusInEvent(QFocusEvent *e)
{
  emit focusChanged(e);
  QTextBrowser::focusInEvent(e);

}


void KNArticleWidget::focusOutEvent(QFocusEvent *e)
{
  emit focusChanged(e);
  QTextBrowser::focusOutEvent(e);
}


void KNArticleWidget::keyPressEvent(QKeyEvent *e)
{
  if ( !e ) return;

  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;

  switch(e->key()) {
    case Key_Prior:
      scrollBy( 0, -visibleHeight()+offs);
      break;
    case Key_Next:
      scrollBy( 0, visibleHeight()-offs);
      break;
    case Key_Left:
      if (emuKMail)
        emit(keyLeftPressed());
      else
        QTextBrowser::keyPressEvent(e);
      break;
    case Key_Right:
      if (emuKMail)
        emit(keyRightPressed());
      else
        QTextBrowser::keyPressEvent(e);
      break;
    default:
      QTextBrowser::keyPressEvent(e);
  }
}


bool KNArticleWidget::eventFilter(QObject *o, QEvent *e)
{
  if ((e->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(e)->key() == Key_Tab)) {
    emit(focusChangeRequest(this));
    if (!hasFocus())  // focusChangeRequest was successfull
      return true;
  }
  return KTextBrowser::eventFilter(o, e);
}


void KNArticleWidget::viewportMousePressEvent(QMouseEvent *e)
{
  QString a=QTextBrowser::anchorAt(e->pos());
  if(!a.isEmpty() && (e->button()==RightButton || e->button()==MidButton))
    anchorClicked(a, e->button(), &(e->globalPos()));
  else
    if (e->button()==RightButton)
      b_odyPopup->popup(e->globalPos());

  QTextBrowser::viewportMousePressEvent(e);
}


void KNArticleWidget::viewportMouseReleaseEvent(QMouseEvent *e)
{
  QTextBrowser::viewportMouseReleaseEvent(e);

  if (e->button()==LeftButton) {
    if(hasSelectedText() && !selectedText().isEmpty()) {
      copy();
      a_ctCopy->setEnabled(true);
    } else
      a_ctCopy->setEnabled(false);
  }
}


bool KNArticleWidget::canDecode8BitText(const QCString &charset)
{
  if(charset.isEmpty())
    return false;
  bool ok=true;
  (void) KGlobal::charsets()->codecForName(charset,ok);
  return ok;
}


void KNArticleWidget::applyConfig()
{
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();
  KNConfig::ReadNewsViewer *rnv=knGlobals.cfgManager->readNewsViewer();

  //custom tags <bodyblock> , <headerblock>, <txt_attachment>
  QStyleSheetItem *style;
  style=new QStyleSheetItem(styleSheet(), "bodyblock");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  if (rnv->showHeaderDecoration())
    style->setMargin(QStyleSheetItem::MarginAll, 5);
  else
    style->setMargin(QStyleSheetItem::MarginAll, 0);
  if (rnv->rewrapBody())
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNormal);
  else
    style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);

  style=new QStyleSheetItem(styleSheet(), "headerblock");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  if (rnv->showHeaderDecoration()) {
    style->setMargin(QStyleSheetItem::MarginLeft, 10);
    style->setMargin(QStyleSheetItem::MarginVertical, 2);
  } else {
    style->setMargin(QStyleSheetItem::MarginLeft, 0);
    style->setMargin(QStyleSheetItem::MarginRight, 0);
    style->setMargin(QStyleSheetItem::MarginTop, 0);
    style->setMargin(QStyleSheetItem::MarginBottom, 2);
  }

  style=new QStyleSheetItem(styleSheet(), "txt_attachment");
  style->setDisplayMode(QStyleSheetItem::DisplayBlock);
  style->setWhiteSpaceMode(QStyleSheetItem::WhiteSpaceNoWrap);

  QColorGroup pcg(paperColorGroup());
  pcg.setColor(QColorGroup::Base, app->backgroundColor());
  pcg.setColor(QColorGroup::Text, app->textColor());
  setPaperColorGroup(pcg);
  setLinkColor(app->linkColor());

  if(!knGlobals.cfgManager->readNewsGeneral()->autoMark())
    t_imer->stop();

  emuKMail = ((this==knGlobals.artWidget) && knGlobals.cfgManager->readNewsNavigation()->emulateKMail());

  updateContents();
}


QString KNArticleWidget::toHtmlString(const QString &line, bool parseURLs, bool beautification, bool allowRot13)
{
  QString text,result;
  QRegExp regExp;
  uint len=line.length();
  int matchLen;
  bool forceNBSP=false; //use "&nbsp;" for spaces => workaround for a bug in QTextBrowser

  if (allowRot13 && r_ot13)
    text = KNHelper::rot13(line);
  else
    text = line;

  if (!knGlobals.cfgManager->readNewsViewer()->interpretFormatTags())
    beautification=false;

  uint lastReplacement=0;
  for(uint idx=0; idx<len; idx++){

    switch(text[idx].latin1()) {

      case '\r':  lastReplacement=idx; break;
      case '\n':  lastReplacement=idx; result+="<br>"; break;
      case '<' :  lastReplacement=idx; result+="&lt;"; break;
      case '>' :  lastReplacement=idx; result+="&gt;"; break;
      case '&' :  lastReplacement=idx; result+="&amp;"; break;
      case '"' :  lastReplacement=idx; result+="&quot;"; break;
      case '\t':  lastReplacement=idx; result+="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"; break;  // tab == 8 spaces

      case 32 :
        if(text[idx+1].latin1()==32) {
          while(text[idx].latin1()==32) {
            result+="&nbsp;";
            idx++;
          }
          idx--;
          forceNBSP=true; // force &nbsp; for the rest of this line
        } else
          if(idx==0 || forceNBSP) {
            result+="&nbsp;";
            forceNBSP=true; // force &nbsp; for the rest of this line
          } else
            result+=' ';
        lastReplacement=idx;
        break;

      case '@' :            // email-addresses or message-ids
        if (parseURLs) {
          uint startIdx = idx;
          // move backwards to the begin of the address, stop when
          // the end of the last replacement is reached. (

          while ((startIdx>0) && (startIdx>lastReplacement) && (text[startIdx-1]!=' ') && (text[startIdx-1]!='\t')
                                                            && (text[startIdx-1]!=',') && (text[startIdx-1]!='<')
                                                            && (text[startIdx-1]!='>') && (text[startIdx-1]!='(')
                                                            && (text[startIdx-1]!=')') && (text[startIdx-1]!='[')
                                                            && (text[startIdx-1]!=']') && (text[startIdx-1]!='{')
                                                            && (text[startIdx-1]!='}') )
            startIdx--;

          regExp="[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,startIdx,&matchLen)!=-1) {
            if (text[startIdx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[startIdx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            if (matchLen < 3)
              result+=text[idx];
            else {
              result.remove(result.length()-(idx-startIdx), idx-startIdx);
              result+=QString::fromLatin1("<a href=\"addrOrId:") + text.mid(startIdx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(startIdx,matchLen) + QString::fromLatin1("</a>");
              idx=startIdx+matchLen-1;
              lastReplacement=idx;
            }
            break;
          }
        }
        result+=text[idx];
        break;

      case 'h' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'h'
          regExp="^https?://[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'w' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='w')) {   // don't do all the stuff for every 'w'
          regExp="^www\\.[^\\s<>()\"|\\[\\]{}]+\\.[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"http://") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'f' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='t')) {   // don't do all the stuff for every 'f'
          regExp="^ftp://[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
          regExp="^ftp\\.[^\\s<>()\"|\\[\\]{}]+\\.[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"ftp://") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'n' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='e')) {   // don't do all the stuff for every 'e'
          regExp="^news:[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case 'm' :
        if((parseURLs)&&
           (text[idx+1].latin1()=='a')) {   // don't do all the stuff for every 'm'
          regExp="^mailto:[^\\s<>()\"|\\[\\]{}]+";
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (text[idx+matchLen-1]=='.')   // remove trailing dot
              matchLen--;
            else if (text[idx+matchLen-1]==',')   // remove trailing comma
              matchLen--;
            result+=QString::fromLatin1("<a href=\"") + text.mid(idx,matchLen) +
                    QString::fromLatin1("\">") + text.mid(idx,matchLen) + QString::fromLatin1("</a>");
            idx+=matchLen-1;
            lastReplacement=idx;
            break;
          }
        }
        result+=text[idx];
        break;

      case '_' :
      case '/' :
      case '*' :
        if(beautification) {
          regExp=QString("^\\%1[^\\s%2]+\\%3").arg(text[idx]).arg(text[idx]).arg(text[idx]);
          if (regExp.match(text,idx,&matchLen)!=-1) {
            if (( matchLen > 3 ) &&
                ((idx==0)||text[idx-1].isSpace()||(text[idx-1] == '(')) &&
                ((idx+matchLen==len)||text[idx+matchLen].isSpace()||(text[idx+matchLen]==',')||
                 (text[idx+matchLen]=='.')||(text[idx+matchLen]==')'))) {
              switch (text[idx].latin1()) {
                case '_' :
                  result+=QString("<u>%1</u>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;
                case '/' :
                  result+=QString("<i>%1</i>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;
                case '*' :
                  result+=QString("<b>%1</b>").arg(toHtmlString(text.mid(idx+1,matchLen-2),parseURLs,beautification));
                  break;
              }
              idx+=matchLen-1;
              lastReplacement=idx;
              break;
            }
          }
        }
        result+=text[idx];
        break;

      default  : result+=text[idx];
    }
  }
  return result;
}


void KNArticleWidget::openURL(const QString &url)
{
  if(url.isEmpty()) return;

  if (knGlobals.cfgManager->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTkonq)
    kapp->invokeBrowser(url);
  else if (knGlobals.cfgManager->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTnetscape){
    KProcess proc;
    proc << "netscape";

    struct stat info;      // QFileInfo is unable to handle netscape's broken symlink
    if (lstat((QDir::homeDirPath()+"/.netscape/lock").local8Bit(),&info)==0)
      proc << "-remote" << QString("openURL(%1)").arg(url);
    else
      proc << url;

    proc.start(KProcess::DontCare);
  }
  else if (knGlobals.cfgManager->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTmozilla){
    KProcess proc;
    proc << "run-mozilla.sh";
    proc << "mozilla-bin";
    proc << url;
    proc.start(KProcess::DontCare);
  }
  else if (knGlobals.cfgManager->readNewsViewer()->browser()==KNConfig::ReadNewsViewer::BTopera){
    KProcess proc;
    proc << "opera";
    proc << QString("-page=%1").arg(url);
    proc << url;
    proc.start(KProcess::DontCare);
  } else {
    KProcess proc;

    QStringList command = QStringList::split(' ',knGlobals.cfgManager->readNewsViewer()->browserCommand());
    bool urlAdded=false;
    for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
      if ((*it).contains("%u")) {
        (*it).replace(QRegExp("%u"),url);
        urlAdded=true;
      }
      proc << (*it);
    }
    if(!urlAdded)    // no %u in the browser command
      proc << url;

    proc.start(KProcess::DontCare);
  }
}


void KNArticleWidget::saveAttachment(int id)
{
  KNMimeContent *a=a_tt->at(id);

  if(a)
    knGlobals.artManager->saveContentToFile(a,this);
  else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}


void KNArticleWidget::openAttachment(int id)
{
 KNMimeContent *a=a_tt->at(id);

 if(a)
   knGlobals.artManager->openContent(a);
 else KMessageBox::error(this, i18n("Internal error: Malformed identifier!"));
}


bool KNArticleWidget::inlinePossible(KNMimeContent *c)
{
  KNHeaders::ContentType *ct=c->contentType();
  return ( ct->isText() || ct->isImage() );
}


void KNArticleWidget::showBlankPage()
{
  kdDebug(5003) << "KNArticleWidget::showBlankPage()" << endl;

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  setText(QString::null);

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  a_rticle=0;
  delete a_tt;
  a_tt=0;
  h_tmlDone=false;
  a_ctSave->setEnabled(false);
  a_ctPrint->setEnabled(false);
  a_ctCopy->setEnabled(false); //probaly not neede, but who knows ;-)
  a_ctSelAll->setEnabled(false);
  a_ctReply->setEnabled(false);
  a_ctRemail->setEnabled(false);
  a_ctForward->setEnabled(false);
  a_ctCancel->setEnabled(false);
  a_ctSupersede->setEnabled(false);
  a_ctVerify->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
  a_ctSetCharset->setEnabled(false);
  a_ctSetCharsetKeyb->setEnabled(false);
  a_ctViewSource->setEnabled(false);
}


void KNArticleWidget::showErrorMessage(const QString &s)
{
  setFont(a_ctToggleFixedFont->isChecked()?
                           knGlobals.cfgManager->appearance()->articleFixedFont()
                         : knGlobals.cfgManager->appearance()->articleFont());  // switch back from possible obscure charsets

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  QString errMsg=s;
  errMsg.replace(QRegExp("\n"),QString("<br>"));  // error messages can contain html-links, but are plain text otherwise
  QString msg="<qt>"+i18n("<b><font size=+1 color=red>An error occured!</font></b><hr><br>")+errMsg+"</qt>";
  setText(msg);

  // mark article as read, typically the article is expired on the server, so its
  // impossible to read it later anyway.
  if(knGlobals.cfgManager->readNewsGeneral()->autoMark() &&
     a_rticle && a_rticle->type()==KNMimeBase::ATremote && !a_rticle->isOrphant()) {
    KNRemoteArticle::List l;
    l.append((static_cast<KNRemoteArticle*>(a_rticle)));
    knGlobals.artManager->setRead(l, true);
  }

  a_rticle=0;
  delete a_tt;
  a_tt=0;
  h_tmlDone=false;
  a_ctSave->setEnabled(false);
  a_ctPrint->setEnabled(false);
  a_ctSelAll->setEnabled(true);
  a_ctReply->setEnabled(false);
  a_ctRemail->setEnabled(false);
  a_ctForward->setEnabled(false);
  a_ctCancel->setEnabled(false);
  a_ctSupersede->setEnabled(false);
  a_ctVerify->setEnabled(false);
  a_ctToggleFullHdrs->setEnabled(false);
  a_ctToggleRot13->setEnabled(false);
  a_ctSetCharset->setEnabled(false);
  a_ctSetCharsetKeyb->setEnabled(false);
  a_ctViewSource->setEnabled(false);
}


void KNArticleWidget::updateContents()
{
  if(a_rticle && a_rticle->hasContent())
    createHtmlPage();
  else
    showBlankPage();
}


void KNArticleWidget::setArticle(KNArticle *a)
{
  if(a_rticle && a_rticle->isOrphant())
    delete a_rticle; //don't leak orphant articles

  a_rticle=a;
  h_tmlDone=false;
  r_ot13=false;
  a_ctToggleRot13->setChecked(false);

  t_imer->stop();

  if(!a)
    showBlankPage();
  else {
    if(a->hasContent()) //article is already loaded => just show it
      createHtmlPage();
    else {
      if( !knGlobals.artManager->loadArticle(a_rticle) )
        articleLoadError( a, i18n("Unable to load the article!") );
      else if(a->hasContent()) // try again...
        createHtmlPage();
    }
  }
}


void KNArticleWidget::slotKeyUp()
{
  scrollBy( 0, -10 );
}


void KNArticleWidget::slotKeyDown()
{
  scrollBy( 0, 10 );
}


void KNArticleWidget::slotKeyPrior()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, -visibleHeight()+offs);
}


void KNArticleWidget::slotKeyNext()
{
  int offs = (visibleHeight() < 30) ? visibleHeight() : 30;
  scrollBy( 0, visibleHeight()-offs);
}


void KNArticleWidget::processJob(KNJobData *j)
{
  if(j->type()==KNJobData::JTfetchSource) {
    KNRemoteArticle *a=static_cast<KNRemoteArticle*>(j->data());
    if(!j->canceled()) {
      QString html;
      if (!j->success())
        html= i18n("<b><font size=+1 color=red>An error occured!</font></b><hr><br>")+j->errorString();
      else
        html= QString("%1<br>%2").arg(toHtmlString(a->head(),false,false)).arg(toHtmlString(a->body(),false,false));

      new KNSourceViewWindow(html);
    }
    delete j;
    delete a;
  }
  else
    delete j;
}


void KNArticleWidget::createHtmlPage()
{
  kdDebug(5003) << "KNArticleWidget::createHtmlPage()" << endl;

  if(!a_rticle) {
    showBlankPage();
    return;
  }

  if(!a_rticle->hasContent()) {
    showErrorMessage(i18n("the article contains no data"));
    return;
  }

  if ((f_orceCS!=a_rticle->forceDefaultCS())||
      (f_orceCS && (a_rticle->defaultCharset()!=o_verrideCS))) {
    a_rticle->setDefaultCharset(o_verrideCS);
    a_rticle->setForceDefaultCS(f_orceCS);
  }

  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();
  KNConfig::ReadNewsViewer *rnv=knGlobals.cfgManager->readNewsViewer();

  delete f_actory;                          // purge old image data
  f_actory = new QMimeSourceFactory();
  setMimeSourceFactory(f_actory);

  //----------------------------------- <Header> ---------------------------------------

  QString html, hLine;
  if (rnv->showHeaderDecoration())
    html=QString("<qt><table width=\"100%\" cellpadding=0 cellspacing=1><tr><td width=40 bgcolor=\"%1\"></td><td width=\"1%\"><headerblock><table cellpadding=0 cellspacing=0>")
         .arg(app->headerDecoHexcode());
  else
    html="<qt><headerblock><table width=\"100%\" cellpadding=0 cellspacing=0><tr><td><table cellpadding=0 cellspacing=0>";

  if(a_ctToggleFullHdrs->isChecked()) {
    QCString head = a_rticle->head();
    KNHeaders::Generic *header=0;

    while(!head.isEmpty()) {
      header = a_rticle->getNextHeader(head);
      if (header) {
        html+=QString("<tr><td align=right><b>%1</b></td><td width=\"100%\">%2</td></tr>")
                      .arg(toHtmlString(header->type())+":")
                      .arg(toHtmlString(header->asUnicodeString(),true));
        delete header;
      }
    }
  }
  else {
    KNHeaders::Base *hb;
    KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.cfgManager->displayedHeaders()->iterator();
    for(; it.current(); ++it) {
      dh=it.current();
      hb=a_rticle->getHeaderByType(dh->header().latin1());
      if(!hb) continue; //header not found

      if(dh->hasName()) {
        html += QString("<tr><td align=right>") + i18n("%1%2:%3")
        .arg(dh->nameOpenTag()).arg(toHtmlString(dh->translatedName()))
        .arg(dh->nameCloseTag()) + "</td><td width=\"100%\">";
      }
      else
        html+="<tr><td colspan=2>";

      html+=dh->headerOpenTag();

      if(hb->is("From")) {
        html+=QString("<a href=\"internal:author\">%1</a>")
                .arg(toHtmlString(hb->asUnicodeString()));
      } else if(hb->is("Date")) {
        KNHeaders::Date *date=static_cast<KNHeaders::Date*>(hb);
        html+=toHtmlString(KGlobal::locale()->formatDateTime(date->qdt(), false, true));
      }
      else
        html+=toHtmlString(hb->asUnicodeString(),true);

      html += dh->headerCloseTag()+"</td></tr>";
    }
  }

  if (rnv->showHeaderDecoration()) {    // no decorations <=> no references
    html+=QString("</table></headerblock></td></tr><tr><td colspan=2 bgcolor=\"%1\"><headerblock>")
          .arg(app->headerDecoHexcode());

    //References
    KNHeaders::References *refs=a_rticle->references(false);
    if(a_rticle->type()==KNMimeBase::ATremote && refs) {
      int refCnt=refs->count(), i=1;
      QCString id = refs->first();
      id = id.mid(1, id.length()-2);  // remove <>
      html+=QString("<b>%1</b>").arg(i18n("References:"));

      while (i <= refCnt) {
        html+=QString(" <a href=\"news:%1\">%2</a>").arg(id).arg(i);
        id = refs->next();
        id = id.mid(1, id.length()-2);  // remove <>
        i++;
      }
    }
    else html+=i18n("no references");

    html+="</headerblock></td></tr>";
  }

  KNMimeContent *text=a_rticle->textContent();
  if(text) {
    if(!canDecode8BitText(text->contentType()->charset())) {
      if (rnv->showHeaderDecoration())
        html+=QString("<tr><td colspan=3 bgcolor=red><font color=black><headerblock>%1</headerblock></font></td></tr>")
              .arg(i18n("Unknown charset! Default charset is used instead."));
      else
        html+=QString("<tr><td colspan=2 bgcolor=red><font color=black>%1</font></td></tr>")
              .arg(i18n("Unknown charset! Default charset is used instead."));

      kdDebug(5003) << "KNArticleWidget::createHtmlPage() : unknown charset = " << text->contentType()->charset() << " not available!" << endl;
      setFont(a_ctToggleFixedFont->isChecked()? app->articleFixedFont():app->articleFont());
    }
    else {
      QFont f=(a_ctToggleFixedFont->isChecked()? app->articleFixedFont():app->articleFont());
      if (!app->useFontsForAllCS())
        KGlobal::charsets()->setQFont(f, KGlobal::charsets()->charsetForEncoding(text->contentType()->charset()));
      setFont(f);
    }
  }
  else
    setFont(a_ctToggleFixedFont->isChecked()? app->articleFixedFont():app->articleFont());

  //kdDebug(5003) << "KNArticleWidget::createHtmlPage() : font-family = " << font().family() << endl;
  //kdDebug(5003) << "KNArticleWidget::createHtmlPage() : font-charset = " << (int)(font().charSet()) << endl;
  //kdDebug(5003) << "KNArticleWidget::createHtmlPage() : article-charset = " << text->contentType()->charset() << endl;

  if (rnv->showHeaderDecoration())
    html+="</table>";
  else
    html+="</table></tr></td></table></headerblock>";

  //----------------------------------- </Header> --------------------------------------


  //------------------------------------- <Body> ---------------------------------------

  // if the article is pgp signed and the user asked for verifying the
  // signature, we show a nice header:
  if ( a_rticle->type() == KNMimeBase::ATremote ) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rticle);
    KNpgp *pgp = knGlobals.pgp;

    if (knGlobals.cfgManager->readNewsGeneral()->autoCheckPgpSigs() || ra->isPgpSigned()) {
      pgp->setMessage(ra->body());
      html += "<p>";
      if (!pgp->isSigned()) {
        if (!knGlobals.cfgManager->readNewsGeneral()->autoCheckPgpSigs())
          html += "<b>" + i18n("Cannot find a signature in this message!") + "</b>";
      }
      else {
        QString sig;
        if (pgp->goodSignature())
          sig = i18n("Message was signed by");
        else
          sig = i18n("Warning: Bad signature from");

        QString sdata=pgp->signedBy();
        QString signer = sdata;

        if (sdata.contains(QRegExp("unknown key ID")))
        {
          sdata.replace(QRegExp("unknown key ID"), i18n("unknown key ID"));
          html += QString("<b>%1 %2</b><br>").arg(sig).arg(toHtmlString(sdata,false,false,false));
        }
        else {
          html += QString("<b>%1 <a href=\"mailto:%2\">%3</a></b><br>")
                     .arg(sig).arg(signer).arg(toHtmlString(sdata,false,false,false));
        }
      }
      html += "</p>";
    }
  }

  KNHeaders::ContentType *ct=a_rticle->contentType();

  //Attachments
  if(!text || ct->isMultipart()) {
    if(a_tt) a_tt->clear();
    else {
      a_tt=new KNMimeContent::List;
      a_tt->setAutoDelete(false);
    }

    a_rticle->attachments(a_tt, rnv->showAlternativeContents());
  } else {
    delete a_tt;
    a_tt=0;
  }

  //Partial message
  if(ct->isPartial()) {
    html+=i18n("<br><bodyblock><b>This article has the Mime-Type &quot;message/partial&quot;, which KNode cannot handle yet.<br>Meanwhile you can save the article as a text-file and reassemble it by hand.</b></bodyblock></qt>");
    setText(html);
    h_tmlDone=true;

    //enable actions
    a_ctReply->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
    a_ctRemail->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
    a_ctSave->setEnabled(true);
    a_ctPrint->setEnabled(true);
    a_ctSelAll->setEnabled(true);
    a_ctToggleFullHdrs->setEnabled(true);
    a_ctSetCharset->setEnabled(true);
    a_ctSetCharsetKeyb->setEnabled(true);
    a_ctViewSource->setEnabled(true);
    return;
  }

  //body text
  if(text && text->hasContent()) {
    html+="<bodyblock>";
    if(text->contentType()->isHTMLText()) {
      QString htmlTxt;
      text->decodedText(htmlTxt, true);
      html+=htmlTxt+"</bodyblock>";
    }
    else {
      QChar firstChar;
      int oldLevel=0, newLevel=0;
      unsigned int idx=0;
      bool isSig=false;
      QStringList lines;
      QString line;
      text->decodedText(lines, true);
      QString quoteChars = rnv->quoteCharacters().simplifyWhiteSpace();
      if (quoteChars.isEmpty()) quoteChars = ">";

      for(QStringList::Iterator it=lines.begin(); it!=lines.end(); ++it) {
        line=(*it);
        if(!line.isEmpty()) {
          if(!isSig && line=="-- ") {
            isSig=true;
            if(newLevel>0) {
              newLevel=0;
              html+="</font>";
            }
            if(rnv->showSignature()) {
              html+="<hr size=2>";
              continue;
            }
            else break;
          }
          if(!isSig) {
            idx=0;
            oldLevel=newLevel;
            newLevel=0;
            firstChar=line[idx];
            while(idx < line.length()) {
              firstChar=line[idx];
              if(firstChar.isSpace()) idx++;
              else if(quoteChars.find(firstChar)!=-1) { idx++; newLevel++; }
              else break;
            }

            if(newLevel!=oldLevel) {
              if(newLevel==0) html+="</font>";
              else {
                if(newLevel>=3) newLevel=3;
                if(oldLevel>0) html+="</font>";
                html+=QString("<font color=\"%1\">").arg(app->quotedTextHexcode(newLevel-1));
              }
            }
          }
          html+=toHtmlString(line,true,true,true)+"<br>";
        }
        else
          html+="<br>";
      }
      if(newLevel>0) html+="</font>";
    }

    html+="</bodyblock>";
  }

  //------------------------------------- </Body> --------------------------------------


  //----------------------------------  <Attachments> ----------------------------------

  if(a_tt) {
    int attCnt=0;
    QString path;
    if(!a_tt->isEmpty()) {
      html+="<table border width=\"100%\">";
      html+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>")
                    .arg(i18n("name")).arg(i18n("mime-type")).arg(i18n("description"));

      for(KNMimeContent *var=a_tt->first(); var; var=a_tt->next()) {
        ct=var->contentType();
        html+=QString("<tr><td align=center><a href=\"internal:att=%1\">%2</a></td><td align=center>%3</td><td align=center>%4</td></tr>")
              .arg(attCnt)
              .arg((ct->name().isEmpty())? i18n("unnamed"):ct->name())
              .arg(ct->mimeType())
              .arg(toHtmlString(var->contentDescription()->asUnicodeString()));

        if(rnv->showAttachmentsInline() && inlinePossible(var)) {
          html+="<tr><td colspan=3>";
          if(ct->isImage()) { //image
            path=QString::number(attCnt)+"_"+QString::number((int)(a_rticle));
            f_actory->setData(path,new KNMimeSource(var->decodedContent(),ct->mimeType()));
            html+=QString("<a href=\"internal:att=%1\"><img src=\"%2\"></a>").arg(attCnt).arg(path);
          }
          else { //text
            QString tmp;
            var->decodedText(tmp);
            if(ct->isHTMLText())
              html+=tmp;
            else
              html+="<txt_attachment>"+toHtmlString(tmp,true)+"</txt_attachment>";
          }
          html+="</td></tr>";
        }
        attCnt++;
      }
      html+="</table>";
    }
  }

  //----------------------------------  </Attachments> ---------------------------------

  //display html
  html+="</qt>";
  setText(html);
  h_tmlDone=true;

  //enable actions
  a_ctSave->setEnabled(true);
  a_ctPrint->setEnabled(true);
  a_ctSelAll->setEnabled(true);

  a_ctReply->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
  a_ctRemail->setEnabled(a_rticle->type()==KNMimeBase::ATremote);
  a_ctForward->setEnabled(true);
  a_ctCancel->setEnabled( (a_rticle->type()==KNMimeBase::ATremote) ||
                          (a_rticle->collection()==knGlobals.folManager->sent()));
  a_ctSupersede->setEnabled( (a_rticle->type()==KNMimeBase::ATremote) ||
                             (a_rticle->collection()==knGlobals.folManager->sent()));
  a_ctVerify->setEnabled(true);
  a_ctToggleFullHdrs->setEnabled(true);
  a_ctToggleRot13->setEnabled(true);
  a_ctSetCharset->setEnabled(true);
  a_ctSetCharsetKeyb->setEnabled(true);
  a_ctViewSource->setEnabled(true);

  //start automark-timer
  if(a_rticle->type()==KNMimeBase::ATremote && knGlobals.cfgManager->readNewsGeneral()->autoMark())
    t_imer->start( (knGlobals.cfgManager->readNewsGeneral()->autoMarkSeconds()*1000), true);
}


void KNArticleWidget::setSource(const QString &s)
{
  if(!s.isEmpty())
    anchorClicked(s);
}


void KNArticleWidget::anchorClicked(const QString &a, ButtonState button, const QPoint *p)
{
  anchorType type=ATunknown;
  QString target;

  if(a.left(15)=="internal:author") {
    type=ATauthor;
  }
  else if(a.left(13)=="internal:att=") {
    target=a.mid(13, a.length()-13);
    type=ATattachment;
  }
  else if(a.left(7).lower()=="http://" || a.left(8).lower() == "https://" ||
          a.left(6).lower()=="ftp://") {
    target=a;
    type=ATurl;
  }
  else if(a.left(5).lower()=="news:") {
    target=a;
    type=ATnews;
  }
  else if(a.left(7)=="mailto:") {
    target=a.mid(7, a.length()-7);
    type=ATmailto;
  }
  else if(a.left(9)=="addrOrId:") {
    target=a.mid(9, a.length()-9);

    if ((button==LeftButton)||(button==MidButton)) {
      if (a_rticle->collection()->type()!=KNArticleCollection::CTgroup)
        type=ATmailto;
      else {
        switch (KMessageBox::warningYesNoCancel(this, i18n("<qt>Do you want treat the selected text as an <b>email address</b> or a <b>message-id</b>?</qt>"),
                                                i18n("Address or ID"), i18n("&Email"), i18n("&Message-Id"))) {
          case KMessageBox::Yes:  type=ATmailto;
                                  break;
          case KMessageBox::No:   type=ATmsgid;
                                  break;
          default:                type=ATunknown;
        }
      }
    } else {
      type=ATunknown;
    }
  }

  if((button==LeftButton)||(button==MidButton)) {
    KNGroup *g;
    KNRemoteArticle *a;
    KNArticleWindow *awin;
    KNHeaders::AddressField adr(a_rticle);

    switch(type) {
      case ATauthor:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        knGlobals.artFactory->createMail(a_rticle->from());
      break;
      case ATmsgid:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : message-id " << target << endl;

        if (a_rticle->collection()->type()!=KNArticleCollection::CTgroup)  // we need a group for doing network stuff
          break;

        g=static_cast<KNGroup*>(a_rticle->collection());
        if (target.find(QRegExp("<*>",false,true))==-1)   // add "<>" when necessary
          target = QString("<%1>").arg(target);

        if(!KNArticleWindow::raiseWindowForArticle(target.latin1())) { //article not yet opened
          a=g->byMessageId(target.latin1());
          if(a) {
            //article found in KNGroup
            awin=new KNArticleWindow(a);
            awin->show();
          }
          else {
            //article not in local group => create a new (orphant) article.
            //It will be deleted by the displaying widget/window
            a=new KNRemoteArticle(g); //we need "g" to access the nntp-account
            a->messageID()->from7BitString(target.latin1());
            awin=new KNArticleWindow(a);
            awin->show();
          }
        }
      break;
      case ATattachment:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : attachment " << target << endl;
        if(knGlobals.cfgManager->readNewsViewer()->openAttachmentsOnClick())
          openAttachment(target.toInt());
        else
          saveAttachment(target.toInt());
      break;
      case ATurl:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : url " << target << endl;;
        openURL(target);
      break;
      case ATnews:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : news-url " << target << endl;;
        knGlobals.top->openURL(target);
      break;
      case ATmailto:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : mailto author" << endl;
        adr.fromUnicodeString(target,"");
        knGlobals.artFactory->createMail(&adr);
      break;
      default:
        kdDebug(5003) << "KNArticleWidget::anchorClicked() : unknown" << endl;
      break;
    }
  }
  else {

    if(type==ATattachment) {
      kdDebug(5003) << "KNArticleWidget::anchorClicked() : popup for attachment " << target << endl;
      switch(a_ttPopup->exec(*p)) {
        case PUP_OPEN:
          openAttachment(target.toInt());
        break;
        case PUP_SAVE:
          saveAttachment(target.toInt());
        break;
      }
    }

    else if(type==ATurl) {
      kdDebug(5003) << "KNArticleWidget::anchorClicked() : popup for url " << target << endl;
      switch(u_rlPopup->exec(*p)) {
        case PUP_OPEN:
          openURL(target);
        break;
        case PUP_COPYURL:
          QApplication::clipboard()->setText(target);
        break;
      }
    }
  }
}


void KNArticleWidget::slotSave()
{
  kdDebug(5003) << "KNArticleWidget::slotSave()" << endl;
  if(a_rticle)
    knGlobals.artManager->saveArticleToFile(a_rticle,this);
}


void KNArticleWidget::slotPrint()
{
  kdDebug(5003) << "KNArticleWidget::slotPrint()" << endl;
  KPrinter *printer=new KPrinter();

  if(printer->setup(this)) {

    QPaintDeviceMetrics metrics(printer);
    QPainter p;

    const int margin=20;
    int yPos=0;
    KNHeaders::Base *hb=0;
    QString text;
    QString hdr;

    p.begin(printer);
    p.setFont( QFont(font().family(), 12, QFont::Bold) );
    QFontMetrics fm=p.fontMetrics();

    KNDisplayedHeader *dh;
    KNConfig::DisplayedHeaders::Iterator it=knGlobals.cfgManager->displayedHeaders()->iterator();
    dh=it.current();
    while(dh) {
      hb=a_rticle->getHeaderByType(dh->header().latin1());

      if(hb && !hb->isEmpty()) {
        if(dh->hasName())
          text=QString("%1: %2").arg(dh->translatedName()).arg(hb->asUnicodeString());
        else
          text=hb->asUnicodeString();

        p.drawText( 10, yPos+margin,  metrics.width(),
                  fm.lineSpacing(), ExpandTabs | DontClip,
                  text );

        if( (dh=++it)!=0 )
          yPos+=fm.lineSpacing();
      }
      else
        dh=++it;

    }

    yPos+=fm.lineSpacing()+10;


    QPen pen(QColor(0,0,0), 2);
    p.setPen(pen);

    p.drawLine(10, yPos+margin, metrics.width(), yPos+margin);
    yPos+=2*fm.lineSpacing();

    p.setFont( QFont(font().family(), 10, QFont::Normal) );
    fm=p.fontMetrics();

    QStringList lines;
    KNMimeContent *txt=a_rticle->textContent();

    if(txt) {
      txt->decodedText(lines, true);
      for(QStringList::Iterator it=lines.begin(); it!=lines.end(); ++it) {

        if(yPos+margin > metrics.height()) {
          printer->newPage();
          yPos=0;
        }

        text=(*it);
        p.drawText( 10, yPos+margin,  metrics.width(),
                    fm.lineSpacing(), ExpandTabs | DontClip,
                    text );

        yPos+=fm.lineSpacing();
      }
    }

    p.end();
  }

  delete printer;
}


void KNArticleWidget::slotSelectAll()
{
  kdDebug(5003) << "KNArticleWidget::slotSelectAll()" << endl;
  selectAll();
  a_ctCopy->setEnabled(true);
}


void KNArticleWidget::slotReply()
{
  kdDebug(5003) << "KNArticleWidget::slotReply()" << endl;
  if(a_rticle && a_rticle->type()==KNMimeBase::ATremote)
    knGlobals.artFactory->createReply(static_cast<KNRemoteArticle*>(a_rticle), selectedText(), true, false);
}


void KNArticleWidget::slotRemail()
{
  kdDebug(5003) << "KNArticleWidget::slotRemail()" << endl;
  if(a_rticle && a_rticle->type()==KNMimeBase::ATremote)
    knGlobals.artFactory->createReply(static_cast<KNRemoteArticle*>(a_rticle), selectedText(), false, true);
}


void KNArticleWidget::slotForward()
{
  kdDebug(5003) << "KNArticleWidget::slotForward()" << endl;
  knGlobals.artFactory->createForward(a_rticle);
}


void KNArticleWidget::slotCancel()
{
  kdDebug(5003) << "KNArticleWidget::slotCancel()" << endl;
  knGlobals.artFactory->createCancel(a_rticle);
}


void KNArticleWidget::slotSupersede()
{
  kdDebug(5003) << "KNArticleWidget::slotSupersede()" << endl;
  knGlobals.artFactory->createSupersede(a_rticle);
}


void KNArticleWidget::slotToggleFullHdrs()
{
  kdDebug(5003) << "KNArticleWidget::slotToggleFullHdrs()" << endl;

  // ok, this is a hack
  if (knGlobals.artWidget == this)
    knGlobals.cfgManager->readNewsViewer()->setShowFullHdrs(!knGlobals.cfgManager->readNewsViewer()->showFullHdrs());
  updateContents();
}


void KNArticleWidget::slotToggleRot13()
{
  r_ot13=!r_ot13;
  updateContents();
}


void KNArticleWidget::slotToggleFixedFont()
{
  // ok, this is a hack
  if (knGlobals.artWidget == this)
    knGlobals.cfgManager->readNewsViewer()->setUseFixedFont(!knGlobals.cfgManager->readNewsViewer()->useFixedFont());
  updateContents();
}


void KNArticleWidget::slotSetCharset(const QString &s)
{
  if(s.isEmpty())
    return;

  if (s == i18n("Automatic")) {
    f_orceCS=false;
    o_verrideCS=KNHeaders::Latin1;
  } else {
    f_orceCS=true;
    o_verrideCS=s.latin1();
  }

  if (a_rticle && a_rticle->hasContent()) {
    a_rticle->setDefaultCharset(o_verrideCS);  // the article will choose the correct default,
    a_rticle->setForceDefaultCS(f_orceCS);     // when we disable the overdrive
    createHtmlPage();
  }
}


void KNArticleWidget::slotSetCharsetKeyboard()
{
  int newCS = KNHelper::selectDialog(this, i18n("Select Charset"), a_ctSetCharset->items(), a_ctSetCharset->currentItem());
  if (newCS != -1) {
    a_ctSetCharset->setCurrentItem(newCS);
    slotSetCharset(*(a_ctSetCharset->items().at(newCS)));
  }
}


void KNArticleWidget::slotViewSource()
{
  kdDebug(5003) << "KNArticleWidget::slotViewSource()" << endl;
  if (a_rticle && a_rticle->type()==KNMimeBase::ATlocal && a_rticle->hasContent()) {
    new KNSourceViewWindow(toHtmlString(a_rticle->encodedContent(false),false,false));
  } else {
    if (a_rticle && a_rticle->type()==KNMimeBase::ATremote) {
      KNGroup *g=static_cast<KNGroup*>(a_rticle->collection());
      KNRemoteArticle *a=new KNRemoteArticle(g); //we need "g" to access the nntp-account
      a->messageID(true)->from7BitString(a_rticle->messageID()->as7BitString(false));
      a->lines(true)->from7BitString(a_rticle->lines(true)->as7BitString(false));
      a->setArticleNumber(static_cast<KNRemoteArticle*>(a_rticle)->articleNumber());
      emitJob( new KNJobData(KNJobData::JTfetchSource, this, g->account(), a));
    }
  }
}


void KNArticleWidget::slotTimeout()
{
  if(a_rticle && a_rticle->type()==KNMimeBase::ATremote && !a_rticle->isOrphant()) {
    KNRemoteArticle::List l;
    l.append((static_cast<KNRemoteArticle*>(a_rticle)));
    knGlobals.artManager->setRead(l, true);
  }
}


void KNArticleWidget::slotVerify()
{
  //  knGlobals.artManager->verifyPGPSignature(a_rticle);
  //create a PGP object and check if the posting is signed
  if (a_rticle->type() == KNMimeBase::ATremote) {
    KNRemoteArticle *ra = static_cast<KNRemoteArticle*>(a_rticle);
    ra->setPgpSigned(true);
    updateContents();
  }
}


//--------------------------------------------------------------------------------------


QList<KNArticleWidget> KNArticleWidget::i_nstances;

void KNArticleWidget::configChanged()
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    i->applyConfig();
}


bool KNArticleWidget::articleVisible(KNArticle *a)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      return true;
  return false;
}


void KNArticleWidget::articleRemoved(KNArticle *a)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      i->showBlankPage();
}


void KNArticleWidget::articleChanged(KNArticle *a)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      i->updateContents();
}


void KNArticleWidget::articleLoadError(KNArticle *a, const QString &error)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(a==i->article())
      i->showErrorMessage(error);
}


void KNArticleWidget::collectionRemoved(KNArticleCollection *c)
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    if(i->article() && i->article()->collection()==c)
      i->showBlankPage();
}


void KNArticleWidget::cleanup()
{
  for(KNArticleWidget *i=i_nstances.first(); i; i=i_nstances.next())
    i->setArticle(0); //delete orphant articles => avoid crash in destructor
}

//=============================================================================================================


// some standard headers
static const char *predef[] = { "Approved","Content-Transfer-Encoding","Content-Type","Control","Date","Distribution",
                                "Expires","Followup-To","From","Lines","Mail-Copies-To","Message-ID","Mime-Version","NNTP-Posting-Host",
                                "Newsgroups","Organization","Path","References","Reply-To", "Sender","Subject",
                                "Supersedes","To", "User-Agent","X-Mailer","X-Newsreader","X-No-Archive","XRef",0 };

// default display names KNode uses
static const char *disp[] = { "Groups", 0 };

void dummyHeader()
{
  i18n("collection of article headers","Approved");
  i18n("collection of article headers","Content-Transfer-Encoding");
  i18n("collection of article headers","Content-Type");
  i18n("collection of article headers","Control");
  i18n("collection of article headers","Date");
  i18n("collection of article headers","Distribution");
  i18n("collection of article headers","Expires");
  i18n("collection of article headers","Followup-To");
  i18n("collection of article headers","From");
  i18n("collection of article headers","Lines");
  i18n("collection of article headers","Mail-Copies-To");
  i18n("collection of article headers","Message-ID");
  i18n("collection of article headers","Mime-Version");
  i18n("collection of article headers","NNTP-Posting-Host");
  i18n("collection of article headers","Newsgroups");
  i18n("collection of article headers","Organization");
  i18n("collection of article headers","Path");
  i18n("collection of article headers","References");
  i18n("collection of article headers","Reply-To");
  i18n("collection of article headers","Sender");
  i18n("collection of article headers","Subject");
  i18n("collection of article headers","Supersedes");
  i18n("collection of article headers","To");
  i18n("collection of article headers","User-Agent");
  i18n("collection of article headers","X-Mailer");
  i18n("collection of article headers","X-Newsreader");
  i18n("collection of article headers","X-No-Archive");
  i18n("collection of article headers","XRef");

  i18n("collection of article headers","Groups");
}


//=============================================================================================================


KNDisplayedHeader::KNDisplayedHeader()
 : t_ranslateName(true)
{
  f_lags.fill(false, 8);
  f_lags[1] = true;   // header name bold by default
}


KNDisplayedHeader::~KNDisplayedHeader()
{
}


// some common headers
const char** KNDisplayedHeader::predefs()
{
  return predef;
}


// *trys* to translate the name
QString KNDisplayedHeader::translatedName()
{
  if (t_ranslateName) {
    // major hack alert !!!
    if (!n_ame.isEmpty()) {
      if (i18n("collection of article headers",n_ame.local8Bit())!=n_ame.local8Bit().data())    // try to guess if this english or not
        return i18n("collection of article headers",n_ame.local8Bit());
      else
        return n_ame;
    } else
      return QString::null;
  } else
    return n_ame;
}


// *trys* to retranslate the name to english
void KNDisplayedHeader::setTranslatedName(const QString &s)
{
  bool retranslated = false;
  for (const char **c=predef;(*c)!=0;c++) {  // ok, first the standard header names
    if (s==i18n("collection of article headers",*c)) {
      n_ame = QString::fromLatin1(*c);
      retranslated = true;
      break;
    }
  }

  if (!retranslated) {
    for (const char **c=disp;(*c)!=0;c++)   // now our standard display names
      if (s==i18n("collection of article headers",*c)) {
        n_ame = QString::fromLatin1(*c);
        retranslated = true;
        break;
      }
  }

  if (!retranslated) {      // ok, we give up and store the maybe non-english string
    n_ame = s;
    t_ranslateName = false;  // and don't try to translate it, so a german user *can* use the original english name
  } else
    t_ranslateName = true;
}


void  KNDisplayedHeader::createTags()
{
  const char *tokens[] = {  "<large>","</large>","<b>","</b>",
                            "<i>","</i>","<u>","</u>" };

  for(int i=0; i<4; i++) t_ags[i]=QString::null;

  if(f_lags.at(0)) {    // <font>
    t_ags[0]=tokens[0];
    t_ags[1]=tokens[1];
  }
  if(f_lags.at(4)) {
    t_ags[2]=tokens[0];
    t_ags[3]=tokens[1];
  }

  if(f_lags.at(1)) {     // <b>
    t_ags[0]+=(tokens[2]);
    t_ags[1].prepend(tokens[3]);
  }
  if(f_lags.at(5)) {
    t_ags[2]+=tokens[2];
    t_ags[3].prepend(tokens[3]);
  }

  if(f_lags.at(2)) {     // <i>
    t_ags[0]+=tokens[4];
    t_ags[1].prepend(tokens[5]);
  }
  if(f_lags.at(6)) {
    t_ags[2]+=tokens[4];
    t_ags[3].prepend(tokens[5]);
  }

  if(f_lags.at(3)) {    // <u>
    t_ags[0]+=tokens[6];
    t_ags[1].prepend(tokens[7]);
  }
  if(f_lags.at(7)) {
    t_ags[2]+=tokens[6];
    t_ags[3].prepend(tokens[7]);
  }
}


//--------------------------------

#include "knarticlewidget.moc"
