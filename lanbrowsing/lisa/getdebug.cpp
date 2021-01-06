#include "getdebug.h"

//#define LISA_DEBUG_ON

ostream& getDebug()
{
#ifdef LISA_DEBUG_ON
   return cerr;
#else
   static ofstream nullStream("/dev/null");
   return nullStream;
#endif
}

