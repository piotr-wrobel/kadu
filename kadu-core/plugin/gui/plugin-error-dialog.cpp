/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "plugin-error-dialog.h"
#include "moc_plugin-error-dialog.cpp"

#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>

PluginErrorDialog::PluginErrorDialog(
    QString pluginName, const QString &text, bool offerLoadInFutureChoice,
    QWidget *parent)
        :   // using C++ initializers breaks Qt's lupdate
          QDialog(parent),
          DesktopAwareObject(this),
          m_pluginName(std::move(pluginName)), m_text{text}, m_offerLoadInFutureChoice{offerLoadInFutureChoice},
          LoadInFutureCheck(0)
{
}

PluginErrorDialog::~PluginErrorDialog()
{
}

void PluginErrorDialog::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void PluginErrorDialog::init()
{
    setWindowRole("kadu-plugin-error");
    setWindowTitle(tr("Kadu"));
    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout *layout = new QGridLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setSpacing(6);
    layout->setRowStretch(0, 100);
    layout->setColumnStretch(1, 100);

    int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
    QPixmap iconPixmap = m_iconsManager->iconByPath(KaduIcon("dialog-error")).pixmap(iconSize, iconSize);
    if (!iconPixmap.isNull())
    {
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setPixmap(iconPixmap);
        layout->addWidget(iconLabel, 0, 0, Qt::AlignTop);
    }

    QLabel *textLabel = new QLabel(m_text, this);
    textLabel->setWordWrap(true);
    layout->addWidget(textLabel, 0, 1);

    if (m_offerLoadInFutureChoice)
    {
        LoadInFutureCheck = new QCheckBox(tr("Try to load this plugin on next Kadu run"), this);
        LoadInFutureCheck->setChecked(true);
        layout->addWidget(LoadInFutureCheck, 1, 1, Qt::AlignTop);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    layout->addWidget(buttonBox, 2, 0, 1, 2, Qt::AlignCenter);
}

void PluginErrorDialog::accept()
{
    QDialog::accept();
    close();

    if (LoadInFutureCheck)
        emit accepted(m_pluginName, LoadInFutureCheck->isChecked());
}
