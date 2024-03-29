/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtGui/QDrag>
#include <QtGui/QKeyEvent>

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "widgets/filtered-tree-view.h"

#include "kadu-tree-view.h"
#include "moc_kadu-tree-view.cpp"

KaduTreeView::KaduTreeView(QWidget *parent)
        : QTreeView(parent), BackgroundImageMode(BackgroundNone), BackgroundTemporaryFile(0)
{
    setAnimated(true);
    setDragEnabled(true);
    setDragDropMode(DragOnly);
    setItemsExpandable(true);
    setExpandsOnDoubleClick(false);
    setHeaderHidden(true);
    setMouseTracking(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(false);
    setWordWrap(true);
}

KaduTreeView::~KaduTreeView()
{
}

void KaduTreeView::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void KaduTreeView::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void KaduTreeView::init()
{
    configurationUpdated();
}

Configuration *KaduTreeView::configuration() const
{
    return m_configuration;
}

void KaduTreeView::configurationUpdated()
{
    bool showExpandingControl = m_configuration->deprecatedApi()->readBoolEntry("Look", "ShowExpandingControl", false);

    if (rootIsDecorated() && !showExpandingControl)
        collapseAll();
    setRootIsDecorated(showExpandingControl);
}

void KaduTreeView::setBackground(
    const QString &backgroundColor, const QString &alternateColor, const QString &file, BackgroundMode mode)
{
    BackgroundColor = backgroundColor;
    AlternateBackgroundColor = alternateColor;
    setAnimated(mode == BackgroundNone);
    BackgroundImageMode = mode;
    BackgroundImageFile = file;
    updateBackground();
}

void KaduTreeView::updateBackground()
{
    // TODO fix image "Stretched" + update on resize event - write image into resource tree
    QString style;
    style.append("QTreeView::branch:has-siblings:!adjoins-item { border-image: none; image: none }");
    style.append("QTreeView::branch:has-siblings:adjoins-item { border-image: none; image: none }");
    style.append("QTreeView::branch:has-childres:!has-siblings:adjoins-item { border-image: none; image: none }");
    if (m_configuration->deprecatedApi()->readBoolEntry("Look", "AlignUserboxIconsTop"))
    {
        style.append(
            "QTreeView::branch:has-children:!has-siblings:closed, QTreeView::branch:closed:has-children:has-siblings "
            "{ border-image: none; image: url(" +
            m_iconsManager->iconPath(KaduIcon{"kadu_icons/stylesheet-branch-closed", "16x16"}) +
            "); margin-top: 4px; image-position: top }");
        style.append(
            "QTreeView::branch:open:has-children:!has-siblings, QTreeView::branch:open:has-children:has-siblings "
            "{ border-image: none; image: url(" +
            m_iconsManager->iconPath(KaduIcon{"kadu_icons/stylesheet-branch-open", "16x16"}) +
            "); image-position: top; margin-top: 8px }");
    }
    else
    {
        style.append(
            "QTreeView::branch:has-children:!has-siblings:closed, QTreeView::branch:closed:has-children:has-siblings "
            "{ border-image: none; image: url(" +
            m_iconsManager->iconPath(KaduIcon{"kadu_icons/stylesheet-branch-closed", "16x16"}) + ") }");
        style.append(
            "QTreeView::branch:open:has-children:!has-siblings, QTreeView::branch:open:has-children:has-siblings "
            "{ border-image: none; image: url(" +
            m_iconsManager->iconPath(KaduIcon{"kadu_icons/stylesheet-branch-open", "16x16"}) + ") }");
    }

    style.append("QTreeView { background-color: transparent;");

    QString viewportStyle(QString("QWidget { background-color: %1;").arg(BackgroundColor));

    if (BackgroundImageMode == BackgroundNone)
    {
        setAlternatingRowColors(true);
        style.append(QString("alternate-background-color: %1;").arg(AlternateBackgroundColor));
    }
    else
    {
        setAlternatingRowColors(false);

        if (BackgroundImageMode != BackgroundTiled && BackgroundImageMode != BackgroundTiledAndCentered)
            viewportStyle.append("background-repeat: no-repeat;");

        if (BackgroundImageMode == BackgroundCentered || BackgroundImageMode == BackgroundTiledAndCentered)
            viewportStyle.append("background-position: center;");

        if (BackgroundImageMode == BackgroundStretched)
        {
            // style.append("background-size: 100% 100%;"); will work in 4.6 maybe?
            QImage image(BackgroundImageFile);
            if (!image.isNull())
            {
                delete BackgroundTemporaryFile;
                BackgroundTemporaryFile = new QTemporaryFile(QDir::tempPath() + "/kadu_background_XXXXXX.png", this);

                if (BackgroundTemporaryFile->open())
                {
                    QImage stretched = image.scaled(
                        viewport()->width(), viewport()->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    if (stretched.save(BackgroundTemporaryFile, "PNG"))
                        viewportStyle.append(
                            QString("background-image: url(%1);").arg(BackgroundTemporaryFile->fileName()));
                    BackgroundTemporaryFile->close();
                }
            }
        }
        else
            viewportStyle.append(QString("background-image: url(%1);").arg(BackgroundImageFile));

        viewportStyle.append("background-attachment: fixed;");
    }

    style.append("}");
    viewportStyle.append("}");

    setStyleSheet(style);
    viewport()->setStyleSheet(viewportStyle);
}

void KaduTreeView::keyPressEvent(QKeyEvent *event)
{
    if (FilteredTreeView::shouldEventGoToFilter(event))
        event->ignore();
    else
        QTreeView::keyPressEvent(event);
}

void KaduTreeView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // QTreeView::resizeEvent(event);

    if (BackgroundImageMode == BackgroundStretched)
        updateBackground();

    scheduleDelayedItemsLayout();
}

void KaduTreeView::startDrag(Qt::DropActions supportedActions)
{
    const QModelIndexList &indexes = selectedIndexes();
    if (indexes.isEmpty())
        return;

    QMimeData *data = model()->mimeData(indexes);
    if (!data)
        return;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(data);

    drag->exec(supportedActions, Qt::LinkAction);
}
