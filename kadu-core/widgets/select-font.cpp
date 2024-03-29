/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QFontDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

#include "select-font.h"
#include "moc_select-font.cpp"

SelectFont::SelectFont(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(5);

    fontEdit = new QLineEdit(this);
    fontEdit->setReadOnly(true);

    QPushButton *button = new QPushButton(tr("Select"), this);
    connect(button, SIGNAL(clicked()), this, SLOT(onClick()));

    layout->addWidget(fontEdit);
    layout->addWidget(button);

    setLayout(layout);
}

void SelectFont::setFont(const QFont &font)
{
    currentFont = font;
    fontEdit->setText(QString("%1 %2").arg(currentFont.family(), QString::number(currentFont.pointSize())));

    emit fontChanged(currentFont);
}

const QFont &SelectFont::font() const
{
    return currentFont;
}

void SelectFont::onClick()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, currentFont, parentWidget());

    if (ok)
        setFont(f);
}
