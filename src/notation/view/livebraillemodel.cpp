/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "livebraillemodel.h"

#include "types/translatablestring.h"

#include "log.h"

using namespace mu::notation;

LiveBrailleModel::LiveBrailleModel(QObject* parent)
    : QObject(parent)
{
    LOGD("LiveBrailleModel::LiveBrailleModel");
    load();
}

QString LiveBrailleModel::liveBrailleInfo() const
{
    //LOGD("LiveBrailleModel::liveBrailleInfo");
    return livebraille() ? QString::fromStdString(livebraille()->liveBrailleInfo().val) : QString();
}

void LiveBrailleModel::load()
{
    LOGD("LiveBrailleModel::load");
    TRACEFUNC;

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}
void LiveBrailleModel::onCurrentNotationChanged()
{    
    if (!notation()) {
        return;
    }

    listenChangesInLiveBraille();
}

void LiveBrailleModel::listenChangesInLiveBraille()
{
    LOGD("LiveBrailleModel::listenChangesInLiveBraille");
    if (!livebraille()) {
        return;
    }

    emit liveBrailleInfoChanged();

    livebraille()->liveBrailleInfo().ch.onReceive(this, [this](const std::string&) {
        emit liveBrailleInfoChanged();
    });
}

INotationPtr LiveBrailleModel::notation() const
{
    return context()->currentNotation();
}

INotationLiveBraillePtr LiveBrailleModel::livebraille() const
{
    LOGD("LiveBrailleModel::livebraille");
    return notation() ? notation()->livebraille() : nullptr;
}
