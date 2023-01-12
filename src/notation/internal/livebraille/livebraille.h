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

namespace mu::engraving {
class Score;
class Measure;
}

namespace mu::notation::livebraille {
class LiveBrailleImpl;
class LiveBraille
{
public:
    LiveBraille(mu::engraving::Score* score);
    ~LiveBraille();

    bool write(QIODevice& destinationDevice);
    bool writeMeasure(QIODevice& destinationDevice, mu::engraving::Measure * m);

private:
    LiveBrailleImpl* m_impl = nullptr;
};
}

#endif // MU_IMPORTEXPORT_EXPORTBRAILLE_H
