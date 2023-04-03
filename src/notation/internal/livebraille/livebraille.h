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
#ifndef MU_NOTATION_LIVEBRAILLE_LIVEBRAILLE_H
#define MU_NOTATION_LIVEBRAILLE_LIVEBRAILLE_H

#include <QIODevice>

#include "libmscore/engravingitem.h"

namespace mu::engraving {
class Score;
class Measure;
}

namespace mu::notation {
class BrailleEngravingItemList;
class LiveBrailleImpl;
class LiveBraille
{
public:
    LiveBraille(mu::engraving::Score* score);
    ~LiveBraille();

    bool writeMeasure(BrailleEngravingItemList*, mu::engraving::Measure* m);
    bool writeItem(BrailleEngravingItemList*, mu::engraving::EngravingItem* el);

private:
    LiveBrailleImpl* m_impl = nullptr;
};

#define MAX_LIVE_BRAILLE_LENGTH 10240

enum class BEIType
{
    Undefined = 0,
    EngravingItem,
    LyricItem,
    LineIndicator,
    VoiceInAccord,
    EndOfLine,
};

class BrailleEngravingItem
{
public:
    BrailleEngravingItem(BEIType, mu::engraving::EngravingItem* e, QString b);
    BrailleEngravingItem(BEIType, mu::engraving::EngravingItem* e, QString b,
                          QString extra_info, int extra_val);
    ~BrailleEngravingItem();

    BEIType type();
    mu::engraving::EngravingItem* el();
    QString braille();
    int start();
    int end();
    QString extra_info();
    int extra_val();

    void setBraille(QString b);
    void setPos(int s, int e);
    void setExtra(QString info, int val);

private:
    BEIType m_type;
    mu::engraving::EngravingItem* m_el;
    QString m_braille;
    int m_start, m_end;
    QString m_extra_info;
    int m_extra_val;
};

class BrailleEngravingItemList
{
public:
    BrailleEngravingItemList();
    ~BrailleEngravingItemList();

    void clear();

    void join(BrailleEngravingItemList*, bool newline = true, bool del = true);
    void join(std::vector<BrailleEngravingItemList*>, bool newline = true, bool del = true);

    QString brailleStr();
    std::vector<BrailleEngravingItem>* items();

    void setBrailleStr(QString str);    
    void insert(int pos, BrailleEngravingItem bei);
    int pos(BEIType type);

    void addEngravingItem(mu::engraving::EngravingItem*, QString braille);
    void addLyricsItem(mu::engraving::Lyrics*);

    bool isEmpty();
    BrailleEngravingItem* getItem(int pos);
    BrailleEngravingItem* getItem(mu::engraving::EngravingItem* e);

    void log();
private:
    QString m_braille_str;
    std::vector<BrailleEngravingItem> m_items;
};
}

#endif // MU_NOTATION_LIVEBRAILLE_LIVEBRAILLE_H
