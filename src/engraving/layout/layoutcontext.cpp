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
#include "layoutcontext.h"

#include "libmscore/score.h"
#include "libmscore/spanner.h"

using namespace mu::engraving;
using namespace Ms;

LayoutStateContext::LayoutStateContext(Score* score)
    : m_score(score)
{
}

LayoutStateContext::~LayoutStateContext()
{
    for (Spanner* s : processedSpanners) {
        s->layoutSystemsDone();
    }

    for (MuseScoreView* v : score()->getViewer()) {
        v->layoutChanged();
    }
}
