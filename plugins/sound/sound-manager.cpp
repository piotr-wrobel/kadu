/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sound-manager.h"
#include "moc_sound-manager.cpp"

#include "sound-player.h"
#include "sound-theme-manager.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "themes.h"

#include <QtMultimedia/QSound>

SoundManager::SoundManager(QObject *parent) : QObject{parent}, m_mute{false}
{
}

SoundManager::~SoundManager()
{
    stopSound();
}

void SoundManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void SoundManager::setSoundThemeManager(SoundThemeManager *soundThemeManager)
{
    m_soundThemeManager = soundThemeManager;
}

void SoundManager::init()
{
    createDefaultConfiguration();
    setMute(!m_configuration->deprecatedApi()->readBoolEntry("Sounds", "PlaySound"));
}

void SoundManager::createDefaultConfiguration()
{
    m_configuration->deprecatedApi()->addVariable("Notify", "ConnectionError_Sound", false);
    m_configuration->deprecatedApi()->addVariable("Notify", "InvalidPassword_Sound", false);
    m_configuration->deprecatedApi()->addVariable("Notify", "NewChat_Sound", true);
    m_configuration->deprecatedApi()->addVariable("Notify", "NewMessage_Sound", true);
    m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToFreeForChat", false);
    m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToOnline_Sound", false);
    m_configuration->deprecatedApi()->addVariable("Notify", "StatusChanged/ToAway_Sound", false);
    m_configuration->deprecatedApi()->addVariable("Notify", "FileTransfer/IncomingFile_Sound", true);

    m_configuration->deprecatedApi()->addVariable("Sounds", "PlaySound", true);
    m_configuration->deprecatedApi()->addVariable("Sounds", "SoundPaths", QString());
    m_configuration->deprecatedApi()->addVariable("Sounds", "SoundTheme", "default");
    m_configuration->deprecatedApi()->addVariable("Sounds", "SoundVolume", 100);
}

bool SoundManager::isMuted() const
{
    return m_mute;
}

void SoundManager::setMute(bool mute)
{
    m_mute = mute;
}

QObject *SoundManager::playFile(const QString &soundFile, bool force, bool stopCurrentlyPlaying)
{
    if (isMuted() && !force)
        return nullptr;

    if (stopCurrentlyPlaying)
        stopSound();

    if (m_playingSound && !m_playingSound->isFinished())
        return nullptr;

    if (m_player)
    {
        m_soundObject = m_player->playSound(soundFile);
        return m_soundObject;
    }

    m_playingSound->deleteLater();
    m_playingSound = new QSound{soundFile};
    m_playingSound->play();
    m_soundObject = m_playingSound;
    return m_soundObject;
}

QObject *SoundManager::playSoundByName(const QString &soundName)
{
    if (isMuted())
        return nullptr;

    auto file = m_configuration->deprecatedApi()->readEntry("Sounds", soundName + "_sound");
    return playFile(file);
}

QObject *SoundManager::testSoundPlaying()
{
    auto soundFile = m_soundThemeManager->themes()->themePath("default") + "msg.wav";
    return playFile(soundFile, true, true);
}

void SoundManager::stopSound()
{
    if (m_soundObject)
        delete m_soundObject.data();
}

void SoundManager::setPlayer(SoundPlayer *player)
{
    m_player = player;
}
