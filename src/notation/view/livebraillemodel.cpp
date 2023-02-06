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

int LiveBrailleModel::cursorPosition() const
{
    return livebraille() ? livebraille()->cursorPosition().val : 0;
}

int LiveBrailleModel::currentItemPositionStart() const
{
    return livebraille() ? livebraille()->currentItemPositionStart().val : 0;
}

int LiveBrailleModel::currentItemPositionEnd() const
{
    return livebraille() ? livebraille()->currentItemPositionEnd().val : 0;
}

void LiveBrailleModel::setCursorPosition(int pos) const
{
    if (!livebraille()) {
        return;
    }

    if (livebraille()->cursorPosition().val == pos) {
        return;
    }

    LOGD("LiveBrailleModel::setCursorPosition %d", pos);

    livebraille()->setCursorPosition(pos);
    //emit currentItemChanged();
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
    listenCurrentItemChanges();
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

void LiveBrailleModel::listenCursorPositionChanges()
{
    LOGD("LiveBrailleModel::listenCursorPositionChanges");
    if (!livebraille()) {
        return;
    }

    //emit cursorPositionChanged();

    livebraille()->cursorPosition().ch.onReceive(this, [this](const int) {
        emit cursorPositionChanged();
    });
}

void LiveBrailleModel::listenCurrentItemChanges()
{
    LOGD("LiveBrailleModel::listenCurrentItemChanges");
    if (!livebraille()) {
        return;
    }

    //emit currentItemChanged();

    livebraille()->currentItemPositionStart().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });

    livebraille()->currentItemPositionEnd().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });
}

INotationPtr LiveBrailleModel::notation() const
{
    return context()->currentNotation();
}

INotationLiveBraillePtr LiveBrailleModel::livebraille() const
{
    return notation() ? notation()->livebraille() : nullptr;
}
