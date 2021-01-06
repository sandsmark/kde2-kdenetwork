// kitapp.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITAPP_H
#define KITAPP_H

#include <kuniqueapp.h>
#include <kaboutdata.h>

class TWindow;

#define KIT_NAME KApplication::kApplication()->aboutData()->programName()

class KitApp : public KUniqueApplication
{
	Q_OBJECT

	public:
		KitApp(bool = true, bool = true);
		~KitApp();

		virtual int newInstance(void);
		void popProfileInfo(bool useDefault = false);
	signals:
		void saveAll(void);
	protected slots:
		void windowClosing(void);
	private:
		static int numInstances;
};

#endif
