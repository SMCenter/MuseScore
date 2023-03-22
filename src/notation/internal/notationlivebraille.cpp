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

#include "types/symnames.h"

#include "internal/livebraille/louis.h"
#include "internal/livebraille/braille.h"
#include "internal/livebraille/brailleinput.h"

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

    setMode(LiveBrailleMode::Navigation);

    path_t tablesdir = tablesDefaultDirPath();
    setTablesDir(tablesdir.toStdString().c_str());
    initTables(tablesdir.toStdString());

    std::string welcome = braille_translate(table_for_literature.c_str(), "Welcome to MuseScore 4!");
    setLiveBrailleInfo(QString(welcome.c_str()));

    notationConfiguration()->liveBrailleStatusChanged().onNotify(this, [this]() {
        bool enabled = notationConfiguration()->liveBrailleStatus();
        setEnabled(enabled);
    });

    updateTableForLyricsFromPreferences();
    notationConfiguration()->liveBrailleTableChanged().onNotify(this, [this]() {
        updateTableForLyricsFromPreferences();
    });

    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        LOGD() << "Selection changed";
        doLiveBraille();
    });

    notation->notationChanged().onNotify(this, [this]() {
        LOGD() << "Notation changed";
        setCurrentItemPosition(0, 0);
        doLiveBraille(true);        
    });

    notation->interaction()->noteInput()->stateChanged().onNotify(this, [this]() {
        if(interaction()->noteInput()->isNoteInputMode()) {
            setMode(LiveBrailleMode::BrailleInput);
            m_braille_input.initialize();
        }
    });

    notation->interaction()->noteInput()->noteAdded().onNotify(this, [this]() {
        LOGD() << "Note added ";
        if(current_engraving_item != NULL && current_engraving_item->isNote()) {
            LOGD() << current_engraving_item->accessibleInfo();
            Note* note = toNote(current_engraving_item);
            m_braille_input.setAddedOctave(note->octave());
        }
    });
}

void NotationLiveBraille::updateTableForLyricsFromPreferences()
{
    QString table = notationConfiguration()->liveBrailleTable();
    int startPos = table.indexOf('[');
    int endPos = table.indexOf(']');
    if (startPos != -1 && endPos != -1) {
        table = table.mid(startPos + 1, endPos - startPos - 1);
        QString table_full_path = QString::fromStdString(tables_dir) + "/" + table;
        if (check_tables(table_full_path.toStdString().c_str()) == 0) {
            updateTableForLyrics(table.toStdString());
        } else {
            LOGD() << "Table check error!";
        }
    }
}

void NotationLiveBraille::doLiveBraille(bool force)
{
    if (notationConfiguration()->liveBrailleStatus()) {
        EngravingItem* e = NULL;
        Measure* m = NULL;

        if (selection()->isSingle()) {
            e = selection()->element();
            LOGD() << " selected " << e->accessibleInfo();
            m = e->findMeasure();
        } else if (selection()->isRange()) {
            for (auto el: selection()->elements()) {
                if (el->isMeasure()) {
                    m = toMeasure(el);
                    break;
                } else {
                    m = el->findMeasure();
                    if (m) {
                        break;
                    }
                }
            }
            e = m ? m : selection()->elements().front();
        } else if (selection()->isList()) {
            e = selection()->elements().front();
            m = e->findMeasure();
        }        

        if (e) {            
            setCurrentEngravingItem(e, false);

            if (!m) {
                brailleEngravingItemList()->clear();
                LiveBraille lb(score());
                bool res = lb.writeItem(brailleEngravingItemList(), e);
                if (!res) {
                    QString txt = e->accessibleInfo();
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItemList()->setBrailleStr(QString::fromStdString(braille));
                    setLiveBrailleInfo(QString::fromStdString(braille));
                } else {
                    setLiveBrailleInfo(brailleEngravingItemList()->brailleStr());
                }
                current_measure = NULL;
            } else {
                if (m != current_measure || force) {
                    brailleEngravingItemList()->clear();
                    LiveBraille lb(score());
                    lb.writeMeasure(brailleEngravingItemList(), m);
                    setLiveBrailleInfo(brailleEngravingItemList()->brailleStr());
                    current_measure = m;
                }

                current_bei = brailleEngravingItemList()->getItem(e);
                if (current_bei != NULL) {                    
                    setCurrentItemPosition(current_bei->start(), current_bei->end());
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

mu::ValCh<std::string> NotationLiveBraille::keys() const
{
    return m_keys;
}

mu::ValCh<bool> NotationLiveBraille::enabled() const
{
    return m_enabled;
}

mu::ValCh<int> NotationLiveBraille::mode() const
{
    return m_mode;
}

mu::ValCh<std::string> NotationLiveBraille::cursorColor() const
{
    return m_cursorColor;
}

void NotationLiveBraille::setEnabled(bool enabled)
{
    if (enabled == m_enabled.val) {
        return;
    }
    m_enabled.set(enabled);
}

BrailleEngravingItemList* NotationLiveBraille::brailleEngravingItemList()
{
    return &m_beil;
}

QString NotationLiveBraille::getBrailleStr()
{
    return m_beil.brailleStr();
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
    if(!isNavigationMode()) return;

    if (m_cursorPosition.val == pos || pos == 0) {
        return;
    }

    m_cursorPosition.set(pos);

    current_bei = brailleEngravingItemList()->getItem(pos);
    if (current_bei != NULL && current_bei->el() != NULL
        && (current_bei->type() == BEIType::EngravingItem
            || current_bei->type() == BEIType::LyricItem)) {
            //interaction()->select({current_bei->el()});
            setCurrentEngravingItem(current_bei->el(), true);
    }
}

void NotationLiveBraille::setCurrentItemPosition(const int start, const int end)
{        

    if (m_currentItemPositionStart.val == start
        && m_currentItemPositionEnd.val == end) {
        return;
    }

    m_currentItemPositionStart.set(start);
    m_currentItemPositionEnd.set(end);
}

INotationPtr NotationLiveBraille::notation()
{
    return context()->currentNotation();
}

INotationInteractionPtr NotationLiveBraille::interaction()
{
    return notation() ? notation()->interaction() : NULL;
}

bool isDigit(const QString key)
{
    bool is_ok = false;
    int val = key.toInt(&is_ok);
    return (is_ok && val > 0 && val < 10);
}

Duration getDuration(const QString key)
{
    static const DurationType types[] = {
        DurationType::V_64TH, DurationType::V_32ND, DurationType::V_16TH,
        DurationType::V_EIGHTH, DurationType::V_QUARTER, DurationType::V_HALF,
        DurationType::V_WHOLE, DurationType::V_BREVE, DurationType::V_LONG
    };
    bool is_ok = false;
    int val = key.toInt(&is_ok);

    if(!is_ok || val < 1 || val > 9) {
        return Duration(DurationType::V_INVALID);
    } else {
        return Duration(types[val - 1]);
    }
}

void NotationLiveBraille::setInputNoteDuration(Duration d)
{
    NoteInputState state = interaction()->noteInput()->state();

    if(state.duration > d) {
        while(state.duration > d) {
            interaction()->noteInput()->halveNoteInputDuration();
            state = interaction()->noteInput()->state();
        }
    } else if(state.duration < d) {
        while(state.duration < d) {
            interaction()->noteInput()->doubleNoteInputDuration();
            state = interaction()->noteInput()->state();
        }
    }
}

void NotationLiveBraille::setKeys(const QString& sequence)
{
    LOGD() << sequence;
    std::string seq = sequence.toStdString();
    m_keys.set(seq);

    if (seq == "Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
    } else if (seq == "Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
    } else if (seq == "Ctrl+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
    } else if (seq == "Ctrl+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
    } else if (seq == "Alt+Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::EngravingItem);
    } else if (seq == "Alt+Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::EngravingItem);
    } else if (seq == "Ctrl+End") {
        interaction()->selectLastElement();
    } else if (seq == "Ctrl+Home") {
        interaction()->selectFirstElement();
    } else if (seq == "Delete") {
        if(current_engraving_item) {
            interaction()->deleteSelection();            
        }
    } else if (seq == "N") {
        toggleMode();
        if(isBrailleInputMode()) {
            interaction()->noteInput()->startNoteInput();
        } else {
            interaction()->noteInput()->endNoteInput();
        }
        m_braille_input.initialize();
    } else if (seq == "Plus") {
        if(isBrailleInputMode()) {
            interaction()->noteInput()->doubleNoteInputDuration();
         }
    } else if (seq == "Minus") {
        if(isBrailleInputMode()) {
            interaction()->noteInput()->halveNoteInputDuration();
        }
    } else if(isDigit(sequence)) {
        if(isBrailleInputMode()) {
            Duration d = getDuration(sequence);
            setInputNoteDuration(d);
        }
    } else if(isBrailleInputMode()) {        
        QString pattern = parseBrailleKeyInput(sequence);
        if(!pattern.isEmpty()) {
            m_braille_input.insertToBuffer(pattern);
        }        
        BraillePatternType type = m_braille_input.parseBraille();
        LOGD() << m_braille_input.buffer() << " >> " << (int)type;
        switch(type) {
            case BraillePatternType::Note: {
                if(m_braille_input.accidental() != mu::notation::AccidentalType::NONE) {
                    interaction()->noteInput()->setAccidental(m_braille_input.accidental());
                }                                
                interaction()->noteInput()->addNote(m_braille_input.notename(), NoteAddingMode::NextChord);

                if(m_braille_input.addedOctave() != -1) {
                    if(m_braille_input.addedOctave() < m_braille_input.octave()) {
                        for(int i = m_braille_input.addedOctave(); i < m_braille_input.octave(); i++){
                            interaction()->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
                        }
                    } else if(m_braille_input.addedOctave() > m_braille_input.octave()) {
                        for(int i = m_braille_input.octave(); i < m_braille_input.addedOctave(); i++){
                            interaction()->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
                        }
                    }
                }
                m_braille_input.reset();
                break;
            }
            case BraillePatternType::NoteTie: case BraillePatternType::ChordTie: {
                LOGD() << "adding tie...";
                interaction()->noteInput()->addTie();
                m_braille_input.reset();
                break;
            }
            /*
            case BraillePatternType::Slur: {
                LOGD() << "Trying adding slur...";
                LOGD() << " start: " << current_engraving_item->typeName();
                if(current_engraving_item->isChordRest()) {
                    LOGD() << "adding slur...";
                    ChordRest* cr = toChordRest(current_engraving_item);
                    Slur * slur = score()->addSlur(cr, NULL, NULL);
                    interaction()->noteInput()->addSlur(slur);
                }
                m_braille_input.initialize();
                break;
            }
            */
            case BraillePatternType::Interval: {
                BraillePattern btp = recognizeBrailleInput(m_braille_input.buffer());
                int interval = getInterval(btp.data.front());
                if(interval != -1) {
                    LOGD() << " add interval " << interval;
                    interaction()->addIntervalToSelectedNotes(interval);
                }
                m_braille_input.initialize();
            }
            /*
            case BraillePatternType::LongSlurStart: {
                break;
            }
            case BraillePatternType::LongSlurStop: {
                break;
            }
            case BraillePatternType::InAccord: {
                // add new voice
                break;
            }
            */
            default: {
                break;
            }
        }
    }
}
bool NotationLiveBraille::addVoice()
{
    LOGD() << "Try adding a voice...";
    if(current_engraving_item == NULL) {
        return false;
    }

    staff_idx_t staff = current_engraving_item->staffIdx();

    int voices = 0;
    for (size_t i = 1; i < VOICES; ++i) {
        if (current_measure->hasVoice(staff * VOICES + i)) {
            voices++;
        }
    }
    if(voices >= 4) {
        return false;
    }



    return true;
}
void NotationLiveBraille::setMode(const LiveBrailleMode mode)
{    
    switch (mode) {
        case LiveBrailleMode::Undefined:
        case LiveBrailleMode::Navigation:
            setCursorColor("black");
            break;
        case LiveBrailleMode::BrailleInput:
            setCursorColor("green");
            break;        
    }

    m_mode.set((int)mode);
}

void NotationLiveBraille::toggleMode()
{
    switch ((LiveBrailleMode)mode().val) {
        case LiveBrailleMode::Undefined:
        case LiveBrailleMode::Navigation:
            setMode(LiveBrailleMode::BrailleInput);
            break;
        case LiveBrailleMode::BrailleInput:
            setMode(LiveBrailleMode::Navigation);
            break;
    }
}

bool NotationLiveBraille::isNavigationMode()
{
    return mode().val == (int)LiveBrailleMode::Navigation;
}

bool NotationLiveBraille::isBrailleInputMode()
{
    return mode().val == (int)LiveBrailleMode::BrailleInput;
}

void NotationLiveBraille::setCursorColor(const QString color)
{
    LOGD() << color;
    m_cursorColor.set(color.toStdString());
}


path_t NotationLiveBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "tables";
}

void NotationLiveBraille::setCurrentEngravingItem(EngravingItem* e, bool select)
{
    if(!e) {
        return;
    }

    current_engraving_item  = e;

    if(isNavigationMode()) {        
        if(select) {
            interaction()->select({e});
        }
    }
}

