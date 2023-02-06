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

#ifndef MU_NOTATION_NOTATIONLIVEBRAILLE_H
#define MU_NOTATION_NOTATIONLIVEBRAILLE_H

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "ui/iuiconfiguration.h"
#include "engraving/iengravingconfiguration.h"

#include "inotationlivebraille.h"
#include "notationtypes.h"

#include "async/asyncable.h"
#include "async/notification.h"

#include "internal/livebraille/livebraille.h"

namespace Ms {
class Score;
class Selection;
}

namespace mu::notation {
class IGetScore;
class Notation;
class NotationLiveBraille : public INotationLiveBraille, public async::Asyncable
{
    INJECT(notation, framework::IGlobalConfiguration, globalConfiguration)
public:
    NotationLiveBraille(const Notation* notation);

    ValCh<std::string> liveBrailleInfo() const override;
    ValCh<int> cursorPosition() const override;
    ValCh<int> currentItemPositionStart() const override;
    ValCh<int> currentItemPositionEnd() const override;

    void setEnabled(bool enabled) override;
    void setTriggeredCommand(const std::string& command) override;

    void setCursorPosition(const int pos) override;
    void setCurrentItemPosition(const int, const int) override;

    livebraille::BrailleEngravingItems* brailleEngravingItems();
    QString getBrailleStr();
private:
    engraving::Score* score();
    engraving::Selection* selection();

    Measure* current_measure = nullptr;

    void updateLiveBrailleInfo();
    void updateCursorPosition();

    void setLiveBrailleInfo(const QString& info);

    io::path_t tablesDefaultDirPath() const;

    const IGetScore* m_getScore = nullptr;

    ValCh<std::string> m_liveBrailleInfo;
    ValCh<int> m_cursorPosition;
    ValCh<int> m_currentItemPositionStart;
    ValCh<int> m_currentItemPositionEnd;

    livebraille::BrailleEngravingItems m_bei;
    async::Notification m_selectionChanged;
};
}

#endif // MU_NOTATION_NOTATIONLIVEBRAILLE_H
