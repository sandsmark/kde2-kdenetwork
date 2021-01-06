/*
 * findnic.h
 *
 * Copyright (c) 2001 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef FINDNIC_H
#define FINDNIC_H

#include <qlist.h>
#include <qstring.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct MyNIC
{
   QString name;
   struct sockaddr_in addr;
   struct sockaddr_in netmask;
};

typedef QList<MyNIC> NICList;

NICList* findNICs();

#endif

