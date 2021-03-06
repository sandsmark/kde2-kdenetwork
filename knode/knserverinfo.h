/*
    knserverinfo.h

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

#ifndef KNSERVERINFO_H
#define KNSERVERINFO_H

#include <qstring.h>

class KConfig;


class KNServerInfo {
  
  public:
    enum serverType { STnntp, STsmtp, STpop3 };
    KNServerInfo();
    ~KNServerInfo();
    
    void clear();
    
    void readConf(KConfig *conf);
    void saveConf(KConfig *conf);
            
    //get
    serverType type()         { return t_ype; }
    int id()                  { return i_d; }
    const QString& server()   { return s_erver; }
    const QString& user()     { return u_ser; }
    const QString& pass()     { return p_ass; }
    int port()                { return p_ort; }
    int hold()                { return h_old; }
    int timeout()             { return t_imeout; }
    bool needsLogon()         { return n_eedsLogon; }
    bool isEmpty()            { return s_erver.isEmpty(); }

    //set
    void setType(serverType t)        { t_ype=t; }
    void setId(int i)                 { i_d=i; }
    void setServer(const QString &s)  { s_erver=s; }
    void setUser(const QString &s)    { u_ser=s; }
    void setPass(const QString &s)    { p_ass=s; }
    void setPort(int p)               { p_ort=p; }
    void setHold(int h)               { h_old=h; }
    void setTimeout(int t)            { t_imeout=t; }
    void setNeedsLogon(bool b)        { n_eedsLogon=b; }

    bool operator==(const KNServerInfo &s);

  protected:
    serverType t_ype;

    QString  s_erver,
             u_ser,
             p_ass;

    int i_d,
        p_ort,
        h_old,
        t_imeout;

    bool n_eedsLogon;
    
};


#endif
