#ifndef MUSICKWDOWNLOADTEXTREQUEST_H
#define MUSICKWDOWNLOADTEXTREQUEST_H

/***************************************************************************
 * This file is part of the TTK Music Player project
 * Copyright (C) 2015 - 2022 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#include "musicabstractdownloadrequest.h"

/*! @brief The class of the download the type of kuwo txt.
 * @author Greedysky <greedysky@163.com>
 */
class TTK_MODULE_EXPORT MusicKWDownLoadTextRequest : public MusicAbstractDownLoadRequest
{
    Q_OBJECT
    TTK_DECLARE_MODULE(MusicKWDownLoadTextRequest)
public:
    /*!
     * Object contsructor provide download url save local path and download type.
     */
    MusicKWDownLoadTextRequest(const QString &url, const QString &path, QObject *parent = nullptr);

    /*!
     * Start to download data from net.
     */
    virtual void startRequest() override final;

public Q_SLOTS:
    /*!
     * Download data from net finished.
     */
    virtual void downLoadFinished() override final;

};

#endif // MUSICKWDOWNLOADTEXTREQUEST_H
