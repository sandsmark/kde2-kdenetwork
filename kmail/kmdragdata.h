/* class for drag data 
 * $Id: kmdragdata.h 752 1997-08-11 16:13:11Z stefan $
 */
#ifndef kmdragdata_h
#define kmdragdata_h

#include <qlist.h>

class KMFolder;

class KMDragData
{
public:
  void init (KMFolder* fld, int fromId, int toId);

  KMFolder* folder(void) { return f; }
  int from(void) { return fromId; }
  int to(void) { return toId; }

protected:
  KMFolder* f;
  int fromId, toId;
};

inline void KMDragData::init(KMFolder* fld, int from, int to)
{
  f = fld;
  fromId = from;
  toId = to;
}

#endif /*kmdragdata_h*/

