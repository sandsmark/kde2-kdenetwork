/*******************************************************************

 aListBox

 $$Id: alistbox.cpp 76649 2001-01-06 16:05:31Z malte $$

 List box that outputs a right click mouse so I can popup a qpopupmenu.

 Does special sorting, and maintains a two part list, one for ops,
 other for users.

 nothing special.

*******************************************************************/

#include "alistbox.h"
#include "linelistitem.h"
#include "config.h"

#include <qscrollbar.h>
#include <qdragobject.h>

#include <kdebug.h>

aListBox::aListBox(QWidget *parent, const char *name )
    : QListBox(parent,name)
{
    clear();
    p_scroll = palette().copy();
    setAcceptDrops( true );
    connect(this, SIGNAL(selected (const QString&)),
            this, SIGNAL(selectedNick(const QString&)));

    m_nickListDirty = true;

    updateNickPrefixWidth();
}

aListBox::~aListBox()
{
}

void aListBox::mousePressEvent(QMouseEvent *e)
{

  QListBox::mousePressEvent(e);

  if(e->button() == RightButton){
    // #### HPB: rewrite this
    emit rightButtonPress(index(itemAt(QPoint(0, mapFromGlobal(cursor().pos()).y()))));
  }
}


void aListBox::clear()
{
  QListBox::clear();
}


void aListBox::inSort ( nickListItem *lbi)
{
  int insert;
  bool found;
  insert = searchFor(lbi->text(), found, lbi->op());
  if(found == TRUE){
    kdDebug() << lbi->text() << " is already in nick list!" << endl;
    return;
  }
  insertItem(lbi, insert);
//  for(uint index = 0; index < count(); index++){
//    debug("%d is %s", index, text(index));
//  }

  m_nickListDirty = true;
}

void aListBox::inSort ( const char * text, bool top)
{
  nickListItem *nli = new nickListItem();
  nli->setText(text);
  if(top == TRUE)
    nli->setOp(TRUE);
  inSort(nli);
}

int aListBox::findSep()
{
  uint i = 0;
  for(; i < count(); i++)
    if(item(i)->op() == FALSE)
      break; // stop now

  return i;

}
int aListBox::searchFor(QString nick, bool &found, bool top)
{
  int min = 0, max = 0;
  int current = 0, compare = 0;
  int real_max = 0;
  int insert;

  found = FALSE;

  // If there's nothing in the list, don't try and search it, etc

  if(count() == 0){
    insert = 0;
  }
  else{
    int sep = findSep();
    if(sep >= 0){
      if(top == TRUE){
        min = 0;
        max = (sep >= 1) ? sep - 1 : 0;
      }
      else{
        min = sep;
        max = count() - 1;
      }
    }
    else
      current = -1;

    real_max = max;
    current = (min + max)/2; // + (max-min)%2;
    insert = current;
    int last_current = -1;
    uint loop = 0;           // Most loops should be log_2 count(), but...
    do {
      if(current == last_current){
//        debug("Insert looping on %s", nick.data());
        //      current++;
        break; // we're looping, so stop
      }
      if(current >= max)
        break; // Don't go too far
      last_current = current;

      compare = text(current).lower().compare(nick.lower());
      if(compare < 0){
        min = current;
	insert = current + 1;
//	debug("1 < 0: %s is greater then: %s, min: %d max: %d current: %d", nick.data(), text(current), min, max, current);
      }
      else if(compare > 0){
        max = current;
 	insert = current;
//	debug("1 > 0: %s is less then: %s, min: %d max: %d current: %d", nick.data(), text(current), min, max, current);
      }
      else {// We got a match?
	insert = current;
        min = current;
        found = TRUE;
        break;
      }
      current = (min + max)/2;
      loop++; // Infinite loop detector increment
    } while(max != min && loop < count());

    if(current >= real_max - 1){
      compare = text(real_max).lower().compare(nick.lower());
      if(compare < 0){
	min = current;
	insert = real_max + 1;
//	debug("End check got one!");
      }
      else if (compare == 0){// We got a match
	insert = real_max + 1;
	min = real_max;
	found = TRUE;
      }
    }

    // Sanity check
    if((top == TRUE && insert > sep) ||
       (top == FALSE && insert < sep)){
      insert = sep;
    }

    if(loop == count())
    {
//        debug("Loop inifitly on: %s", nick.data());
    }

    if(found == TRUE){
//      debug("Found %s", nick.data());
      return min; // We found one, so return the number found
    }
  }
//  debug("%s is at %d", nick.data(), insert);
  return insert;

}
bool aListBox::isTop(int index)
{
  if(index >= findSep())
    return FALSE;
  else
    return TRUE;
}

int aListBox::findNick(QString str)
{
  bool found;
  int index;
  index = searchFor(str, found, TRUE);
  if(found == TRUE)
    return index;
  index = searchFor(str, found, FALSE);
  if(found == TRUE)
    return index;
//  debug("Did not find: %s", str.data());
  return -1;
}

nickListItem *aListBox::item(int index){
  return (nickListItem *) QListBox::item(index);
}

void aListBox::dragMoveEvent( QDragMoveEvent *e )
{
    bool ok = (count() > 0 && QUriDrag::canDecode( e ));
    e->accept( ok );
    if ( ok )
	setCurrentItem( itemAt( e->pos() ));
}


void aListBox::dropEvent( QDropEvent *e )
{
    QListBoxItem *item = itemAt( e->pos() );
    if ( !item )
	return;

    setCurrentItem( item );

    QStringList urls;
    QUriDrag::decodeLocalFiles( e, urls );

    if ( !urls.isEmpty() )
	emit urlsDropped( urls, item->text() );
}

bool aListBox::needNickPrefix() const
{
    if ( m_nickListDirty )
        updateNeedNickPrefixFlag();

    return m_needNickPrefix;
}

void aListBox::updateNeedNickPrefixFlag() const
{
    m_needNickPrefix = false;

    QListBoxItem *item = firstItem();
    for (; item; item = item->next() )
    {
        nickListItem *nickItem = static_cast<nickListItem *>( item );
        if ( nickItem->op() ||
             nickItem->voice() ||
             nickItem->away() ||
             nickItem->ircOp() )
        {
            m_needNickPrefix = true;
            break;
        }
    }

    m_nickListDirty = false;
}

void aListBox::fontChange( const QFont &f )
{
    QListBox::fontChange( f );
    updateNickPrefixWidth();
}

void aListBox::updateNickPrefixWidth()
{
    QFontMetrics metrics( font() );

    m_nickPrefixWidth = metrics.width( nickPrefixOp() );
    m_nickPrefixWidth = QMAX( m_nickPrefixWidth, metrics.width( nickPrefixVoice() ) );
    m_nickPrefixWidth = QMAX( m_nickPrefixWidth, metrics.width( nickPrefixAway() ) );
    m_nickPrefixWidth = QMAX( m_nickPrefixWidth, metrics.width( nickPrefixIrcOp() ) );

    // padding
    m_nickPrefixWidth += metrics.width( " " );
}

nickListItem::nickListItem()
  : QListBoxItem()
{
  is_op = FALSE;
  is_voice = FALSE;
  is_away = FALSE;
  is_ircop = FALSE;
}

nickListItem::~nickListItem()
{
  string.truncate(0);
}

void nickListItem::setOp(bool _op)
{
  is_op = _op;
  if ( listBox() )
      static_cast<aListBox *>( listBox() )->setNickListDirty();
}

void nickListItem::setVoice(bool _voice)
{
  is_voice = _voice;
  if ( listBox() )
      static_cast<aListBox *>( listBox() )->setNickListDirty();
}
void nickListItem::setAway(bool _away)
{
  is_away = _away;
  if ( listBox() )
      static_cast<aListBox *>( listBox() )->setNickListDirty();
}

void nickListItem::setIrcOp(bool _ircop)
{
  is_ircop = _ircop;
  if ( listBox() )
      static_cast<aListBox *>( listBox() )->setNickListDirty();
}

void nickListItem::paint(QPainter *p)
{
  QFontMetrics fm = p->fontMetrics();
  int yPos;                       // vertical text position

//  QPen pen = p->pen();
//  QFont font = p->font();

  /*
  if(is_voice == TRUE)
      p->setPen(*kSircConfig->colour_chan);
  if(is_op == TRUE)
      p->setPen(*kSircConfig->colour_error);
  if(is_away == TRUE)
    p->setPen(p->pen().color().dark(150));
  if(is_ircop == TRUE){
    QFont bfont = font;
    bfont.setBold(TRUE);
    p->setFont(bfont);
  }
  */

  yPos = fm.ascent() + fm.leading()/2;

  int nickPosX = 3;

  aListBox *lb = static_cast<aListBox *>( listBox() );

  if ( lb->needNickPrefix() )
  {
      p->drawText( 3, yPos, nickPrefix() );

      nickPosX += lb->nickPrefixWidth();
  }

  p->drawText( nickPosX, yPos, text() );
//  p->setPen(pen);
//  p->setFont(font);
}

QString nickListItem::nickPrefix() const
{
    if ( voice() )
        return aListBox::nickPrefixVoice();
    if ( op() )
        return aListBox::nickPrefixOp();
    if ( away() )
        return aListBox::nickPrefixAway();
    if ( ircOp() )
        return aListBox::nickPrefixIrcOp();

    return QString::null;
}

int nickListItem::height(const QListBox *lb ) const
{
  return lb->fontMetrics().lineSpacing() + 1;
}

int nickListItem::width(const QListBox *lb ) const
{
    return
        static_cast<const aListBox *>( lb )->nickPrefixWidth() +
        lb->fontMetrics().width( text() ) + 6;
}

const QPixmap* nickListItem::pixmap() const
{
  return 0l;
}

nickListItem &nickListItem::operator= (const nickListItem &nli)
{
  string = nli.string;
  is_op = nli.is_op;
  is_voice = nli.is_voice;
  is_away = nli.is_away;
  is_ircop = nli.is_ircop;
  return (*this);
}
#include "alistbox.moc"
