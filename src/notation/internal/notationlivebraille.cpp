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

#include "libmscore/masterscore.h"
#include "libmscore/spanner.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sig.h"
#include "libmscore/measure.h"

#include "internal/livebraille/livebraille.h"
#include "internal/livebraille/louis.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;
using namespace mu::io;
using namespace mu::notation::livebraille;

NotationLiveBraille::NotationLiveBraille(const Notation* notation)
    : m_getScore(notation)
{
    MScore::debugMode = true;
    setCurrentItemPosition(-1, -1);

    path_t tablesdir = tablesDefaultDirPath();
    setTablesDir(tablesdir.toStdString().c_str());
    initTables(tablesdir.toStdString());

    bool check_table_failed = true;
    if (check_tables(table_ascii_to_unicode.c_str()) == -1) {
        std::string msg = "Check tables " + table_ascii_to_unicode + " failed!";
        setLiveBrailleInfo(msg.c_str());
    } else if (check_tables(table_unicode_to_ascii.c_str()) == -1) {
        std::string msg = "Check tables " + table_unicode_to_ascii + " failed!";
        setLiveBrailleInfo(msg.c_str());
    } else if (check_tables(table_for_literature.c_str()) == -1) {
        std::string msg = "Check tables " + table_for_literature + " failed!";
        setLiveBrailleInfo(msg.c_str());
        //} else if(check_tables(table_for_general.c_str()) == -1) {
        //    std::string msg = "Check tables " + table_for_general + " failed!";
        //    setLiveBrailleInfo(msg.c_str());
    } else {
        //setLiveBrailleInfo("Check tables succeeded!");
        check_table_failed = false;
    }
    if (!check_table_failed) {
        std::string welcome = braille_translate(table_for_literature.c_str(), "Welcome to MuseScore 4.0!");
        setLiveBrailleInfo(QString(welcome.c_str()));
    }
    //setLiveBrailleInfo(QString(tablesdir.toStdString().c_str()));

    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        EngravingItem* e = nullptr;
        Measure* m = nullptr;

        if (selection()->isSingle()) {
            e = selection()->element();
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
            if (!m) {
                brailleEngravingItems()->clear();
                LiveBraille lb(score());
                bool res = lb.writeItem(brailleEngravingItems(), e);
                if (!res) {
                    QString txt = e->accessibleInfo();
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItems()->setBrailleStr(QString::fromStdString(braille));
                    setLiveBrailleInfo(QString::fromStdString(braille));
                } else {
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                }
                current_measure = nullptr;
            } else {
                if (m != current_measure) {
                    brailleEngravingItems()->clear();
                    LiveBraille lb(score());
                    lb.writeMeasure(brailleEngravingItems(), m);
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                    current_measure = m;
                }
                std::pair<int, int> pos = brailleEngravingItems()->getBraillePos(e);
                if (pos.first != -1) {
                    setCurrentItemPosition(pos.first, pos.second);
                }
            }
        }
    });

    notation->notationChanged().onNotify(this, [this]() {
        //updateLiveBrailleInfo();
        EngravingItem* e = nullptr;
        Measure* m = nullptr;

        if (selection()->isSingle()) {
            e = selection()->element();
        } else if (selection()->isRange()) {
            Measure* m = nullptr;
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
            if (!m) {
                brailleEngravingItems()->clear();
                LiveBraille lb(score());
                bool res = lb.writeItem(brailleEngravingItems(), e);
                if (!res) {
                    QString txt = e->accessibleInfo();
                    LOGD() << "type " << (int)e->type() << " :" << txt;
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItems()->setBrailleStr(QString::fromStdString(braille));
                    setLiveBrailleInfo(QString::fromStdString(braille));
                } else {
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                }
                current_measure = nullptr;
            } else {
                if (m != current_measure) {
                    brailleEngravingItems()->clear();
                    LiveBraille lb(score());
                    lb.writeMeasure(brailleEngravingItems(), m);
                    setLiveBrailleInfo(brailleEngravingItems()->brailleStr());
                    current_measure = m;
                }
                std::pair<int, int> pos = brailleEngravingItems()->getBraillePos(e);
                if (pos.first != -1) {
                    setCurrentItemPosition(pos.first, pos.second);
                }
            }
        }
        //LOGD("notationChanged()");
    });
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

/*
void NotationLiveBraille::setMapToScreenFunc(const AccessibleMapToScreenFunc& func)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    score()->rootItem()->accessible()->accessibleRoot()->setMapToScreenFunc(func);
    score()->dummy()->rootItem()->accessible()->accessibleRoot()->setMapToScreenFunc(func);
#else
    UNUSED(func)
#endif
}
*/

void NotationLiveBraille::setEnabled(bool enabled)
{
    UNUSED(enabled)
}

void NotationLiveBraille::setTriggeredCommand(const std::string& command)
{
#ifndef ENGRAVING_NO_LIVEBRAILLE
//    score()->rootItem()->accessible()->accessibleRoot()->setCommandInfo(QString::fromStdString(command));
//    score()->dummy()->rootItem()->accessible()->accessibleRoot()->setCommandInfo(QString::fromStdString(command));
    UNUSED(command)
#else
    UNUSED(command)
#endif
}

livebraille::BrailleEngravingItems* NotationLiveBraille::brailleEngravingItems()
{
    return &m_bei;
}

QString NotationLiveBraille::getBrailleStr()
{
    return m_bei.brailleStr();
}

void NotationLiveBraille::updateLiveBrailleInfo()
{
    if (!score()) {
        return;
    }

    QString newLiveBrailleInfo = QStringLiteral("Live braille here");

    setLiveBrailleInfo(newLiveBrailleInfo);
}

void NotationLiveBraille::updateCursorPosition()
{
    if (!score()) {
        return;
    }
    // TODO
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
    LOGD("%d", pos);

    if (m_cursorPosition.val == pos) {
        return;
    }

    m_cursorPosition.set(pos);

    notation::EngravingItem* el = brailleEngravingItems()->getEngravingItem(pos);
    if (el != nullptr) {
        LOGD() << el->accessibleInfo();
        //el->setSelected(true);
        score()->select(el);
    } else {
        LOGD() << "Item not found";
    }
}

void NotationLiveBraille::setCurrentItemPosition(const int start, const int end)
{
    LOGD("NotationLiveBraille::setCurrentItemPosition %d:%d", start, end);
    if (m_currentItemPositionStart.val == start
        && m_currentItemPositionEnd.val == end) {
        return;
    }

    m_currentItemPositionStart.set(start);
    m_currentItemPositionEnd.set(end);
}

path_t NotationLiveBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "tables";
}
