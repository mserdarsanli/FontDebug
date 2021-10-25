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

#include <glibmm/main.h>
#include <freetype/freetype.h>
#include <gtkmm/widget.h>
#include <cairomm/context.h>

struct Signals {
    sigc::signal<void(FT_Face)> font_reloaded;
    sigc::signal<void(Cairo::Matrix)> glyph_transform_updated;
    sigc::signal<void(int, uint32_t)> pixel_selected;
};

Gtk::Widget& makePropertiesWidget(Signals &);
