/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "player_arts.h"
#include "debug.h"
#include "config_dialog.h"
#include <string>

extern "C" int arts_sound_init()
{
	kdebugf();

	artsPlayerObj=new ArtsPlayerSlots();
	if (artsPlayerObj->server.isNull())
	{
		delete artsPlayerObj;
		return -1;
	}

	QObject::connect(sound_manager, SIGNAL(playSound(const QString &, bool, double)),
					 artsPlayerObj, SLOT(playSound(const QString &, bool, double)));
	QObject::connect(sound_manager, SIGNAL(playOnMessage(UinsList, const QString &, const QString &, bool, double)),
					 artsPlayerObj, SLOT(playMessage(UinsList, const QString &, const QString &, bool, double)));
	QObject::connect(sound_manager, SIGNAL(playOnChat(UinsList, const QString &, const QString &, bool, double)),
					 artsPlayerObj, SLOT(playChat(UinsList, const QString &, const QString &, bool, double)));
	QObject::connect(sound_manager, SIGNAL(playOnNotify(const UinType, const QString &, bool, double)),
					 artsPlayerObj, SLOT(playNotify(const UinType, const QString &, bool, double)));

	return 0;
}
extern "C" void arts_sound_close()
{
	kdebugf();

	QObject::disconnect(sound_manager, SIGNAL(playSound(const QString &, bool, double)),
						artsPlayerObj, SLOT(playSound(const QString &, bool, double)));
	QObject::disconnect(sound_manager, SIGNAL(playOnMessage(UinsList, const QString &, const QString &, bool, double)),
						artsPlayerObj, SLOT(playMessage(UinsList, const QString &, const QString &, bool, double)));
	QObject::disconnect(sound_manager, SIGNAL(playOnChat(UinsList, const QString &, const QString &, bool, double)),
						artsPlayerObj, SLOT(playChat(UinsList, const QString &, const QString &, bool, double)));
	QObject::disconnect(sound_manager, SIGNAL(playOnNotify(const UinType, const QString &, bool, double)),
						artsPlayerObj, SLOT(playNotify(const UinType, const QString &, bool, double)));
	delete artsPlayerObj;
	artsPlayerObj=NULL;
}

ArtsPlayerSlots::ArtsPlayerSlots()
{
	kdebugf();
	server=Arts::Reference("global:Arts_SoundServerV2");
}

void ArtsPlayerSlots::play(const QString &s, bool volCntrl, double vol)
{
	//warunku server.isNull() i server.audioMethod()=="" nie mo�na sprawdza�
	//jednocze�nie, bo je�eli najpierw zostanie sprawdzony drugi, a pierwszy
	//jest prawd�, to program si� wywr�ci
	kdebug("ArtsPlayerSlots::play(): null: %d\n",server.isNull());

	if (!server.isNull())
		kdebug("audioMethod: %s audioDevice: %s\n", server.audioMethod().c_str(), server.audioDevice().c_str());

	if (server.isNull())
		server=Arts::Reference("global:Arts_SoundServerV2");
	else if (server.audioMethod()=="")
		server=Arts::Reference("global:Arts_SoundServerV2");
	
	if (!server.isNull())
		if (server.audioMethod()!="")
		{
			if (volCntrl)
				server.outVolume().scaleFactor(vol);
			server.play(std::string(s.ascii()));
		}
}

void ArtsPlayerSlots::playSound(const QString &s, bool volCntrl, double vol)
{
	kdebugf();
	play(s, volCntrl, vol);
}

void ArtsPlayerSlots::playMessage(UinsList senders, const QString &sound, const QString &msg, bool volCntrl, double vol)
{
	kdebugf();
	play(sound, volCntrl, vol);
}

void ArtsPlayerSlots::playChat(UinsList senders, const QString &sound, const QString &msg, bool volCntrl, double vol)
{
	kdebugf();
	play(sound, volCntrl, vol);
}

void ArtsPlayerSlots::playNotify(const UinType uin, const QString &sound, bool volCntrl, double vol)
{
	kdebugf();
	play(sound, volCntrl, vol);
}

ArtsPlayerSlots *artsPlayerObj;
