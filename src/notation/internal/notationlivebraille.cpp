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

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;
using namespace mu::io;
using namespace mu::notation::livebraille;

NotationLiveBraille::NotationLiveBraille(const Notation* notation)
    : m_getScore(notation)
{
    path_t tablesdir = tablesDefaultDirPath();
    setTablesDir(tablesdir.toStdString().c_str());

    std::string welcome = braille_translate("unicode.dis,en-us-g2.ctb", "Welcome to MuseScore 4.0!");
    setLiveBrailleInfo(QString(welcome.c_str()));

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
        }
        if (e) {
            if (!m) {
                QString txt = e->accessibleInfo();
                std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                setLiveBrailleInfo(QString::fromStdString(braille));
                crmeasure = m;
            } else {
                if (m != crmeasure) {
                    QString txt;
                    if (m) {
                        QBuffer buf;
                        buf.open(QBuffer::WriteOnly);
                        LiveBraille lb(score());
                        lb.writeMeasure(buf, m);
                        txt = QString(buf.buffer());
                        setLiveBrailleInfo(txt);
                    } else {
                        txt = QString();
                    }
                    crmeasure = m;
                } else {
                    // TODO: set selected braille text for item
                }
            }
        }
    });

    notation->notationChanged().onNotify(this, [this]() {
        //updateLiveBrailleInfo();
        EngravingItem* e = nullptr;
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
        }

        if (e) {
            QString txt;
            Measure* m = e->findMeasure();
            if (m) {
                //txt = score()->getMusicXmlMeasure4Element(e);
                txt = QString("musicxml text");
            } else {
                //txt = QString(notation->accessibility().accessibilityInfo().val);
                txt = QString("Accessibility text");
            }
            setLiveBrailleInfo(txt);
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

void NotationLiveBraille::updateLiveBrailleInfo()
{
    if (!score()) {
        return;
    }

    QString newLiveBrailleInfo = QStringLiteral("Live braille here");

    setLiveBrailleInfo(newLiveBrailleInfo);
}

void NotationLiveBraille::setLiveBrailleInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_liveBrailleInfo.val == infoStd) {
        return;
    }

    m_liveBrailleInfo.set(infoStd);
}

path_t NotationLiveBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "/tables";
}
