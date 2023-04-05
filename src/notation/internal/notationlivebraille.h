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
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"

#include "internal/livebraille/livebraille.h"
#include "internal/livebraille/brailleinput.h"

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
    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, notation::INotationConfiguration, notationConfiguration)
public:
    NotationLiveBraille(const Notation* notation);

    void doLiveBraille(bool force = false);

    // Braille input
    bool addNote();
    bool addVoice();
    bool addSlurStart();
    bool addSlurEnd();
    bool addTie();
    bool addSlur();
    bool addLongSlur();
    bool addTuplet(const mu::notation::TupletOptions& options);

    bool incDuration();
    bool decDuration();

    bool setArticulation();

    void setInputNoteDuration(Duration d);

    ValCh<std::string> liveBrailleInfo() const override;
    ValCh<int> cursorPosition() const override;
    ValCh<int> currentItemPositionStart() const override;
    ValCh<int> currentItemPositionEnd() const override;
    ValCh<std::string> keys() const override;
    ValCh<bool> enabled() const override;
    ValCh<QString> intervalDirection() const override;
    ValCh<int> mode() const override;
    ValCh<std::string> cursorColor() const override;

    void setEnabled(const bool enabled) override;
    void setIntervalDirection(const QString direction) override;

    void setCursorPosition(const int pos) override;
    void setCurrentItemPosition(const int, const int) override;
    void setKeys(const QString&) override;

    void setMode(const LiveBrailleMode) override;
    void toggleMode() override;
    bool isNavigationMode() override;
    bool isBrailleInputMode() override;    

    void setCursorColor(const QString color) override;

    EngravingItem* currentEngravingItem();

    INotationPtr notation();
    INotationInteractionPtr interaction();

    BrailleEngravingItemList* brailleEngravingItemList();
    QString getBrailleStr();

    BrailleInputState * brailleInput();
private:
    engraving::Score* score();    
    engraving::Selection* selection();

    Notation* m_notation;
    Measure* current_measure = NULL;
    EngravingItem* current_engraving_item = NULL;
    BrailleEngravingItem* current_bei = NULL;

    void setLiveBrailleInfo(const QString& info);
    void setCurrentEngravingItem(EngravingItem* el, bool select);

    void updateTableForLyricsFromPreferences();
    io::path_t tablesDefaultDirPath() const;

    IntervalDirection getIntervalDirection();


    BrailleInputState m_braille_input;

    const IGetScore* m_getScore = nullptr;

    ValCh<std::string> m_liveBrailleInfo;
    ValCh<int> m_cursorPosition;
    ValCh<int> m_currentItemPositionStart;
    ValCh<int> m_currentItemPositionEnd;
    ValCh<std::string> m_keys;
    ValCh<bool> m_enabled;
    ValCh<int> m_mode;
    ValCh<QString> m_intervalDirection;
    ValCh<std::string> m_cursorColor;

    BrailleEngravingItemList m_beil;
    async::Notification m_selectionChanged;
};
}
#endif // MU_NOTATION_NOTATIONLIVEBRAILLE_H
