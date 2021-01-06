// Copyright (C) 2001 Neil Stevens <multivac@fcmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Except as contained in this notice, the name(s) of the author(s) shall not be
// used in advertising or otherwise to promote the sale, use or other dealings
// in this Software without prior written authorization from the author(s).

#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <qlabel.h>
#include <qlayout.h>

#include "kitpasswordchanger.h"

KitPasswordChanger::KitPasswordChanger(QWidget *_parent, const char *_name)
	: KDialogBase(_parent, _name, true, i18n("Change Password"), Ok | Cancel, Ok)
{
	QFrame *box = makeMainWidget();
	QGridLayout *layout = new QGridLayout(box, 4, 4, KDialog::marginHint(), KDialog::spacingHint());

	QLabel *picture = new QLabel(box);
	picture->setPixmap(locate("data", "kdeui/pics/keys.png"));
	picture->setAlignment(AlignVCenter);
	picture->setMaximumWidth(picture->sizeHint().width());
	layout->addMultiCellWidget(picture, 0, 3, 0, 0, 1);

	layout->addMultiCellWidget(new QLabel(i18n("Passwords must be from 4 to 16 characters,\nuse letters and numbers only, and may not\nbe the same as your screen name."), box), 0, 0, 1, 2, 1);
	layout->addWidget(new QLabel(i18n("Old Password"), box), 1, 1);
	layout->addWidget(new QLabel(i18n("New Password"), box), 2, 1);
	layout->addWidget(new QLabel(i18n("Verify New Password"), box), 3, 1);

	oldPasswordEdit = new KRestrictedLine(box);
	oldPasswordEdit->setValidChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	oldPasswordEdit->setEchoMode(KRestrictedLine::Password);
	newPasswordEdit = new KRestrictedLine(box);
	newPasswordEdit->setValidChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	newPasswordEdit->setEchoMode(KRestrictedLine::Password);
	verifyEdit = new KRestrictedLine(box);
	verifyEdit->setValidChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	verifyEdit->setEchoMode(KRestrictedLine::Password);

	layout->addWidget(oldPasswordEdit, 1, 2);
	layout->addWidget(newPasswordEdit, 2, 2);
	layout->addWidget(verifyEdit, 3, 2);
}

void KitPasswordChanger::accept(void)
{
	if(oldPasswordEdit->text() != old)
	{
		KMessageBox::error(this, i18n("Old Password is incorrect."));
		oldPasswordEdit->clear();
		oldPasswordEdit->setFocus();
	}
	else if(newPasswordEdit->text() != verifyEdit->text())
	{
		KMessageBox::error(this, i18n("New Password and Verify do not match."));
		newPasswordEdit->clear();
		verifyEdit->clear();
		newPasswordEdit->setFocus();
	}
	else KDialogBase::accept();
}

#include "kitpasswordchanger.moc"
