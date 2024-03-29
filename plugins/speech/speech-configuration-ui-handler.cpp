/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
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

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>

#include "buddies/buddy-dummy-factory.h"
#include "buddies/buddy-preferred-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "configuration/gui/configuration-ui-handler-repository.h"
#include "core/core.h"
#include "misc/paths-provider.h"
#include "parser/parser.h"
#include "speech.h"
#include "widgets/configuration/config-combo-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "widgets/select-file.h"
#include "windows/main-configuration-window.h"

#include "speech-configuration-ui-handler.h"
#include "moc_speech-configuration-ui-handler.cpp"

SpeechConfigurationUiHandler::SpeechConfigurationUiHandler(QObject *parent)
        : QObject{parent}, frequencySlider{}, tempoSlider{}, baseFrequencySlider{}, dspDeviceLineEdit{},
          klattSyntCheckBox{}, melodyCheckBox{}, programSelectFile{}, soundSystemComboBox{}
{
}

SpeechConfigurationUiHandler::~SpeechConfigurationUiHandler()
{
}

void SpeechConfigurationUiHandler::setBuddyDummyFactory(BuddyDummyFactory *buddyDummyFactory)
{
    m_buddyDummyFactory = buddyDummyFactory;
}

void SpeechConfigurationUiHandler::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void SpeechConfigurationUiHandler::setParser(Parser *parser)
{
    m_parser = parser;
}

void SpeechConfigurationUiHandler::setSpeech(Speech *speech)
{
    m_speech = speech;
}

void SpeechConfigurationUiHandler::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
    frequencySlider = static_cast<QSlider *>(mainConfigurationWindow->widget()->widgetById("speech/frequency"));
    tempoSlider = static_cast<QSlider *>(mainConfigurationWindow->widget()->widgetById("speech/tempo"));
    baseFrequencySlider = static_cast<QSlider *>(mainConfigurationWindow->widget()->widgetById("speech/baseFrequency"));
    melodyCheckBox = static_cast<QCheckBox *>(mainConfigurationWindow->widget()->widgetById("spech/melody"));

    programSelectFile = static_cast<SelectFile *>(mainConfigurationWindow->widget()->widgetById("speech/program"));
    ;

    soundSystemComboBox =
        static_cast<ConfigComboBox *>(mainConfigurationWindow->widget()->widgetById("speech/soundSystem"));
    dspDeviceLineEdit = static_cast<QLineEdit *>(mainConfigurationWindow->widget()->widgetById("speech/dspDevice"));
    klattSyntCheckBox = static_cast<QCheckBox *>(mainConfigurationWindow->widget()->widgetById("speech/klattSynt"));

    connect(soundSystemComboBox, SIGNAL(activated(int)), this, SLOT(soundSystemChanged(int)));
    connect(mainConfigurationWindow->widget()->widgetById("speech/test"), SIGNAL(clicked()), this, SLOT(testSpeech()));
}

void SpeechConfigurationUiHandler::mainConfigurationWindowDestroyed()
{
}

void SpeechConfigurationUiHandler::mainConfigurationWindowApplied()
{
}

void SpeechConfigurationUiHandler::soundSystemChanged(int index)
{
    Q_UNUSED(index)
    bool dsp = soundSystemComboBox->currentItemValue() == "Dsp";

    dspDeviceLineEdit->setEnabled(dsp);
    klattSyntCheckBox->setEnabled(dsp);
}

void SpeechConfigurationUiHandler::testSpeech()
{
    if (!programSelectFile)
        return;

    QString program = programSelectFile->file();
    // TODO: mo�e u�ywa� jakiego� normalnego tekstu ?
    QString formatM = m_configuration->deprecatedApi()->readEntry("Speech", "NewChat_Syntax/Male");
    QString formatF = m_configuration->deprecatedApi()->readEntry("Speech", "NewChat_Syntax/Female");
    QString device = dspDeviceLineEdit->text();
    bool klatt = klattSyntCheckBox->isChecked();
    bool mel = melodyCheckBox->isChecked();

    QString sound_system = soundSystemComboBox->currentItemValue();

    int frequency = frequencySlider->value();
    int tempo = tempoSlider->value();
    int baseFrequency = baseFrequencySlider->value();

    QString text;
    text = m_parser->parse(formatF, Talkable(m_buddyDummyFactory->dummy()), ParserEscape::HtmlEscape);

    m_speech->say(
        text.contains("%1") ? text.arg("Test") : QString("Test"), program, klatt, mel, sound_system, device, frequency,
        tempo, baseFrequency);
}
