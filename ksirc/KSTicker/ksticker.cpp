#include <iostream>
#include <stdlib.h>
#include <qpainter.h>
#include <qframe.h>
#include <qpaintdevice.h>

#include <kfontdialog.h>

#include "ksticker.h"
#include "speeddialog.h"

#include "kspainter.h"

#include <kconfig.h>
#include <kwin.h>
#include <kapp.h>
#include "../config.h"

KSTicker::KSTicker(QWidget * parent, const char * name, WFlags f)  
: QFrame(parent, name, f)
{

  pHeight = 1;

  pic = new QPixmap(); // create pic map here, resize it later though.
  //  pic->setBackgroundMode(TransparentMode);

  KConfig *conf = kapp->config();
  conf->setGroup("KSTicker");
  bScrollConstantly = conf->readNumEntry("ScollConst", TRUE);

  bAtEnd = FALSE;
  ourFont = QFont("Courier");
  setFont(ourFont);
  // ring = "Hi";
  ring = "";
  SInfo *si = new SInfo;
  si->length = 0;
  StrInfo.append(si);
  StrInfo.setAutoDelete(TRUE);
  setMinimumSize(100, 10);
  setFixedHeight((fontMetrics().height()+fontMetrics().descent()*2)*pHeight);
  descent =  fontMetrics().descent();
  onechar = fontMetrics().width("X");

  pic->resize(width() + onechar, height());
  pic->fill(backgroundColor());

  tickStep = 2;
  cOffset = 0;

  tickRate = 30;

  currentChar = 0;
  chars = this->width() / onechar;
  StrInfo.setAutoDelete( TRUE );

  popup = new QPopupMenu();
  popup->insertItem("Font...", this, SLOT(fontSelector()));
  popup->insertItem("Scroll Rate...", this, SLOT(scrollRate()));
  iScrollItem = popup->insertItem("Scroll Constantly", this, SLOT(scrollConstantly()));
  popup->setItemChecked(iScrollItem, bScrollConstantly);
  popup->insertSeparator();
  popup->insertItem("Hide...", this, SIGNAL(doubleClick()));

  display = " ";

  /*
   * Tell KWM to use only minimum decurations
   */
  // ### FIXME: port to kwin
  //KWM::setDecoration(winId(), KWM::tinyDecoration);
  KWin::setState(winId(), NET::Override);

  /*
   * Setup basic colours and background status info for ticker.
   */
  bold = FALSE;
  underline = FALSE;
  italics = FALSE;
  defbg = backgroundColor();
  deffg = foregroundColor();
  bg = backgroundColor();
  fg = foregroundColor();

}

KSTicker::~KSTicker() 
{
  killTimers();
  delete pic;
}

void KSTicker::show() 
{
  /*
   * Tell KWM to use only minimum decurations
   */
    // ### FIXME: port to kwin
    //KWM::setDecoration(winId(), KWM::tinyDecoration);
  QSize size = this->size();
  size.setHeight(QFontMetrics(font()).height()+10);
  setFixedHeight(QFontMetrics(font()).height()+10);
  resize(size);

  QFrame::show();
  if(display.length() != 0)
    startTicker();
  currentChar = display.length() + 1;
  repaint(TRUE);
}

void KSTicker::hide() 
{
  killTimers();
  QFrame::hide();
}

void KSTicker::iconify() 
{
  QFrame::showMinimized();
  killTimers();
}

void KSTicker::setString(QString str) 
{
  ring.truncate(0);
  ring = str;
  StrInfo.clear();
  SInfo *si = new SInfo;
  si->length = str.length();
  StrInfo.append(si);
  repaint(TRUE);
  startTicker();
}

void KSTicker::mergeString(QString str) 
{
  str.append("~C");
  ring += str;
  SInfo *si = new SInfo;
  si->length = str.length();
  StrInfo.append(si);
  while((ring.length() > (uint) 2*chars + 10) &&
        ((ring.length() - StrInfo.at(0)->length) > (uint) (chars + chars/2)) &&
        (StrInfo.count() > 1)) {

    if(currentChar < StrInfo.at(0)->length)
      break;

    currentChar -= StrInfo.at(0)->length;

    ring.remove(0, StrInfo.at(0)->length);
    StrInfo.removeFirst();
  }

  if(bScrollConstantly == FALSE)
    startTicker();

  display = ring;


}

void KSTicker::timerEvent(QTimerEvent *) 
{
  if((uint)currentChar >= display.length()){
    if(bScrollConstantly == TRUE){
      display = ring;
      currentChar = 0;
    }
    else{
      stopTicker();
      return;
    }
  }

  bAtEnd = FALSE;
  static BGMode bgmode = TransparentMode;

  bitBlt(pic, -tickStep, 0, pic);
  QPainter p(pic);

  cOffset += tickStep;
  if(cOffset >= onechar){
    int step = 1; // Used to check if we did anything, and hence
    // catch ~c~c type things. Set to 1 to start loop
    while(((display[currentChar] == '~') || (display[currentChar] == 0x03))
          && (step > 0)){
      step = 1; // reset in case it's our second, or more loop.
      const char *text = qstrdup(display.mid(currentChar).ascii());
      char buf[3];
      memset(buf, 0, sizeof(char)*3);

      if((text[step] >= '0') &&
         (text[step] <= '9')) {
        buf[0] = text[step];
        step++;
        if((text[step] >= '0') &&
           (text[step] <= '9')) {
          buf[1] = text[step];
          step++;
        }
        int col = atoi(buf);
        if((col >= 0) || (col <= KSPainter::maxcolour)){
          fg = KSPainter::num2colour[col];
        }
        bg = defbg;
        if(text[step] == ','){
          step++;
          memset(buf, 0, sizeof(char)*3);
          if((text[step] >= '0') &&
             (text[step] <= '9')) {
            buf[0] = text[step];
            step++;
            if((text[step] >= '0') &&
               (text[step] <= '9')) {
              buf[1] = text[step];
              step++;
            }
            int col = atoi(buf);
            if((col >= 0) || (col <= KSPainter::maxcolour)){
              bg = KSPainter::num2colour[col];
              bgmode = OpaqueMode;
            }
          }
        }
        else{
          bgmode = TransparentMode;
        }
        delete [] text;
      }
      else{
        switch(text[step]){
        case 'c':
          fg = deffg;
          bg = defbg;
          step++;
          break;
        case 'C':
          fg = deffg;
          bg = defbg;
          bold = FALSE;
          underline = FALSE;
          italics = FALSE;
          step++;
          break;
        case 'b':
          bold == TRUE ? bold = FALSE : bold = TRUE;
          step++;
          break;
        case 'u':
          underline == TRUE ? underline = FALSE : underline = TRUE;
          step++;
          break;
        case 'i':
          italics == TRUE ? italics = FALSE : italics = TRUE;
          step++;
          break;
        case '~':
          currentChar++; // Move ahead 1, but...
          step = 0;      // Don't loop on next ~.
          break;
        default:
          if(display[currentChar] == 0x03){
            fg = deffg;
            bg = defbg;
          }
          else
            step = 0;
        }
      }
      currentChar += step;
    }
    if((uint)currentChar >= display.length()){ // Bail out if we're
      // at the end of the string.
      return;
    }


    QFont fnt = font();
    fnt.setBold(bold);
    fnt.setUnderline(underline);
    fnt.setItalic(italics);
    p.setFont(fnt);

    p.setPen(fg);
    p.setBackgroundColor(bg);
    p.setBackgroundMode(OpaqueMode);
    p.drawText(this->width() - cOffset + onechar, // remove -onechar.
               this->height() / 4 + p.fontMetrics().height() / 2,
               display.mid(currentChar, 1),
               1);
    currentChar++; // Move ahead to next character.
    cOffset -= onechar; // Set offset back one.
  }
  p.end();
  bitBlt(this, 0, descent, pic);
}

void KSTicker::paintEvent( QPaintEvent *) 
{
  if(isVisible() == FALSE)
    return;
  bitBlt(this, 0, descent, pic);
}

void KSTicker::resizeEvent( QResizeEvent *e) 
{
  QFrame::resizeEvent(e);
  onechar = fontMetrics().width("X");
  chars = this->width() / onechar;
  killTimers();
  QPixmap *new_pic = new QPixmap(width() + onechar, height());
  new_pic->fill(backgroundColor());
  bitBlt(new_pic,
         new_pic->width() - pic->width(), 0,
         pic, 0, 0,
         pic->width(), pic->height(),
         CopyROP, TRUE);
  delete pic;
  pic = new_pic;
  //  if(ring.length() > (uint) chars)
    startTicker();
}

void KSTicker::closeEvent( QCloseEvent *)
{
  emit closing();
  killTimers();
  //  delete this;
}

void KSTicker::startTicker()
{
  killTimers();
  startTimer(tickRate);
}

void KSTicker::stopTicker()
{
  killTimers();
}

void KSTicker::mouseDoubleClickEvent( QMouseEvent * )
{
  emit doubleClick();
}

void KSTicker::mousePressEvent( QMouseEvent *e)
{
  if(e->button() == RightButton){
    popup->popup(this->cursor().pos());
  }
  else{
    QFrame::mousePressEvent(e);
  }
}
void KSTicker::fontSelector()
{
  int result = KFontDialog::getFont( ourFont, true );
  if ( result == KFontDialog::Accepted ) {
    setFont(ourFont);
    setFixedHeight((fontMetrics().height()+fontMetrics().descent()*2)*pHeight);
    resize(fontMetrics().width("X")*chars,
          (fontMetrics().height()+fontMetrics().descent())*pHeight);
 }
}

void KSTicker::scrollRate() 
{
  SpeedDialog *sd = new SpeedDialog(tickRate, tickStep);
  sd->setLimit(5, 200, 1, onechar);
  connect(sd, SIGNAL(stateChange(int, int)),
          this, SLOT(setSpeed(int, int)));
  sd->show();
}

void KSTicker::scrollConstantly()
{
  bScrollConstantly = !bScrollConstantly;
  popup->setItemChecked(iScrollItem, bScrollConstantly);
  if(bScrollConstantly == TRUE)
    startTicker();
  KConfig *conf = kapp->config();
  conf->setGroup("KSTicker");
  conf->writeEntry("ScollConst", bScrollConstantly);
  conf->sync();
}

void KSTicker::updateFont(const QFont &font){
  setFont(font);
  setFixedHeight((fontMetrics().height()+fontMetrics().descent()*2)*pHeight);
  resize(fontMetrics().width("X")*chars,
         (fontMetrics().height()+fontMetrics().descent())*pHeight);
}

void KSTicker::setSpeed(int _tickRate, int _tickStep){
  tickRate = _tickRate;
  tickStep = _tickStep;
  startTicker();
}

void KSTicker::speed(int *_tickRate, int *_tickStep){
  *_tickRate = tickRate;
  *_tickStep = tickStep;
}

void KSTicker::setBackgroundColor ( const QColor &c )
{
  QFrame::setBackgroundColor(c);
  pic->fill(c);
  bitBlt(this, 0,0, pic);
  defbg = backgroundColor();
  bg = backgroundColor();
}

void KSTicker::setPalette ( const QPalette & p )
{
  QFrame::setPalette(p);

  pic->fill(backgroundColor());
  bitBlt(this, 0,0, pic);
  defbg = backgroundColor();
  bg = backgroundColor();
  deffg = p.normal().text();
  fg = p.normal().text();
}

#include "ksticker.moc"
