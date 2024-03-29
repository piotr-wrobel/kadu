/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>

#include "configuration/config-file-variant-wrapper.h"
#include "configuration/configuration.h"
#include "core/injected-factory.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "os/generic/window-geometry-manager.h"

#include "path-list-edit.h"
#include "moc_path-list-edit.cpp"

PathListEdit::PathListEdit(QWidget *parent) : QPushButton(tr("Select"), parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(showDialog()));
}

void PathListEdit::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void PathListEdit::showDialog()
{
    if (!Dialog)
    {
        Dialog = m_injectedFactory->makeInjected<PathListEditWindow>(PathList);
        connect(Dialog.data(), SIGNAL(changed(const QStringList &)), this, SLOT(pathListChanged(const QStringList &)));
    }
    Dialog->show();
}

void PathListEdit::pathListChanged(const QStringList &pathList)
{
    PathList = pathList;
    emit changed();
}

void PathListEdit::setPathList(const QStringList &pathList)
{
    PathList = pathList;

    if (Dialog)
        Dialog->setPathList(PathList);
}

PathListEditWindow::PathListEditWindow(const QStringList &pathList, QWidget *parent)
        : QWidget(parent), PathList{pathList}
{
    setWindowTitle(tr("Select paths"));
    setAttribute(Qt::WA_DeleteOnClose);
}

PathListEditWindow::~PathListEditWindow()
{
}

void PathListEditWindow::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void PathListEditWindow::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void PathListEditWindow::init()
{
    createGui();
}

void PathListEditWindow::createGui()
{
    QGridLayout *Layout = new QGridLayout(this);
    Layout->setMargin(5);
    Layout->setSpacing(5);

    PathListWidget = new QListWidget(this);
    Layout->addWidget(PathListWidget, 0, 0, 4, 1);

    QPushButton *add = new QPushButton(m_iconsManager->iconByPath(KaduIcon("list-add")), tr("Add"), this);
    QPushButton *change = new QPushButton(m_iconsManager->iconByPath(KaduIcon("view-refresh")), tr("Change"), this);
    QPushButton *remove = new QPushButton(m_iconsManager->iconByPath(KaduIcon("list-remove")), tr("Remove"), this);

    Layout->addWidget(add, 0, 1);
    Layout->addWidget(change, 1, 1);
    Layout->addWidget(remove, 2, 1);

    connect(add, SIGNAL(clicked()), this, SLOT(addPathClicked()));
    connect(change, SIGNAL(clicked()), this, SLOT(changePathClicked()));
    connect(remove, SIGNAL(clicked()), this, SLOT(deletePathClicked()));

    QWidget *bottom = new QWidget;
    bottom->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    QHBoxLayout *bottom_layout = new QHBoxLayout;
    bottom_layout->setSpacing(5);

    QWidget *hm = new QWidget;
    hm->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    QPushButton *ok = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogOkButton), tr("OK"), this);
    QPushButton *cancel =
        new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("&Cancel"), this);
    bottom_layout->addWidget(hm);
    bottom_layout->addWidget(ok);
    bottom_layout->addWidget(cancel);
    bottom->setLayout(bottom_layout);

    Layout->addWidget(bottom, 5, 0, 1, 2);
    connect(ok, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

    setPathList(PathList);

    new WindowGeometryManager(
        new ConfigFileVariantWrapper(m_configuration, "General", "SelectPathDialogGeometry"), QRect(0, 50, 330, 330),
        this);
}

void PathListEditWindow::setPathList(const QStringList &list)
{
    PathListWidget->clear();
    PathListWidget->insertItems(0, list);
    if (PathListWidget->item(0))
        PathListWidget->item(0)->setSelected(true);
}

bool PathListEditWindow::validatePath(QString &path)
{
    if (path.isEmpty())
        return false;

    QDir dir(path);
    if (!dir.isReadable())
        return false;

    if (!path.endsWith('/'))
        path += '/';

    if (!(PathListWidget->findItems(path, Qt::MatchExactly).isEmpty()))
        return false;

    return true;
}

void PathListEditWindow::addPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose a directory"));

    if (!validatePath(path))
        return;

    PathListWidget->insertItem(0, path);
}

void PathListEditWindow::changePathClicked()
{
    if (!PathListWidget->currentItem())
        return;

    if (!PathListWidget->currentItem()->isSelected())
        return;

    QString path =
        QFileDialog::getExistingDirectory(this, tr("Choose a directory"), PathListWidget->currentItem()->text());

    if (!validatePath(path))
        return;

    QListWidgetItem *pathh = PathListWidget->currentItem();
    pathh->setText(path);
}

void PathListEditWindow::deletePathClicked()
{
    if (!PathListWidget->currentItem())
        return;

    if (!PathListWidget->currentItem()->isSelected())
        return;

    delete PathListWidget->takeItem(PathListWidget->currentRow());
}

void PathListEditWindow::okClicked()
{
    QStringList result;

    for (int i = 0, count = PathListWidget->count(); i < count; i++)
        result.append(PathListWidget->item(i)->text());

    emit changed(result);
    close();
}

void PathListEditWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete)
    {
        e->accept();
        deletePathClicked();
    }
    else if (e->key() == Qt::Key_Escape)
    {
        e->accept();
        close();
    }
    else
        QWidget::keyPressEvent(e);
}
