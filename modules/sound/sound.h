#ifndef SOUND_H
#define SOUND_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qmap.h>
#include "config_file.h"

#include "misc.h"
#include "modules.h"

class SoundSlots: public QObject
{
	Q_OBJECT
	private:
		int muteitem;
		QMap<QString, QString> soundfiles;
		QStringList soundNames;
		QStringList soundTexts;

	private slots:
		void soundPlayer(bool value, bool toolbarChanged=false);
		void onCreateConfigDialog();
		void onApplyConfigDialog();
		void chooseSoundTheme(const QString& string);
		void chooseSoundFile();
		void clearSoundFile();
		void testSoundFile();
		void selectedPaths(const QStringList& paths);
		void muteUnmuteSounds();
	public:
		SoundSlots();
		~SoundSlots();
};

class SoundManager : public Themes
{
    Q_OBJECT
	private:
		QTime lastsoundtime;
		bool mute;

	private slots:
		void newChat(UinsList senders, const QString& msg, time_t time, bool &grab);
		void newMessage(UinsList senders, const QString& msg, time_t time);
		void connectionError(const QString &message);
		void userChangedStatusToAvailable(const UserListElement &ule);
		void userChangedStatusToBusy(const UserListElement &ule);
		void userChangedStatusToNotAvailable(const UserListElement &ule);
		void message(const QString &from, const QString &type, const QString &message, const UserListElement *ule);

	public slots:
		void play(const QString &path, bool force=false);
		void setMute(const bool& enable);

	public:
		SoundManager(const QString& name, const QString& configname);
		~SoundManager();
		bool isMuted();
		int timeAfterLastSound();		

	signals:
		void playSound(const QString &sound, bool volCntrl, double vol);

		void playOnNewMessage(UinsList senders, const QString &sound, bool volCntrl, double vol, const QString &msg);
		void playOnNewChat(UinsList senders, const QString &sound, bool volCntrl, double vol, const QString &msg);
		void playOnConnectionError(const QString &sound, bool volCntrl, double vol, const QString &msg);

		void playOnNotifyAvail(const UinType uin, const QString &sound, bool volCntrl, double vol);
		void playOnNotifyBusy(const UinType uin, const QString &sound, bool volCntrl, double vol);
		void playOnNotifyNotAvail(const UinType uin, const QString &sound, bool volCntrl, double vol);
		
		void playOnMessage(const QString &sound, bool volCntrl, double vol, const QString &from, const QString &type, const QString &msg, const UserListElement *ule);
};

extern SoundManager* sound_manager;

#endif
