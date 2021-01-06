// kmimetypevalidator.cpp - A QValidator for mime types.
// Copyright: (c) 2001 Marc Mutz <mutz@kde.org>
// License: GPL

#include "kmimetypevalidator.h"
#include <qregexp.h>
#include "qregexp3.h"

QValidator::State KMimeTypeValidator::validate( QString & input, int& ) const
{
  if ( input.isEmpty() )
    return Intermediate;

  QRegExp3 acceptable("[!#-'*+.0-9^-~+-]+/[!#-'*+.0-9^-~+-]+",
		      false /*case-insens.*/);
  if ( acceptable.exactMatch( input ) )
    return Acceptable;

  QRegExp3 intermediate("[!#-'*+.0-9^-~+-]*/?[!#-'*+.0-9^-~+-]*",
			false /*case-insensitive*/);
  if ( intermediate.exactMatch( input ) )
    return Intermediate;

  return Invalid;
}

void KMimeTypeValidator::fixup( QString & input ) const
{
  input.replace( QRegExp("[^!#-'*+./0-9^-~+-]+"), "");
}

#include "kmimetypevalidator.moc"
