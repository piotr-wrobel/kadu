/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtGui/QKeyEvent>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "hot-key.h"
#include "moc_hot-key.cpp"

QKeySequence HotKey::shortCutFromFile(Configuration *configuration, const QString &groupname, const QString &name)
{
    return QKeySequence::fromString(
        configuration->deprecatedApi()->readEntry(groupname, name), QKeySequence::PortableText);
}

bool HotKey::shortCut(Configuration *configuration, QKeyEvent *e, const QString &groupname, const QString &name)
{
    QString config = configuration->deprecatedApi()->readEntry(groupname, name);
    return !config.isEmpty() && config == keyEventToString(e, QKeySequence::PortableText);
}

QString HotKey::keyEventToString(QKeyEvent *e, QKeySequence::SequenceFormat format)
{
    QString result;
    if ((e->modifiers() & Qt::ControlModifier) || (e->key() == Qt::Key_Control))
        result = "Ctrl+";

    if ((e->modifiers() & Qt::MetaModifier) || (e->key() == Qt::Key_Meta))
        result += "Shift+Alt+";
    else
    {
        if ((e->modifiers() & Qt::ShiftModifier) || (e->key() == Qt::Key_Shift))
            result += "Shift+";
        if ((e->modifiers() & Qt::AltModifier) || (e->key() == Qt::Key_Alt))
            result += "Alt+";
    }

    if (!((e->key() == Qt::Key_Control) || (e->key() == Qt::Key_Shift) || (e->key() == Qt::Key_Alt) ||
          (e->key() == Qt::Key_Meta)))
        result += QKeySequence(e->key()).toString(format);

    return result;
}

HotKeyEdit::HotKeyEdit(QWidget *parent) : LineEditWithClearButton(parent)
{
}

QString HotKeyEdit::shortCutString() const
{
    return shortCut().toString(QKeySequence::PortableText);
}

QKeySequence HotKeyEdit::shortCut() const
{
    return QKeySequence::fromString(text(), QKeySequence::NativeText);
}

void HotKeyEdit::setShortCut(const QString &shortcut)
{
    if (QKeySequence::fromString(shortcut, QKeySequence::PortableText).isEmpty())
        clear();
    else
        setText(shortcut);
}

void HotKeyEdit::setShortCut(const QKeySequence &shortcut)
{
    setText(shortcut.toString(QKeySequence::NativeText));
}

void HotKeyEdit::keyPressEvent(QKeyEvent *e)
{
    setText(HotKey::keyEventToString(e, QKeySequence::NativeText));
}

void HotKeyEdit::keyReleaseEvent(QKeyEvent *)
{
    // sprawdzenie czy ostatnim znakiem jest "+"
    // jesli tak to nie ma takiego skrotu klawiszowego
    if (text().isEmpty())
        return;
    if (text().at(text().length() - 1) == '+')
        clear();
}
