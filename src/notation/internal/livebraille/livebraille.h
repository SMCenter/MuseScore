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
#ifndef MU_IMPORTEXPORT_EXPORTBRAILLE_H
#define MU_IMPORTEXPORT_EXPORTBRAILLE_H

#include <QIODevice>

#include "libmscore/engravingitem.h"

namespace mu::engraving {
class Score;
class Measure;
}

namespace mu::notation::livebraille {
class BrailleEngravingItems;
class LiveBrailleImpl;
class LiveBraille
{
public:
    LiveBraille(mu::engraving::Score* score);
    ~LiveBraille();

    bool writeMeasure(BrailleEngravingItems * , mu::engraving::Measure * m);
    bool writeItem(BrailleEngravingItems * , mu::engraving::EngravingItem * el);

private:
    LiveBrailleImpl* m_impl = nullptr;
};

class BrailleEngravingItems {
public:
    BrailleEngravingItems();
    ~BrailleEngravingItems();

    void clear();

    void join(BrailleEngravingItems *, bool newline = true, bool del = true);
    void join(std::vector<BrailleEngravingItems *>, bool newline = true, bool del = true);

    QString brailleStr();
    std::vector<std::pair<mu::engraving::EngravingItem*, std::pair<int, int>>> * items();

    void setBrailleStr(QString str);
    void addPrefixStr(QString str);

    void addEngravingItem(mu::engraving::EngravingItem*, QString braille);
    void addLyricsItem(mu::engraving::Lyrics*);

    bool isEmpty() {return braille_str.isEmpty(); }
    engraving::EngravingItem* getEngravingItem(int pos);
    std::pair<int, int> getBraillePos(engraving::EngravingItem * e);

    void log();
private:    
    QString braille_str;
    std::vector<std::pair<mu::engraving::EngravingItem*, std::pair<int, int>>> _items;
};
}

#endif // MU_IMPORTEXPORT_EXPORTBRAILLE_H
