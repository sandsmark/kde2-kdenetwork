#ifndef SERVERDATATYPE_H
#define SERVERDATATYPE_H

#include <qlist.h>
#include <qstring.h>

class port
{

public:
  port( const QString &portnum ) { p=portnum; }
  QString portnum() const { return p; }

private:
  QString p;

};


class Server
{

public:
  Server( const QString &group, const QString &server, QList<port> ports, 
          const QString &serverdesc, const QString &script ) {
          g=group; s=server; p=ports; sd=serverdesc; sc=script;
          p.setAutoDelete(TRUE);
          }
  QString group() const      { return g; }
  QString server() const     { return s; }
  QList<port> ports() const      { return p; }
  QString serverdesc() const { return sd; }
  QString script() const     { return sc; }

private:
  QString     g;
  QString     s;
  QList<port> p;
  QString     sd;
  QString     sc;

};

#endif
