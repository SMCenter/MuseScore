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
    if(!livebraille()) {
        livebraille()->setMode(LiveBrailleMode::Navigation);
    }
    load();
}

QString LiveBrailleModel::liveBrailleInfo() const
{
    return livebraille() ? QString::fromStdString(livebraille()->liveBrailleInfo().val) : QString();
}

QString LiveBrailleModel::keys() const
{
    return livebraille() ? QString::fromStdString(livebraille()->keys().val) : QString();
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
    if(!livebraille()) return;

    if (livebraille()->cursorPosition().val == pos)
        return;

    livebraille()->setCursorPosition(pos);
}

void LiveBrailleModel::setKeys(const QString & sequence) const
{
    if(!livebraille()) return;
    livebraille()->setKeys(sequence);
}

bool LiveBrailleModel::enabled() const
{
    //if(!livebraille()) return false;
    //return livebraille()->enabled().val;
    return notationConfiguration()->liveBrailleStatus();
}

void LiveBrailleModel::setEnabled(bool e) const
{
    if (!livebraille()) {
        return;
    }
    if (livebraille()->enabled().val == e) {
        return;
    }
    livebraille()->setEnabled(e);
    emit liveBrailleStatusChanged();
}

int LiveBrailleModel::mode() const
{
    return livebraille()->mode().val;
}

void LiveBrailleModel::setMode(int m) const
{
    if(!livebraille()) {
        return;
    }

    if(livebraille()->mode().val == m) {
        return;
    }

    livebraille()->setMode((LiveBrailleMode)m);

    emit liveBrailleModeChanged();
}

bool LiveBrailleModel::isNavigationMode()
{
    if(!livebraille()) {
        return false;
    }
    return livebraille()->isNavigationMode();
}

bool LiveBrailleModel::isBrailleInputMode()
{
    if(!livebraille()) {
        return false;
    }
    return livebraille()->isBrailleInputMode();
}

bool LiveBrailleModel::isBrailleEditMode()
{
    if(!livebraille()) {
        return false;
    }
    return livebraille()->isBrailleEditMode();
}

QString LiveBrailleModel::cursorColor() const
{
    QString color = livebraille() ? QString::fromStdString(livebraille()->cursorColor().val) : QString();
    LOGD() << color;
    return color;
}

void LiveBrailleModel::load()
{
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
    listenKeys();
    listenLiveBrailleStatusChanges();
    listenLiveBrailleModeChanges();
    listenCursorPositionChanges();
}

void LiveBrailleModel::listenChangesInLiveBraille()
{
    if (!livebraille()) {
        return;
    }

    livebraille()->liveBrailleInfo().ch.onReceive(this, [this](const std::string&) {
        emit liveBrailleInfoChanged();
    });
}

void LiveBrailleModel::listenKeys()
{
    if (!livebraille()) {
        return;
    }

    livebraille()->keys().ch.onReceive(this, [this](const std::string&) {
        emit keysFired();
    });
}

void LiveBrailleModel::listenCursorPositionChanges()
{
    if (!livebraille()) {
        return;
    }
    livebraille()->cursorPosition().ch.onReceive(this, [this](const int) {
        emit cursorPositionChanged();
    });
}

void LiveBrailleModel::listenCurrentItemChanges()
{
    if (!livebraille()) {
        return;
    }
    livebraille()->currentItemPositionStart().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });

    livebraille()->currentItemPositionEnd().ch.onReceive(this, [this](int) {
        emit currentItemChanged();
    });
}

void LiveBrailleModel::listenLiveBrailleStatusChanges()
{
    if (!livebraille()) {
        return;
    }
    livebraille()->enabled().ch.onReceive(this, [this](const int) {
        emit liveBrailleStatusChanged();
    });
}

void LiveBrailleModel::listenLiveBrailleModeChanges()
{
    if (!livebraille()) {
        return;
    }
    livebraille()->mode().ch.onReceive(this, [this](const int) {
        emit liveBrailleModeChanged();
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
