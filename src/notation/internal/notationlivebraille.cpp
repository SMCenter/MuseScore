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

#include "io/iodevice.h"
#include "io/buffer.h"

#include "notationlivebraille.h"

#include "translation.h"

#include "igetscore.h"
#include "notation.h"
#include "internal/notationactioncontroller.h"

#include "libmscore/masterscore.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"

#include "internal/livebraille/livebraille.h"
#include "internal/livebraille/louis.hpp"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;
using namespace mu::io;
using namespace mu::notation::livebraille;

NotationLiveBraille::NotationLiveBraille(const Notation* notation)
    : m_getScore(notation)    
{    
    setEnabled(notationConfiguration()->liveBrailleStatus());
    setCurrentItemPosition(-1, -1);

    path_t tablesdir = tablesDefaultDirPath();    

    setTablesDir(tablesdir.toStdString().c_str());
    initTables(tablesdir.toStdString());

    std::string welcome = braille_translate(table_for_literature.c_str(), "Welcome to MuseScore 4.0!");
    setLiveBrailleInfo(QString(welcome.c_str()));

    notationConfiguration()->liveBrailleStatusChanged().onNotify(this, [this]() {
        bool enabled = notationConfiguration()->liveBrailleStatus();
        setEnabled(enabled);
    });

    notationConfiguration()->liveBrailleTableChanged().onNotify(this, [this]() {
        QString table = notationConfiguration()->liveBrailleTable();        
        int startPos = table.indexOf('[');
        int endPos = table.indexOf(']');
        if(startPos != -1 && endPos != -1) {
            table = table.mid(startPos + 1, endPos - startPos - 1);
            table = QString::fromStdString(tables_dir) + "/" + table;
            if(check_tables(table.toStdString().c_str()) == 0) {
                LOGD() << "Table ok: " << table;
                updateTableForLyrics(table.toStdString());
            } else {
                LOGD() << "Table check error!";
            }
        }
    });

    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        doLiveBraille();
    });

    notation->notationChanged().onNotify(this, [this]() {
        doLiveBraille(true);
    });

}

void NotationLiveBraille::doLiveBraille(bool force)
{
    if(notationConfiguration()->liveBrailleStatus()) {        
        EngravingItem * e = nullptr;
        Measure * m = nullptr;

        if(selection()->isSingle()) {
            e = selection()->element();
            m = e->findMeasure();
        } else if(selection()->isRange()) {
            for(auto el: selection()->elements()) {
                if(el->isMeasure()) {
                    m = toMeasure(el);
                    break;
                 } else {
                    m = el->findMeasure();
                    if(m) break;
                }
            }
            e = m ? m : selection()->elements().front();
        } else if(selection()->isList()) {
            e = selection()->elements().front();
            m = e->findMeasure();
        }
        if(e) {
            if(!m) {
                brailleEngravingItems()->clear();
                LiveBraille lb(score());
                bool res = lb.writeItem(brailleEngravingItems(), e);
                if(!res) {
                    QString txt = e->accessibleInfo();
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItems()->setBrailleStr(QString::fromStdString(braille));
                    setLiveBrailleInfo(QString::fromStdString(braille));
                } else {
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                }
                current_measure = nullptr;
            } else {                
                if(m != current_measure || force) {
                    brailleEngravingItems()->clear();
                    LiveBraille lb(score());
                    lb.writeMeasure(brailleEngravingItems(), m);
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                    current_measure = m;
                }                
                std::pair<int, int> pos = brailleEngravingItems()->getBraillePos(e);
                if(pos.first != -1) {
                    setCurrentItemPosition(pos.first, pos.second);
                }                
            }
        }
    }
}

mu::engraving::Score* NotationLiveBraille::score()
{
    return m_getScore->score();
}

mu::engraving::Selection* NotationLiveBraille::selection()
{
    return &score()->selection();
}

mu::ValCh<std::string> NotationLiveBraille::liveBrailleInfo() const
{
    return m_liveBrailleInfo;
}

mu::ValCh<int> NotationLiveBraille::cursorPosition() const
{
    return m_cursorPosition;
}
mu::ValCh<int> NotationLiveBraille::currentItemPositionStart() const
{
    return m_currentItemPositionStart;
}
mu::ValCh<int> NotationLiveBraille::currentItemPositionEnd() const
{
    return m_currentItemPositionEnd;
}
mu::ValCh<std::string> NotationLiveBraille::shortcut() const
{
    return m_shortcut;
}

mu::ValCh<bool> NotationLiveBraille::enabled() const
{
    return m_enabled;
}

void NotationLiveBraille::setEnabled(bool enabled)
{
   if(enabled == m_enabled.val) {
       return;
   }
   m_enabled.set(enabled);
}

livebraille::BrailleEngravingItems * NotationLiveBraille::brailleEngravingItems(){
    return &m_bei;
}

QString NotationLiveBraille::getBrailleStr() {
    return m_bei.brailleStr();
}

void NotationLiveBraille::setLiveBrailleInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_liveBrailleInfo.val == infoStd) {
        return;
    }

    m_liveBrailleInfo.set(infoStd);
}

void NotationLiveBraille::setCursorPosition(const int pos)
{
    if (m_cursorPosition.val == pos) {
        return;
    }

    m_cursorPosition.set(pos);

    notation::EngravingItem * el = brailleEngravingItems()->getEngravingItem(pos);
    if(el != nullptr) {                
        interaction()->select({el});        
    }
}

void NotationLiveBraille::setCurrentItemPosition(const int start, const int end)
{    
    if (m_currentItemPositionStart.val == start &&
            m_currentItemPositionEnd.val == end) {
        return;
    }

    m_currentItemPositionStart.set(start);
    m_currentItemPositionEnd.set(end);
}


INotationPtr NotationLiveBraille::notation() {
    return context()->currentNotation();
}

INotationInteractionPtr NotationLiveBraille::interaction() {
    return notation() ? notation()->interaction() : nullptr;
}

void NotationLiveBraille::setShortcut(const QString &sequence)
{
    LOGD() << sequence;
    std::string seq = sequence.toStdString();
    m_shortcut.set(seq);       

    if(seq == "Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
    } else if(seq == "Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
    } else if(seq == "Ctrl+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
    } else if(seq == "Ctrl+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
    } else if(seq == "Alt+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::EngravingItem);
    } else if(seq == "Alt+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::EngravingItem);
    } else if(seq == "Ctrl+End") {
        interaction()->selectLastElement();
    } else if(seq == "Ctrl+Home") {
        interaction()->selectFirstElement();
    }// else if(shortcutsController()->isRegistered(seq)) {
     //   shortcutsController()->activate(seq);
     //}
}

path_t NotationLiveBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "tables";
}

