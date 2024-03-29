/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "widgets/dialog/dialog-widget.h"
#include "widgets/dialog/title-widget.h"

#include "kadu-dialog.h"
#include "moc_kadu-dialog.cpp"

KaduDialog::KaduDialog(DialogWidget *dialogWidget, QWidget *parent)
        : QDialog(parent), DesktopAwareObject(this), CentralWidget(dialogWidget)
{
    setWindowRole("kadu-dialog");
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(CentralWidget->windowTitle());

    createGui();

    connect(this, SIGNAL(accepted()), CentralWidget, SLOT(dialogAccepted()));
    connect(this, SIGNAL(rejected()), CentralWidget, SLOT(dialogRejected()));
    connect(CentralWidget, SIGNAL(valid(bool)), this, SLOT(widgetValidated(bool)));
}

KaduDialog::~KaduDialog()
{
}

void KaduDialog::createGui()
{
    horizontalLayout = new QHBoxLayout(this);

    iconLabel = new QLabel(this);
    iconLabel->setPixmap(CentralWidget->pixmap());
    horizontalLayout->addWidget(iconLabel);

    QSpacerItem *horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);

    verticalLayout = new QVBoxLayout();

    QWidget *widget = new QWidget(this);
    verticalLayout->addWidget(widget);

    Title = new TitleWidget(this);
    Title->setText(CentralWidget->title());

    verticalLayout->addWidget(Title);

    verticalLayout->addWidget(CentralWidget);

    horizontalLayout->addLayout(verticalLayout);

    createButtonBox();
}

void KaduDialog::createButtonBox()
{
    QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal, this);

    OkButton = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogOkButton), tr("Ok"), this);
    OkButton->setDefault(true);
    OkButton->setEnabled(false);
    buttons->addButton(OkButton, QDialogButtonBox::AcceptRole);
    CancelButton = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cancel"), this);
    buttons->addButton(CancelButton, QDialogButtonBox::RejectRole);

    connect(OkButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

    verticalLayout->addWidget(buttons);
}

void KaduDialog::setAcceptButtonText(const QString &text)
{
    OkButton->setText(text);
}

void KaduDialog::setCancelButtonText(const QString &text)
{
    CancelButton->setText(text);
}

void KaduDialog::widgetValidated(bool valid)
{
    OkButton->setEnabled(valid);
}
