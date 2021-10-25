// Copyright 2021 Mustafa Serdar Sanli
//
// This file is part of FontDebug.
//
// FontDebug is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FontDebug is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FontDebug.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "common.hpp"

#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>

struct FreetypeBitmapDrawer : public Gtk::DrawingArea
{
    FreetypeBitmapDrawer(FT_Face &face, Signals &signals);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

public: // TODO
    FT_Face &m_face;
    Signals &m_signals;

    bool m_drawGrayscaleLCD = false;
    bool m_drawBaseline = false;
    bool m_drawGrid = false;
    bool m_drawOutline = false;

    bool m_isHorizontal = true;

    double lastX, lastY;
    bool hadMotion;

    int lastWidth = -1;
    int lastHeight = -1;

    bool m_transformMatrixInitialized = false;
    Cairo::Matrix m_transformMatrix;

    bool pointSelected = false;
    bool pointSignalEmitted = false;
    int selX, selY;
};
