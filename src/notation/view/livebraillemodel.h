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
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int currentItemPositionStart READ currentItemPositionStart NOTIFY currentItemChanged)
    Q_PROPERTY(int currentItemPositionEnd READ currentItemPositionEnd NOTIFY currentItemChanged)
    Q_PROPERTY(QString shorcut READ shortcut WRITE setShortcut NOTIFY shortcutFired)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit LiveBrailleModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString liveBrailleInfo() const;    

    int cursorPosition() const;
    void setCursorPosition(int pos) const;

    int currentItemPositionStart() const;
    int currentItemPositionEnd() const;

    QString shortcut() const;
    void setShortcut(const QString & sequence) const;

    bool enabled() const;
    void setEnabled(bool e) const;

signals:
    void liveBrailleInfoChanged() const;
    void cursorPositionChanged() const;
    void currentItemChanged() const;
    void shortcutFired() const;
    void enabledChanged() const;

private:
    notation::INotationPtr notation() const;
    notation::INotationLiveBraillePtr livebraille() const;

    void onCurrentNotationChanged();

    void listenChangesInLiveBraille();
    void listenCursorPositionChanges();
    void listenCurrentItemChanges();        
    void listenShortcuts();
    void listenEnabledChanges();
};
}

#endif // MU_NOTATION_LIVEBRAILLEMODEL_H
