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

#ifndef MU_NOTATION_LIVEBRAILLEMODEL_H
#define MU_NOTATION_LIVEBRAILLEMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "notation/inotationconfiguration.h"
#include "context/iglobalcontext.h"

#include "notation/notationtypes.h"

namespace mu::notation {
class LiveBrailleModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)    
    INJECT(notation, notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(QString liveBrailleInfo READ liveBrailleInfo NOTIFY liveBrailleInfoChanged)

public:
    explicit LiveBrailleModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString liveBrailleInfo() const;

signals:
    void liveBrailleInfoChanged();

private:
    notation::INotationPtr notation() const;
    notation::INotationLiveBraillePtr livebraille() const;


    void onCurrentNotationChanged();

    void listenChangesInLiveBraille();

};
}

#endif // MU_NOTATION_LIVEBRAILLEMODEL_H
