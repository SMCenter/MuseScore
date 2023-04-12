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
#include "translation.h"
#include "igetscore.h"
#include "notation.h"
#include "libmscore/masterscore.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"
#include "libmscore/factory.h"
#include "types/symnames.h"
#include "log.h"

#include "notationlivebraille.h"
#include "internal/livebraille/louis.h"
#include "internal/livebraille/braille.h"
#include "internal/livebraille/brailleinput.h"
#include "internal/livebraille/brailleinputparser.h"

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;
using namespace mu::io;

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

    setIntervalDirection(notationConfiguration()->intervalDirection());
    LOGD() << "interval direction: " << m_intervalDirection.val;
    notationConfiguration()->intervalDirectionChanged().onNotify(this, [this]() {
        QString direction = notationConfiguration()->intervalDirection();
        setIntervalDirection(direction);
        LOGD() << "interval direction updated: " << m_intervalDirection.val;
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
            if(isNavigationMode()) {
                setMode(LiveBrailleMode::BrailleInput);
            }
        } else {
            if(isBrailleInputMode()) {
                setMode(LiveBrailleMode::Navigation);
            }
        }
    });

    notation->interaction()->noteInput()->noteAdded().onNotify(this, [this]() {        
        if(currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
            LOGD() << "Note added: " << currentEngravingItem()->accessibleInfo();
            Note* note = toNote(currentEngravingItem());
            brailleInput()->setOctave(note->octave());
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

mu::ValCh<QString> NotationLiveBraille::intervalDirection() const
{
    return m_intervalDirection;
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

void NotationLiveBraille::setIntervalDirection(const QString direction)
{
    if (direction == m_enabled.val) {
        return;
    }
    m_intervalDirection.set(direction);
}

BrailleEngravingItemList* NotationLiveBraille::brailleEngravingItemList()
{
    return &m_beil;
}

QString NotationLiveBraille::getBrailleStr()
{
    return m_beil.brailleStr();
}

BrailleInputState * NotationLiveBraille::brailleInput()
{
    return &m_braille_input;
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

DurationType getDuration(const QString key)
{
    static const DurationType types[] = {
        DurationType::V_64TH, DurationType::V_32ND, DurationType::V_16TH,
        DurationType::V_EIGHTH, DurationType::V_QUARTER, DurationType::V_HALF,
        DurationType::V_WHOLE, DurationType::V_BREVE, DurationType::V_LONG
    };
    bool is_ok = false;
    int val = key.toInt(&is_ok);

    if(!is_ok || val < 1 || val > 9) {
        return DurationType::V_INVALID;
    } else {
        return types[val - 1];
    }
}

void NotationLiveBraille::setInputNoteDuration(Duration d)
{    
    InputState& inputState = score()->inputState();    
    inputState.setDuration(d);
    score()->setInputState(inputState);
    brailleInput()->setCurrentDuration(d.type());
}

void NotationLiveBraille::setTupletDuration(int tuplet, Duration d)
{
    LOGD() << tuplet << " " << (int)d.type();
    brailleInput()->setCurrentDuration(d.type());

    switch(tuplet) {
    case 2:
        d = d.shiftRetainDots(-1, false);
        d = d.shiftRetainDots(-1, true);
        break;
    case 3:
        d = d.shiftRetainDots(-1, false);
        break;
    case 5: case 6: case 7:
        d = d.shiftRetainDots(-2, false);
        break;
    case 8:
        d = d.shiftRetainDots(-2, false);
        d = d.shiftRetainDots(-1, true);
        break;
    case 9:
        d = d.shiftRetainDots(-3, false);
        break;
    }
    InputState& inputState = score()->inputState();
    inputState.setDuration(d);
    score()->setInputState(inputState);
}

static TupletOptions makeTupletOption(int num) {
    TupletOptions option;
    switch(num) {
    case 2:
        option.ratio = {2, 3};
        break;
    case 3:
        option.ratio = {3, 2};
        break;
    case 5:
        option.ratio = {5, 4};
        break;
    case 6:
        option.ratio = {6, 4};
        break;
    case 7:
        option.ratio = {7, 4};
        break;
    case 8:
        option.ratio = {8, 6};
        break;
    case 9:
        option.ratio = {9, 8};
        break;
    default:
        break;
    }
    return option;
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
        if(currentEngravingItem()) {
            interaction()->deleteSelection();            
        }
    } else if (seq == "N") {
        toggleMode();        
        brailleInput()->initialize();
    } else if (seq == "Plus") {
        if(isBrailleInputMode()) {
            interaction()->noteInput()->doubleNoteInputDuration();
         }
    } else if (seq == "Minus") {
        if(isBrailleInputMode()) {
            interaction()->noteInput()->halveNoteInputDuration();
        }    
    } else if(seq == "Space+F" && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group1);
        brailleInput()->resetBuffer();
    } else if(seq == "Space+D" && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group2);
        brailleInput()->resetBuffer();
    } else if(seq == "Space+S" && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group3);
        brailleInput()->resetBuffer();
    } else if(seq == "Space" && !brailleInput()->buffer().isEmpty() && isBrailleInputMode()) {
        LOGD() << brailleInput()->buffer();
        std::string braille = translate2Braille(brailleInput()->buffer().toStdString());
        BieRecognize(braille);
        BieSequencePatternType type = brailleInput()->parseBraille(getIntervalDirection());
        switch(type) {
        case BieSequencePatternType::Note: {
            if(brailleInput()->accidental() != mu::notation::AccidentalType::NONE) {
                interaction()->noteInput()->setAccidental(brailleInput()->accidental());
            }

            setVoice(brailleInput()->accord());

            if(brailleInput()->tupletNumber() != -1
                    && brailleInput()->tupletDuration().type() != DurationType::V_INVALID) {
                Duration duration = brailleInput()->tupletDuration();
                int tuplet = brailleInput()->tupletNumber();
                setTupletDuration(tuplet, duration);
                TupletOptions option = makeTupletOption(brailleInput()->tupletNumber());
                interaction()->noteInput()->addTuplet(option);
                brailleInput()->clearTuplet();
            }

            DurationType d = brailleInput()->getCloseDuration();
            Duration duration = Duration(d);
            if(brailleInput()->dots() > 0) {
                duration.setDots(brailleInput()->dots());
            }
            setInputNoteDuration(duration);

            interaction()->noteInput()->addNote(brailleInput()->noteName(), NoteAddingMode::NextChord);

            if(brailleInput()->addedOctave() != -1) {
                if(brailleInput()->addedOctave() < brailleInput()->octave()) {
                    for(int i = brailleInput()->addedOctave(); i < brailleInput()->octave(); i++){
                        interaction()->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
                    }
                } else if(brailleInput()->addedOctave() > brailleInput()->octave()) {
                    for(int i = brailleInput()->octave(); i < brailleInput()->addedOctave(); i++){
                        interaction()->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
                    }
                }
                brailleInput()->setOctave(brailleInput()->addedOctave());
            }
            if(brailleInput()->tie()) {
                if(currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                    Note* note = toNote(currentEngravingItem());
                    brailleInput()->setTieStartNote(note);
                }
            } else if(brailleInput()->tieStartNote() != NULL) {
                addTie();
                brailleInput()->clearTie();
            }
            if(brailleInput()->noteSlur()) {
                if(brailleInput()->slurStartNote() == NULL) {
                    if(currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                        Note* note = toNote(currentEngravingItem());
                        brailleInput()->setSlurStartNote(note);
                    }
                }
            } else if(brailleInput()->slurStartNote() != NULL) {
                addSlur();
                brailleInput()->clearSlur();
            }

            if(brailleInput()->longSlurStart()) {
                if(brailleInput()->longSlurStartNote() == NULL) {
                    if(currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                        Note* note = toNote(currentEngravingItem());
                        brailleInput()->setLongSlurStartNote(note);
                    }
                }
            }

            if(brailleInput()->longSlurStop()) {
                if(brailleInput()->longSlurStartNote() != NULL) {
                    addLongSlur();
                    brailleInput()->clearLongSlur();
                }
            }            

            playbackController()->playElements({ currentEngravingItem() });
            break;
        }
        case BieSequencePatternType::Rest: {            
            DurationType d = brailleInput()->getCloseDuration();
            Duration duration = Duration(d);
            if(brailleInput()->dots() > 0) {
                duration.setDots(brailleInput()->dots());
            }
            setInputNoteDuration(Duration(duration));
            interaction()->putRest(duration);
            break;
        }        
        case BieSequencePatternType::Interval: {            
            if(brailleInput()->accidental() != mu::notation::AccidentalType::NONE) {
                interaction()->noteInput()->setAccidental(brailleInput()->accidental());
            }
            interaction()->noteInput()->addNote(brailleInput()->noteName(), NoteAddingMode::CurrentChord);

            if(brailleInput()->addedOctave() != -1) {
                if(brailleInput()->addedOctave() < brailleInput()->octave()) {
                    for(int i = brailleInput()->addedOctave(); i < brailleInput()->octave(); i++){
                        interaction()->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
                    }
                } else if(brailleInput()->addedOctave() > brailleInput()->octave()) {
                    for(int i = brailleInput()->octave(); i < brailleInput()->addedOctave(); i++){
                        interaction()->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
                    }
                }
            }            
            playbackController()->playElements({ currentEngravingItem() });
            break;
        }
        case BieSequencePatternType::Tuplet: case BieSequencePatternType::Tuplet3: {
            std::string stateTuplet;
            stateTuplet = "Tuplet " + std::to_string(brailleInput()->tupletNumber());
            auto notationAccessibility = notation()->accessibility();
            if (!notationAccessibility) {
                return;
            }            
            notationAccessibility->setTriggeredCommand(stateTuplet);
            break;
        }
        default: {
            // TODO
        }
        }
        brailleInput()->reset();
    } else if(seq == "Space") {
        brailleInput()->reset();
    } else if(isBrailleInputMode() && !sequence.isEmpty()) {
        QString pattern = parseBrailleKeyInput(sequence);
        if(!pattern.isEmpty()) {
            brailleInput()->insertToBuffer(pattern);
        } else {
            brailleInput()->insertToBuffer(sequence);
        }
    }
}
bool NotationLiveBraille::addTie()
{    
    if(brailleInput()->tieStartNote() != NULL
       && currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
        score()->startCmd();
        Note* note = toNote(currentEngravingItem());

        Tie* tie = Factory::createTie(score()->dummy());
        tie->setStartNote(brailleInput()->tieStartNote());
        tie->setEndNote(note);
        tie->setTrack(brailleInput()->tieStartNote()->track());
        tie->setTick(brailleInput()->tieStartNote()->chord()->segment()->tick());
        tie->setTicks(note->chord()->segment()->tick() - brailleInput()->tieStartNote()->chord()->segment()->tick());
        score()->undoAddElement(tie);
        score()->endCmd();
        return true;
   } else {
        return false;
   }
}

bool NotationLiveBraille::addSlur()
{
    if(brailleInput()->slurStartNote() != NULL
       && currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
        Note* note1 = brailleInput()->slurStartNote();
        Note* note2 = toNote(currentEngravingItem());

        if(note1->parent()->isChordRest() && note2->parent()->isChordRest()) {
            LOGD() << "add slur";
            ChordRest* firstChordRest = toChordRest(note1->parent());
            ChordRest* secondChordRest = toChordRest(note2->parent());

            score()->startCmd();

            Slur* slur = Factory::createSlur(firstChordRest->measure()->system());
            slur->setScore(firstChordRest->score());
            slur->setTick(firstChordRest->tick());
            slur->setTick2(secondChordRest->tick());
            slur->setTrack(firstChordRest->track());
            if (secondChordRest->staff()->part() == firstChordRest->staff()->part()
                && !secondChordRest->staff()->isLinked(firstChordRest->staff())) {
                slur->setTrack2(secondChordRest->track());
            } else {
                slur->setTrack2(firstChordRest->track());
            }
            slur->setStartElement(firstChordRest);
            slur->setEndElement(secondChordRest);

            firstChordRest->score()->undoAddElement(slur);
            SlurSegment* ss = new SlurSegment(firstChordRest->score()->dummy()->system());
            ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            if (firstChordRest == secondChordRest) {
                ss->setSlurOffset(Grip::END, PointF(3.0 * firstChordRest->score()->spatium(), 0.0));
            }
            slur->add(ss);

            score()->endCmd();
            doLiveBraille(true);
            return true;
        }
        return false;
   } else {
        return false;
   }
}

bool NotationLiveBraille::addLongSlur()
{    
    if(brailleInput()->longSlurStartNote() != NULL
       && currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
        Note* note1 = brailleInput()->longSlurStartNote();
        Note* note2 = toNote(currentEngravingItem());

        if(note1->parent()->isChordRest() && note2->parent()->isChordRest()) {
            ChordRest* firstChordRest = toChordRest(note1->parent());
            ChordRest* secondChordRest = toChordRest(note2->parent());

            score()->startCmd();

            Slur* slur = Factory::createSlur(firstChordRest->measure()->system());
            slur->setScore(firstChordRest->score());
            slur->setTick(firstChordRest->tick());
            slur->setTick2(secondChordRest->tick());
            slur->setTrack(firstChordRest->track());
            if (secondChordRest->staff()->part() == firstChordRest->staff()->part()
                && !secondChordRest->staff()->isLinked(firstChordRest->staff())) {
                slur->setTrack2(secondChordRest->track());
            } else {
                slur->setTrack2(firstChordRest->track());
            }
            slur->setStartElement(firstChordRest);
            slur->setEndElement(secondChordRest);

            firstChordRest->score()->undoAddElement(slur);
            SlurSegment* ss = new SlurSegment(firstChordRest->score()->dummy()->system());
            ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            if (firstChordRest == secondChordRest) {
                ss->setSlurOffset(Grip::END, PointF(3.0 * firstChordRest->score()->spatium(), 0.0));
            }
            slur->add(ss);

            score()->endCmd();            
            doLiveBraille(true);
            return true;
        }
        return false;
   } else {        
        return false;
   }
}

bool NotationLiveBraille::setVoice(bool new_voice)
{
    if(currentEngravingItem() == NULL || currentMeasure() == NULL) {
        return false;
    }

    staff_idx_t staff = currentEngravingItem()->staffIdx();

    if(new_voice) {
        int voices = 0;
        for (size_t i = 1; i < VOICES; ++i) {
            if (current_measure->hasVoice(staff * VOICES + i)) {
                voices++;
            }
        }
        if(voices >= 3) {
            return false;
        }

        score()->inputState().moveInputPos(currentMeasure()->segments().firstCRSegment());
        interaction()->noteInput()->setCurrentVoice(voices +1);
        return true;
    } else {
        Segment* segment = score()->inputState().segment();
        Measure* measure = segment->measure();
        if(!measure->hasVoice(staff + 1) && segment->tick() == measure->tick()) {
            interaction()->noteInput()->setCurrentVoice(0);
        }
        return false;
    }
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
    std::string stateTitle;

    switch ((LiveBrailleMode)mode().val) {
        case LiveBrailleMode::Undefined:
        case LiveBrailleMode::Navigation:
            setMode(LiveBrailleMode::BrailleInput);            
            interaction()->noteInput()->startNoteInput();
            stateTitle = trc("notation", "Note input mode");
            break;
        case LiveBrailleMode::BrailleInput:
            setMode(LiveBrailleMode::Navigation);
            interaction()->noteInput()->endNoteInput();
            stateTitle = trc("notation", "Normal mode");
            break;
    }

    auto notationAccessibility = notation()->accessibility();
    if (!notationAccessibility) {
        return;
    }    
    notationAccessibility->setTriggeredCommand(stateTitle);
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

    bool play = current_engraving_item != e;
    current_engraving_item = e;
    if(isNavigationMode()) {        
        if(select) {            
            if(play) {
                playbackController()->playElements({e});
            }
            interaction()->select({e});
        }
    }    
}

IntervalDirection NotationLiveBraille::getIntervalDirection()
{
    if(m_intervalDirection.val == "Up")
        return IntervalDirection::Up;
    else if(m_intervalDirection.val == "Down")
        return IntervalDirection::Down;
    else if(m_intervalDirection.val == "Auto") {
        if(currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
            Note* note = toNote(currentEngravingItem());
            Staff* staff = note->staff();
            Fraction tick = note->tick();
            if(staff) {
                ClefType clef = staff->clef(tick);
                if(clef >= ClefType::G && clef <= ClefType::C3) {
                    return IntervalDirection::Down;
                } else {
                    return IntervalDirection::Up;
                }
            } else {
                return IntervalDirection::Down;
            }
        }
    }

    return IntervalDirection::Down;
}

EngravingItem* NotationLiveBraille::currentEngravingItem()
{
    return current_engraving_item;
}

Measure* NotationLiveBraille::currentMeasure()
{
    return current_measure;
}

