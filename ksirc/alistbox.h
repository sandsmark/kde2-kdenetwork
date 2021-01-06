#ifndef ALISTBOX_H
#define ALISTBOX_H

class aListBox;
class nickListItem;

#include <qobject.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qevent.h>

#include "KSircListBox/irclistitem.h"

class nickListItem : public QListBoxItem
{
 public:
  nickListItem();
  virtual ~nickListItem();

  virtual int height ( const QListBox * ) const;
  virtual int width ( const QListBox * ) const;
    QString text () const { return string; }
  const QPixmap* pixmap () const;

    bool op() const { return is_op; }
    bool voice() const { return is_voice; }
    bool away() const { return is_away; }
    bool ircOp() const { return is_ircop; }

  void setOp(bool _op = FALSE);
  void setVoice(bool _voice = FALSE);
  void setAway(bool _away = FALSE);
  void setIrcOp(bool _ircop = FALSE);

    void setText(const QString &str) { string = str; }

    QString nickPrefix() const;

  nickListItem &operator= ( const nickListItem & nli );

 protected:
  virtual void paint ( QPainter * );

 private:
  bool is_op:1;
  bool is_voice:1;
  bool is_away:1;
  bool is_ircop:1;

  QString string;
};

class aListBox : public QListBox
{
  Q_OBJECT;

public:
    aListBox(QWidget *parent = 0, const char *name = 0);

    virtual ~aListBox();

  void clear();

  void inSort ( nickListItem *);
  void inSort ( const char * text, bool top=FALSE);

  nickListItem *item(int index);

  bool isTop(int index);

  int findNick(QString str);

    bool needNickPrefix() const;

    void setNickListDirty()
        { m_nickListDirty = true; }

    unsigned short int nickPrefixWidth() const
        { return m_nickPrefixWidth; }

    virtual void fontChange( const QFont &f );

    static QString nickPrefixOp() { return QString::fromLatin1( "+o" ); }
    static QString nickPrefixVoice() { return QString::fromLatin1( "+v" ); }
    static QString nickPrefixAway() { return QString::fromLatin1( "+a" ); }
    static QString nickPrefixIrcOp() { return QString::fromLatin1( "+O" ); }

signals:
    void rightButtonPress(int index);
    void selectedNick(const QString&);
    void urlsDropped( const QStringList& urls, const QString& nick );

protected:
  virtual void mousePressEvent ( QMouseEvent * );
  virtual int findSep();
  virtual int searchFor(QString nick, bool &found, bool top);
  virtual void dragMoveEvent( QDragMoveEvent * );
  virtual void dropEvent( QDropEvent * );

private:
    void updateNeedNickPrefixFlag() const;
    void updateNickPrefixWidth();

    QPalette p_scroll;
    mutable bool m_nickListDirty;
    mutable bool m_needNickPrefix;
    unsigned short int m_nickPrefixWidth;
};

#endif
