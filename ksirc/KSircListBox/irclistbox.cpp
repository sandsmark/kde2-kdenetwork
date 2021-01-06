#include "irclistbox.h"
#include <iostream.h>
#include "../config.h"

#include <qevent.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qtimer.h>
#include <qdragobject.h>

#include <kapp.h>
#include <kdebug.h>

#include "irclistitem.h"

extern KApplication *kApp;

static const int fudge = 5;

KSircListBox::KSircListBox(QWidget * parent, const char * name, WFlags f) : QListBox(parent,name,f)
{
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOn);
  thDirty = TRUE;
  ScrollToBottom = TRUE;
  saved_ScrollToBottom = TRUE;
  selectMode = FALSE;
  waitForClear = FALSE;
  acceptFiles = false;
  setAcceptDrops( true );
  connect(verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(slotScrollBarChanged(int)));
}

KSircListBox::~KSircListBox()
{
}

bool KSircListBox::scrollToBottom(bool force)
{
  if ( force )
    ScrollToBottom = TRUE;

  if( ScrollToBottom )
  {
     if (numItemsVisible())
        setBottomItem(count()-1);
  }

  return ScrollToBottom;
}

void KSircListBox::resizeEvent(QResizeEvent *e)
{
  QListBox::resizeEvent(e);

  // Dirty the buffer

  thDirty = TRUE;
  emit updateSize();

  thDirty = TRUE;
  scrollToBottom(TRUE);
}

void KSircListBox::pageUp()
{
  setContentsPos(0, QMAX(0, contentsY()-height()));
  ScrollToBottom = FALSE;

}

void KSircListBox::pageDown()
{
  setContentsPos(0, imin(totalHeight()-height()+fudge, contentsY()+height()));
}

void KSircListBox::lineUp()
{
  setContentsPos(0, QMAX(0, contentsY()-itemHeight(topItem())));
  ScrollToBottom = FALSE;
}

void KSircListBox::lineDown()
{
  setContentsPos(0, imin(totalHeight()-height()+fudge, contentsY()+itemHeight(topItem())));
}


int KSircListBox::imin(int max, int offset){
  if(max < offset){
    ScrollToBottom = TRUE;
    return max;
  }
  else{
    ScrollToBottom = FALSE;
    return offset;
  }

}

int KSircListBox::totalHeight ()
{
  thDirty = FALSE;
  theightCache = contentsHeight();
  return theightCache;
}

void KSircListBox::insertItem ( const QListBoxItem *lbi, int index )
{
  QListBox::insertItem(lbi, index);
  theightCache += lbi->height(this);
}

void KSircListBox::insertItem ( const char * text, int index )
{
  QListBox::insertItem(text, index);
  thDirty = TRUE;
}

void KSircListBox::insertItem ( const QPixmap & pixmap, int index )
{
  QListBox::insertItem(pixmap, index);
  thDirty = TRUE;
}

void KSircListBox::removeItem ( int index )
{
  theightCache -= item(index)->height(this);
  thDirty = TRUE;
  QListBox::removeItem(index);
}


void KSircListBox::clear()
{
  thDirty = TRUE;
  QListBox::clear();

}

void KSircListBox::mousePressEvent(QMouseEvent *me){

  if(me->button() == LeftButton){
    selecting = TRUE; // We're set to select, starting watching the action
    selectMode = FALSE; // We're not going to show anything until they move though
    spoint.setX(me->x());
    spoint.setY(me->y());
    saved_ScrollToBottom = ScrollToBottom;
    ScrollToBottom = FALSE; // Lock window
    if(waitForClear == TRUE) // clean up any prior selections
      clearSelection();
  }
}

void KSircListBox::mouseReleaseEvent(QMouseEvent *me){

  if(me->button() == LeftButton){
    disconnect(kApp->clipboard(), SIGNAL(dataChanged()),
               this, SLOT(clearSelection()));
    int erow; // End row

    selecting = FALSE; // We're done stop listning to related mouse events

    // Unlock window THIS HAS TO BE FIRST SINCE THE WINDOW IS LOCKED ON A MOUSE CLICK
    ScrollToBottom = saved_ScrollToBottom;

    if(selectMode == FALSE) // If they just clicked and didn't move, bug out
      return;

    selectMode = FALSE; // We're done selecting
    int row, line, rchar;
    ircListItem *it;
    if(!xlateToText(me->x(), me->y(), &row, &line, &rchar, &it)){
      erow = lrow;
      row = erow;
      it = (ircListItem *) item(erow);
      if( !it ){
	kdDebug() << "Row out of range: " << row << endl;
        return;
      }
    }
    else{
      erow = row;
    }
    if(erow == srow){
      kdDebug() << "Selected: " << KSPainter::stripColourCodes(it->getRev()) << endl;
      kApp->clipboard()->setText(KSPainter::stripColourCodes(it->getRev()));
      updateItem(row);
    }
    else {
      int trow, brow; // Top row, Bottom row
      QString selected;

      if(erow < srow){
        trow = erow;
        brow = srow;
      }
      else{
        trow = srow;
        brow = erow;
      }
      for(int crow = trow; crow <= brow; crow ++){
        ircListItem *cit = (ircListItem *) item(crow);
        if(cit == 0x0){
	  kdDebug() << "Row out of range: " << crow << endl;
          return;
        }
        selected.append(KSPainter::stripColourCodes(cit->getRev()));
        selected.append("\n");
      }
      selected.truncate(selected.length()-1); // Remove the last \n
      kApp->clipboard()->setText(selected);
      kdDebug() << "selected: " << selected << endl;
    }
    waitForClear = TRUE;
    connect(kApp->clipboard(), SIGNAL(dataChanged()),
            this, SLOT(clearSelection()));
    QTimer::singleShot(1000, this, SLOT(clearSelection()));
  }
  else if(me->button() == MidButton){
    emit pasteReq();
  }
}

void KSircListBox::clearSelection() {
  for(int i = 0; i < (int) count(); i++){ // Start one prior and one back to just really make sure
    ircListItem *it = (ircListItem *) item(i);
    if(it == 0x0){
      continue;
    }
    it->setRevOne(-1);
    it->setRevTwo(-1);
    it->updateSize();
    updateItem(i);
  }
  min = 1; // Turns off repeated clears
  max = 0;
  waitForClear = FALSE;
//  cerr << "Got clear\n";
}

void KSircListBox::mouseMoveEvent(QMouseEvent *me){

  if(selecting == TRUE){
    int row = -2, line = -2, rchar = -2;
    ircListItem *it;
    if(!xlateToText(me->x(), me->y(), &row, &line, &rchar, &it))
        return;
    if(selectMode == FALSE){
      int xoff, yoff;
      xoff = me->x() - spoint.x() > 0 ? me->x() - spoint.x() : spoint.x() - me->x();
      yoff = me->y() - spoint.y() > 0 ? me->y() - spoint.y() : spoint.y() - me->y();
      if(!(xoff > 5 || yoff > 5))
        return;
      if(!xlateToText(spoint.x(), spoint.y(),&srow, &sline, &schar, &sit)){
        spoint.setX(me->x());
        spoint.setY(me->y());

        return;
      }
//      clearSelection();
      max = min = sline; // start setting max and min info for clean up
      sit->setRevOne(schar);
      selectMode = TRUE;
    }
    //  if(schar == rchar)
    //    rchar++;
    if(row == srow){
      it->setRevTwo(rchar);
      it->updateSize();
      updateItem(row);
    }
    else if(row > srow){
      sit->setRevTwo(strlen(sit->text()));
      sit->updateSize();
      updateItem(srow);
      for(int crow = srow + 1; crow < row; crow++){
        ircListItem *cit = (ircListItem *) item(crow);
        if(cit == 0x0){
	  kdDebug() << "Row out of range: " << crow << endl;
          return;
        }
        cit->setRevOne(0);
        cit->setRevTwo(strlen(cit->text()));
        cit->updateSize();
        updateItem(crow);
      }
      it->setRevOne(0);
      it->setRevTwo(rchar);
      it->updateSize();
      updateItem(row);
    }
    else if(row < srow){
      sit->setRevTwo(0);
      sit->updateSize();
      updateItem(srow);
      for(int crow = srow - 1; crow > row; crow--){
        ircListItem *cit = (ircListItem *) item(crow);
        if(cit == 0x0){
	  kdDebug() << "Row out of range: " << crow << endl;
          return;
        }
        cit->setRevOne(0);
        cit->setRevTwo(strlen(cit->text()));
        cit->updateSize();
        updateItem(crow);
      }
      it->setRevOne(rchar);
      it->setRevTwo(strlen(it->text()));
      it->updateSize();
      updateItem(row);
    }
    if(lrow > row && lrow > srow){
      int trow = row > srow ? row : srow;
      for(int crow = lrow; crow > trow; crow --){
        ircListItem *cit = (ircListItem *) item(crow);
        if(cit == 0x0){
	  kdDebug() << "Row out of range: " << crow << endl;
          return;
        }
        cit->setRevOne(-1);
        cit->setRevTwo(-1);
        cit->updateSize();
        updateItem(crow);
      }
    }
    else if(lrow < row && lrow < srow){
      int brow = row < srow ? row : srow;
      for(int crow = lrow; crow < brow; crow++){
        ircListItem *cit = (ircListItem *) item(crow);
        if(cit == 0x0){
	  kdDebug() << "Row out of range: " << crow << endl;
          return;
        }
        cit->setRevOne(-1);
        cit->setRevTwo(-1);
        cit->updateSize();
        updateItem(crow);
      }
    }
    if(row > max)
      max = row;
    else if(row < min)
      min = row;

    lrow = row;
  }

}
bool KSircListBox::xlateToText(int x, int y,
                               int *rrow, int *rline, int *rchar, ircListItem **rit){
  int row, line;
  int top; // Index of top item in list box.
  int lineheight; // Height in Pixels for each line
  QString sline; // s line == sample line
  QList<int> c2noc; // Conversion table for colour numbers to "no colour" numbers

  // Bring the cursor positions to within sane ranges
  // If it's beyond the top scroll up and same if it's too far down
  if(x < 0)
    x = 0;
  else if(x > width())
    x = width();
  if(y < 0){
    mouseSelScrollUp();
    y = 0;
  }
  else if(y > height()){
    mouseSelScrollDown();
    y = height();
  }

  // Now we find the offset of the time item.  We use this offset to
  // calculate the line offset within each item.

  int ttotal = 0;
  for(top = topItem()-1; top >= 0; top--){
    ttotal += item(top)->height(this);
  }

  // We've found our offset, now we get which irc item we're looking for.
  top = topItem();
  if(top < 0)
    return FALSE;
  lineheight = fontMetrics().lineSpacing();

  // contentsY return the top pixel and ttotal holds the pixels upto to the top item.  The diffrence between the two is the offset of the top item

  int yoff = y + (contentsY() - ttotal);
  if( !item(top) ) {
    return FALSE;
  }
  
  for(row = top; yoff > item(row)->height(this); row++) {
    yoff -= item(row)->height(this);
    if((row+1 >= (int) count()) || item(row+1) == 0x0)
      return FALSE;
  }

  // We have our row, let's figure which line it is

  for(line = 0; yoff > lineheight; line++) {
    yoff -= lineheight;
  }

  // Let's fetch the row we've found.

  
  if(row < 0 || row >= (int) count())
    return FALSE;

  ircListItem *it = (ircListItem *) item(row);
  if( !it ){
    kdDebug() << "Row out of range: " << row << endl;
    return FALSE;
  }

  // Check line status to make sure it's in range
  // Bail if the line doesn't exist
  // Casting to int should'nt hurt assuming there's < 2 million lines in 1 entry :)

  if(line < 0 || line >= (int) it->paintText()->count())
    return FALSE;

  
  // Go get the right line and strip the colour codes.  (Colour codes
  // aren't displayed so they don't affet spacing on the display)

  sline = KSPainter::stripColourCodes(it->paintText()->at(line), &c2noc);
  if(sline.isNull()){
    kdDebug() << "No such line: " << line << endl;
    return FALSE;
  }

  
  // Now we go looking for which character the cursor was clicked over.

  QFontMetrics fm = fontMetrics();
  int xoff = x, cchar = 0;
  xoff -= it->tsl();      // take eventual timestamp into account
  if(it->pixmap() != 0x0) // Makes adjustments for pixmaps
    xoff -= (it->pixmap()->width() + 5);

  for(;  xoff > fm.width(sline[0]); cchar++){
    xoff -= fm.width(sline[0]);
    sline.remove(0, 1);
    if(sline.isEmpty()){
      break; // End of line, don't give up!
    }
  }

  if(!c2noc.at(cchar))
    return FALSE;

  cchar = *(c2noc.at(cchar)); // Convert to character offset with colour codes.

  if(sline.isEmpty()) // Adjust for end of line
    cchar += 1;

  // Give abolute pos from start of the line
  for(int l = line-1;  l >= 0; l --){
    cchar += strlen(it->paintText()->at(l));
  }

  *rrow = row;
  *rchar = cchar;
  *rline = line;
  *rit = it;
  return TRUE;
}

void KSircListBox::mouseSelScrollUp(){
  if(selecting == TRUE){
    QPoint pos = mapFromGlobal(QCursor::pos());
    if(pos.y() < 0){
      lineUp();
      QTimer::singleShot(250, this, SLOT(mouseSelScrollUp())); // carefull this could lead to a "recursive" type pattern
    }
  }
}

void KSircListBox::mouseSelScrollDown(){
  if(selecting == TRUE){
    QPoint pos = mapFromGlobal(QCursor::pos());
    if(pos.y() > height()){
      lineDown();
      QTimer::singleShot(250, this, SLOT(mouseSelScrollDown()));  // carefull this could lead to a "recursive" type pattern
    }
  }
}

void KSircListBox::dragEnterEvent( QDragEnterEvent *e )
{
    e->accept( QTextDrag::canDecode( e ) ||
               (acceptFiles && QUriDrag::canDecode( e )) );
}

void KSircListBox::dropEvent( QDropEvent *e )
{
    QString text;
    QStringList list;

    // only local files so far
    if ( acceptFiles && QUriDrag::decodeLocalFiles(e, list) ) {
        emit urlsDropped( list );
    }
    else if ( QTextDrag::decode(e, text) ) {
        emit textDropped( text );
    }
}

void KSircListBox::slotScrollBarChanged(int value)
{
    ScrollToBottom = value >= verticalScrollBar()->maxValue();
}

#include "irclistbox.moc"
