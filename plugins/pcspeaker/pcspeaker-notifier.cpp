/*
 * %kadu copyright begin%
 * Copyright 2011 Tomasz Rostanski (rozteck@interia.pl)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "pcspeaker-notifier.h"
#include "moc_pcspeaker-notifier.cpp"

#include "pcspeaker-configuration-widget.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "notification/notification.h"
#include "plugin/plugin-injected-factory.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if defined(Q_OS_UNIX)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#endif

// Sound Frequencies
// Rows - sounds: C, C#, D, D#, E, F, F#, G, G#, A, A#, B
// Cols - octaves (0 to 7)
int sounds[96] = {16, 33, 65,  131, 262, 523, 1046, 2093, 17, 35, 69,  139, 277, 554, 1109, 2217,
                  18, 37, 73,  147, 294, 587, 1175, 2349, 19, 39, 78,  155, 311, 622, 1244, 2489,
                  21, 41, 82,  165, 330, 659, 1328, 2637, 22, 44, 87,  175, 349, 698, 1397, 2794,
                  23, 46, 92,  185, 370, 740, 1480, 2960, 24, 49, 98,  196, 392, 784, 1568, 3136,
                  26, 52, 104, 208, 415, 831, 1661, 3322, 27, 55, 110, 220, 440, 880, 1760, 3520,
                  29, 58, 116, 233, 466, 932, 1865, 3729, 31, 62, 123, 245, 494, 988, 1975, 3951};

#if defined(Q_OS_WIN)
void PCSpeakerNotifier::beep(int pitch, int duration)
{
    if (pitch == 0)
        Sleep(duration / 5); /* instead of (duration * 200) / 1000 */
    else
        Beep(pitch, duration);
}
#elif defined(Q_OS_UNIX)
void PCSpeakerNotifier::beep(int pitch, int duration)
{
    if (pitch == 0)
        usleep(static_cast<useconds_t>(duration * 200));
    else
    {
        XKeyboardState s;   // save previous sound config
        XGetKeyboardControl(xdisplay, &s);
        XKeyboardControl v;                                                                     // pause when set to 0
        v.bell_pitch = pitch;                                                                   // sound frequency in Hz
        v.bell_duration = duration;                                                             // sound duration
        v.bell_percent = 100;                                                                   // set volume to max
        XChangeKeyboardControl(xdisplay, (KBBellPitch | KBBellDuration | KBBellPercent), &v);   // set sound config
        XBell(xdisplay, volume);                                                                // put sound to buffer
        XFlush(xdisplay);                                                                       // flush buffer (beep)
        usleep(static_cast<useconds_t>(pitch * 100));    // wait until sound is played
        v.bell_pitch = static_cast<int>(s.bell_pitch);   // restore previous sound config
        v.bell_duration = static_cast<int>(s.bell_duration);
        v.bell_percent = static_cast<int>(s.bell_percent);
        XChangeKeyboardControl(
            xdisplay, (KBBellPitch | KBBellDuration | KBBellPercent), &v);   // set restored sound config
    }
}
#else
void PCSpeakerNotifier::beep(int pitch, int duration)
{
    Q_UNUSED(pitch);
    Q_UNUSED(duration);
}
#endif

PCSpeakerNotifier::PCSpeakerNotifier(QObject *parent)
        : QObject{parent}, Notifier{"PC Speaker", QT_TRANSLATE_NOOP("@default", "PC Speaker"),
                                    KaduIcon("audio-volume-low")},
#if defined(Q_OS_UNIX)
          xdisplay{},
#endif
          volume{}
{
}

PCSpeakerNotifier::~PCSpeakerNotifier()
{
}

void PCSpeakerNotifier::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void PCSpeakerNotifier::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

NotifierConfigurationWidget *PCSpeakerNotifier::createConfigurationWidget(QWidget *parent)
{
    return m_pluginInjectedFactory->makeInjected<PCSpeakerConfigurationWidget>(this, parent);
}

void PCSpeakerNotifier::notify(const Notification &notification)
{
    parseAndPlay(m_configuration->deprecatedApi()->readEntry("PC Speaker", notification.type + "_Sound"));
}

void PCSpeakerNotifier::parseStringToSound(QString line, int tab[21], int tab2[21])
{
    int length = line.length();
    line = line.toUpper();
    int tmp, k = 0;
    char znak, tmp3;
    int i;
    if (length > 0)
    {
        for (i = 0; i < length; ++i)   // for each sound
        {
            if (k >= 20)
                break;
            znak = line[i].toLatin1();
            switch (znak)
            {   // calculate offset in sound table
            case 'C':
                tmp = 0;
                break;
            case 'D':
                tmp = 2;
                break;
            case 'E':
                tmp = 4;
                break;
            case 'F':
                tmp = 5;
                break;
            case 'G':
                tmp = 7;
                break;
            case 'A':
                tmp = 9;
                break;
            case 'B':
                tmp = 11;
                break;
            case '_':
            {
                tab[k] = 0;   // play pause
                tmp = -1;
                if (line[i + 1] == '/')   // set pause length
                {
                    if (line[i + 2] == 'F')
                        tmp3 = 16;
                    else if ((line[i + 2] >= '1') && (line[i + 2] <= '8'))
                        tmp3 = line[i + 2].toLatin1() - 48;
                    else
                        tmp3 = 1;
                    tab2[k] = (1000 / tmp3);
                    i += 2;
                }
                else
                    tab2[k] = 1000;   // if not given use 1000
                ++k;
            }
            break;
            default:
                tmp = -1;
            }
            if (tmp >= 0)
            {
                tmp *= 8;
                bool alreadyHalf = false;
                if (line[i + 1] == '#')
                {               // for halftone
                    tmp += 8;   // set offset
                    ++i;        // go forward
                    alreadyHalf = true;
                }
                if ((line[i + 1] >= '0') && (line[i + 1] <= '7'))
                {
                    tmp += line[i + 1].toLatin1() - 48;   // calculate offset basing on octave
                    ++i;                                  // go forward
                }
                if (line[i + 1] == '#')
                {   // for halftone
                    if (!alreadyHalf)
                        tmp += 8;   // set offset
                    ++i;            // go forward
                }

                if (tmp >= 0 && tmp < 96)
                {
                    tab[k] = sounds[tmp];   // store sound frequency
                    if (line[i + 1] == '/')
                    {
                        // set duration
                        if (line[i + 2] == 'F')
                            tmp3 = 16;
                        else if ((line[i + 2] >= '1') && (line[i + 2] <= '8'))
                            tmp3 = line[i + 2].toLatin1() - 48;
                        else
                            tmp3 = 1;
                        tab2[k] = (1000 / tmp3);
                        i += 2;
                    }
                    else
                        tab2[k] = 1000;   // if not given use 1000
                    ++k;                  // move to the next sound
                }
            }
        }
    }
    tab[k] = -1;   // set sound end condition (-1)
}

void PCSpeakerNotifier::play(int sound[21], int soundlength[20])
{
#if defined(Q_OS_UNIX)
    xdisplay = XOpenDisplay(NULL);
#endif
    for (int i = 0; i < 20; ++i)
    {
        if (sound[i] == -1)
            break;
        beep(sound[i], soundlength[i]);
    }
#if defined(Q_OS_UNIX)
    XCloseDisplay(xdisplay);
#endif
}

void PCSpeakerNotifier::parseAndPlay(QString line)
{
    volume = m_configuration->deprecatedApi()->readNumEntry("PC Speaker", "SpeakerVolume");
    int sound[21], soundLength[20];
    parseStringToSound(line, sound, soundLength);
    play(sound, soundLength);
}

void PCSpeakerNotifier::createDefaultConfiguration()
{
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "SpeakerVolume", 100);
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "NewChat_Sound", "C4/2");
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "NewMessage_Sound", "F2/2");
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "ConnectionError_Sound", "D3/4");
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "StatusChanged_Sound", "A3/2");
    m_configuration->deprecatedApi()->addVariable("PC Speaker", "FileTransfer_Sound", "E4/4");
}
