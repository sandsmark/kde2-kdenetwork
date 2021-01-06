#ifndef MAPENTRY_H
#define MAPENTRY_H
/*    mapentry.h
 *
 *    Copyright (c) 2000 Alexander Neundorf
 *    neundorf@kde.org
 *
 *    You may distribute under the terms of the GNU General Public
 *    License as specified in the COPYING file.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 */

#include "mystring.h"

struct MapEntry
{
   MyString key;
   MyString value;
};


#endif
