/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "select-talkable-popup.h"
#include "moc_select-talkable-popup.cpp"

#include "core/injected-factory.h"
#include "model/model-chain.h"
#include "model/roles.h"
#include "talkable/filter/hide-anonymous-talkable-filter.h"
#include "talkable/filter/name-talkable-filter.h"
#include "talkable/model/talkable-proxy-model.h"
#include "widgets/filter-widget.h"
#include "widgets/talkable-tree-view.h"

#include <QtWidgets/QLineEdit>

SelectTalkablePopup::SelectTalkablePopup(QWidget *parent) : FilteredTreeView(FilterAtBottom, parent, Qt::Popup)
{
    setAttribute(Qt::WA_WindowPropagation);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);
}

SelectTalkablePopup::~SelectTalkablePopup()
{
}

void SelectTalkablePopup::init()
{
    View = injectedFactory()->makeInjected<TalkableTreeView>(this);
    setView(View);

    Chain = new ModelChain(this);

    ProxyModel = injectedFactory()->makeInjected<TalkableProxyModel>(Chain);
    ProxyModel->setSortByStatusAndUnreadMessages(false);

    HideAnonymousFilter = new HideAnonymousTalkableFilter(ProxyModel);
    ProxyModel->addFilter(HideAnonymousFilter);

    NameTalkableFilter *nameFilter = new NameTalkableFilter(NameTalkableFilter::UndecidedMatching, ProxyModel);
    connect(this, SIGNAL(filterChanged(QString)), nameFilter, SLOT(setName(QString)));

    ProxyModel->addFilter(nameFilter);
    Chain->addProxyModel(ProxyModel);

    connect(View, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(View, SIGNAL(talkableActivated(Talkable)), this, SLOT(talkableActivated(Talkable)));

    View->setItemsExpandable(false);
    View->setChain(Chain);
    View->setRootIsDecorated(false);
    View->setShowIdentityNameIfMany(false);
    View->setSelectionMode(QAbstractItemView::SingleSelection);
}

QSize SelectTalkablePopup::sizeHint() const
{
    QSize newSizeHint = FilteredTreeView::sizeHint();
    newSizeHint.setHeight(2 * newSizeHint.height());
    return newSizeHint;
}

void SelectTalkablePopup::setBaseModel(QAbstractItemModel *model)
{
    Chain->setBaseModel(model);
}

void SelectTalkablePopup::setShowAnonymous(bool showAnonymous)
{
    HideAnonymousFilter->setEnabled(!showAnonymous);
}

void SelectTalkablePopup::show(const Talkable &talkable)
{
    filterWidget()->setFocus();

    QModelIndex currentIndex;

    if (!talkable.isEmpty())
    {
        const QModelIndexList &indexes = View->chain()->indexListForValue(QVariant::fromValue(talkable));
        if (!indexes.isEmpty())
        {
            Q_ASSERT(indexes.size() == 1);
            currentIndex = indexes.at(0);
        }
    }

    View->setCurrentIndex(currentIndex);

    FilteredTreeView::show();
}

void SelectTalkablePopup::itemClicked(const QModelIndex &index)
{
    talkableActivated(index.data(TalkableRole).value<Talkable>());
}

void SelectTalkablePopup::talkableActivated(const Talkable &talkable)
{
    emit talkableSelected(talkable);
    close();
}

void SelectTalkablePopup::addFilter(TalkableFilter *filter)
{
    ProxyModel->addFilter(filter);
}

void SelectTalkablePopup::removeFilter(TalkableFilter *filter)
{
    ProxyModel->removeFilter(filter);
}
