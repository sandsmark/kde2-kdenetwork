// kmimetypevalidator.h - A QValidator for mime types.
// Copyright: (c) 2001 Marc Mutz <mutz@kde.org>
// License: GPL

#ifndef _KMIMETYPEVALIDATOR_H_
#define _KMIMETYPEVALIDATOR_H_

#include <qvalidator.h>

class KMimeTypeValidator : public QValidator
{
  Q_OBJECT
public:
  KMimeTypeValidator( QWidget* parent, const char* name=0)
    : QValidator( parent, name ) {}
  
  /** Checks for well-formed mimetype. Returns
      @li Acceptable iff input ~= /^[a-z0-9-]+\/[a-z0-9-]+$/i
      @li Intermediate iff input ~= /^[a-z0-9-]+\/?/i
      @li Invalid else
  */
  virtual State validate( QString & input, int & pos ) const;
  /** Removes all characters that are forbidden in mimetypes. */
  virtual void fixup( QString & input ) const;
};

#endif // _KMIMETYPEVALIDATOR_H_
