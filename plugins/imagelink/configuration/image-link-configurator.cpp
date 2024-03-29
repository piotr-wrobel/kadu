/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/core.h"

#include "configuration/image-link-configuration.h"
#include "image-expander-dom-visitor-provider.h"
#include "video-expander-dom-visitor-provider.h"

#include "image-link-configurator.h"
#include "moc_image-link-configurator.cpp"

ImageLinkConfigurator::ImageLinkConfigurator(QObject *parent)
{
    Q_UNUSED(parent);
}

ImageLinkConfigurator::~ImageLinkConfigurator()
{
}

void ImageLinkConfigurator::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ImageLinkConfigurator::setImageExpanderProvider(ImageExpanderDomVisitorProvider *imageExpander)
{
    m_imageExpander = imageExpander;
}

void ImageLinkConfigurator::setVideoExpanderProvider(VideoExpanderDomVisitorProvider *videoExpander)
{
    m_videoExpander = videoExpander;
}

void ImageLinkConfigurator::init()
{
    createDefaultConfiguration();
}

void ImageLinkConfigurator::configure()
{
    configurationUpdated();
}

void ImageLinkConfigurator::createDefaultConfiguration()
{
    m_configuration->deprecatedApi()->addVariable("Imagelink", "show_image", true);
    m_configuration->deprecatedApi()->addVariable("Imagelink", "show_yt", true);
}

void ImageLinkConfigurator::configurationUpdated()
{
    ImageLinkConfiguration configuration;
    configuration.setShowImages(m_configuration->deprecatedApi()->readBoolEntry("Imagelink", "show_image", true));
    configuration.setShowVideos(m_configuration->deprecatedApi()->readBoolEntry("Imagelink", "show_yt", true));

    if (m_imageExpander)
        m_imageExpander->setConfiguration(configuration);
    if (m_videoExpander)
        m_videoExpander->setConfiguration(configuration);
}
