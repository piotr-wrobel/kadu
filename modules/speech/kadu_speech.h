#ifndef KADU_SPEAK_H
#define KADU_SPEAK_H
#include <qobject.h>
#include <qstring.h>
#include "misc.h"
#include "userlist.h"

class SpeechSlots : public QObject
{
	Q_OBJECT
	private:
		QTime lastSpeech;
	public:
		SpeechSlots();
		~SpeechSlots();
	private slots:
		void onCreateConfigDialog();
		void chooseSpeechProgram();
		
		void say(const QString &s, const QString &path=QString::null,
					bool klatt=false, bool melodie=false,
					bool arts=false, bool esd=false, bool dsp=false, const QString &device=QString::null,
					int freq=0, int tempo=0, int basefreq=0);
		

		void newChat(UinsList senders, const QString& msg, time_t time, bool &grab);
		void newMessage(UinsList senders, const QString& msg, time_t time);
		void connectionError(const QString &message);
		void userChangedStatusToAvailable(const UserListElement &ule);
		void userChangedStatusToBusy(const UserListElement &ule);
		void userChangedStatusToNotAvailable(const UserListElement &ule);
		void message(const QString &from, const QString &type, const QString &message, const UserListElement *ule);

		void useArts();
		void useEsd();
		void useDsp();
		void testSpeech();

};

extern SpeechSlots *speechObj;

#endif

