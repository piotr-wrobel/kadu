/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "image-link-plugin-object.h"
#include "moc_image-link-plugin-object.cpp"

#include "configuration/image-link-configurator.h"
#include "image-expander-dom-visitor-provider.h"
#include "video-expander-dom-visitor-provider.h"

#include "dom/dom-visitor-provider-repository.h"
#include "misc/paths-provider.h"
#include "windows/main-configuration-window-service.h"
#include "windows/main-configuration-window.h"

ImageLinkPluginObject::ImageLinkPluginObject(QObject *parent) : QObject{parent}
{
}

ImageLinkPluginObject::~ImageLinkPluginObject()
{
}

void ImageLinkPluginObject::setDomVisitorProviderRepository(DomVisitorProviderRepository *domVisitorProviderRepository)
{
    m_domVisitorProviderRepository = domVisitorProviderRepository;
}

void ImageLinkPluginObject::setImageExpanderDomVisitorProvider(
    ImageExpanderDomVisitorProvider *imageExpanderDomVisitorProvider)
{
    m_imageExpanderDomVisitorProvider = imageExpanderDomVisitorProvider;
}

void ImageLinkPluginObject::setImageLinkConfigurator(ImageLinkConfigurator *imageLinkConfigurator)
{
    m_imageLinkConfigurator = imageLinkConfigurator;
}

void ImageLinkPluginObject::setMainConfigurationWindowService(
    MainConfigurationWindowService *mainConfigurationWindowService)
{
    m_mainConfigurationWindowService = mainConfigurationWindowService;
}

void ImageLinkPluginObject::setPathsProvider(PathsProvider *pathsProvider)
{
    m_pathsProvider = pathsProvider;
}

void ImageLinkPluginObject::setVideoExpanderDomVisitorProvider(
    VideoExpanderDomVisitorProvider *videoExpanderDomVisitorProvider)
{
    m_videoExpanderDomVisitorProvider = videoExpanderDomVisitorProvider;
}

void ImageLinkPluginObject::init()
{
    m_mainConfigurationWindowService->registerUiFile(
        m_pathsProvider->dataPath() + QStringLiteral("plugins/configuration/image-link.ui"));
    m_domVisitorProviderRepository->addVisitorProvider(m_imageExpanderDomVisitorProvider, -100);
    m_domVisitorProviderRepository->addVisitorProvider(m_videoExpanderDomVisitorProvider, -50);
    m_imageLinkConfigurator->configure();
}

void ImageLinkPluginObject::done()
{
    m_domVisitorProviderRepository->removeVisitorProvider(m_videoExpanderDomVisitorProvider);
    m_domVisitorProviderRepository->removeVisitorProvider(m_imageExpanderDomVisitorProvider);
    m_mainConfigurationWindowService->unregisterUiFile(
        m_pathsProvider->dataPath() + QStringLiteral("plugins/configuration/image-link.ui"));
}
