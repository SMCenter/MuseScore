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

#include "livebraillepreferencesmodel.h"

#include "log.h"

using namespace mu::appshell;

LiveBraillePreferencesModel::LiveBraillePreferencesModel(QObject* parent)
    : QObject(parent)
{
    std::sort(m_table_list.begin(), m_table_list.end());
}

void LiveBraillePreferencesModel::load()
{
}

bool LiveBraillePreferencesModel::liveBrailleStatus() const
{
    return notationConfiguration()->liveBrailleStatus();
}

void LiveBraillePreferencesModel::setLiveBrailleStatus(bool value)
{
    if (value == liveBrailleStatus()) {
        return;
    }

    notationConfiguration()->setLiveBrailleStatus(value);
    emit liveBrailleStatusChanged(value);
}

QString LiveBraillePreferencesModel::liveBrailleTable() const
{
    return notationConfiguration()->liveBrailleTable();
}

void LiveBraillePreferencesModel::setLiveBrailleTable(QString table) {
    if (table == liveBrailleTable()) {
        return;
    }

    notationConfiguration()->setLiveBrailleTable(table);
    emit liveBrailleTableChanged(table);
}
QVariantList LiveBraillePreferencesModel::tables() const
{
    QVariantList result;

    for(auto val : m_table_list)
        result << val;

    return result;
}
