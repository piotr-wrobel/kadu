/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "image-expander-dom-visitor-provider.h"
#include "moc_image-expander-dom-visitor-provider.cpp"

#include "image-expander.h"

#include "misc/memory.h"

ImageExpanderDomVisitorProvider::ImageExpanderDomVisitorProvider(QObject *parent)
        : QObject{parent}, m_ignoreLinksVisitor{std::make_unique<ImageExpander>()}
{
}

ImageExpanderDomVisitorProvider::~ImageExpanderDomVisitorProvider()
{
}

const DomVisitor *ImageExpanderDomVisitorProvider::provide() const
{
    return m_configuration.showImages() ? &m_ignoreLinksVisitor : nullptr;
}

void ImageExpanderDomVisitorProvider::setConfiguration(const ImageLinkConfiguration &configuration)
{
    m_configuration = configuration;
}
