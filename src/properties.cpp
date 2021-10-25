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

#include "common.hpp"

#include <gtkmm/separator.h>
#include <gtkmm/expander.h>
#include <gtkmm/label.h>


#include <vector>
#include <functional>

#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>

namespace {

static std::string fmtInt(int v)
{
    char buf[30];
    sprintf(buf, "%d", v);
    return buf;
}

static std::string fmtPixelMode(unsigned char mode)
{
    switch (mode)
    {
    case FT_PIXEL_MODE_NONE:  return "FT_PIXEL_MODE_NONE";
    case FT_PIXEL_MODE_MONO:  return "FT_PIXEL_MODE_MONO";
    case FT_PIXEL_MODE_GRAY:  return "FT_PIXEL_MODE_GRAY";
    case FT_PIXEL_MODE_GRAY2: return "FT_PIXEL_MODE_GRAY2";
    case FT_PIXEL_MODE_GRAY4: return "FT_PIXEL_MODE_GRAY4";
    case FT_PIXEL_MODE_LCD:   return "FT_PIXEL_MODE_LCD";
    case FT_PIXEL_MODE_LCD_V: return "FT_PIXEL_MODE_LCD_V";
    case FT_PIXEL_MODE_BGRA:  return "FT_PIXEL_MODE_BGRA";
    default: return "???";
    }
}

static std::string fmtGlyphFormat(FT_Glyph_Format format)
{
    switch (format)
    {
    case FT_GLYPH_FORMAT_NONE:      return "FT_GLYPH_FORMAT_NONE";
    case FT_GLYPH_FORMAT_COMPOSITE: return "FT_GLYPH_FORMAT_COMPOSITE";
    case FT_GLYPH_FORMAT_BITMAP:    return "FT_GLYPH_FORMAT_BITMAP";
    case FT_GLYPH_FORMAT_OUTLINE:   return "FT_GLYPH_FORMAT_OUTLINE";
    case FT_GLYPH_FORMAT_PLOTTER:   return "FT_GLYPH_FORMAT_PLOTTER";
    default: return "???";
    }
}

static std::string fmtFixed26_6(double val)
{
    char buf[100];
    sprintf(buf, "%.3f", val/64);
    return buf;
}

static std::string fmtFixed16_16(double val)
{
    char buf[100];
    sprintf(buf, "%.6f", val/65526);
    return buf;
}

}

Gtk::Widget& makePropertiesWidget(Signals &signals)
{
    auto *propsWrap = Gtk::make_managed<Gtk::ScrolledWindow>();
    propsWrap->set_margin_start(5);
    propsWrap->set_margin_end(5);
    propsWrap->set_margin_top(5);
    propsWrap->set_margin_bottom(5);

    propsWrap->set_vexpand();
    propsWrap->set_hexpand();
    propsWrap->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    propsWrap->show();

    auto *propsGrid = Gtk::make_managed<Gtk::Grid>();
    propsGrid->set_margin_start(5);
    propsGrid->set_margin_end(5);
    propsGrid->set_margin_top(5);
    propsGrid->set_margin_bottom(5);

    propsWrap->add(*propsGrid);


    propsGrid->show();
    propsGrid->set_column_spacing(5);

    Gtk::Grid *lastGrid = nullptr;

    auto addSeparator = [&]()
    {
        auto *sep = Gtk::make_managed<Gtk::HSeparator>();
        sep->set_hexpand();
        sep->set_margin_top(2);
        sep->set_margin_bottom(2);
        lastGrid->attach_next_to(*sep, Gtk::PositionType::POS_BOTTOM, 2, 1);
        sep->show();
    };

    auto addTitle = [&](const char *text)
    {
        std::ostringstream markup;
        markup << "<b>" << text << "</b>";
        auto *label = Gtk::make_managed<Gtk::Label>();
        label->set_markup(markup.str().c_str());
        label->set_xalign(0);
        label->show();

        auto *exp = Gtk::make_managed<Gtk::Expander>();
        exp->set_expanded();
        exp->set_label_widget(*label);
        exp->show();

        propsGrid->attach_next_to(*exp, Gtk::PositionType::POS_BOTTOM);

        auto *g = Gtk::make_managed<Gtk::Grid>();
        exp->add(*g);
        g->set_column_spacing(5);
        g->show();
        lastGrid = g;

        addSeparator();
    };

    auto addProp = [&](const char *label, std::function<std::string(FT_Face)> fn)
    {
        auto *nameLabel = Gtk::make_managed<Gtk::Label>(label);
        nameLabel->show();
        nameLabel->set_xalign(0);
        lastGrid->attach_next_to(*nameLabel, Gtk::PositionType::POS_BOTTOM);

        auto *valLabel = Gtk::make_managed<Gtk::Label>("");
        valLabel->show();
        valLabel->set_xalign(1);
        lastGrid->attach_next_to(*valLabel, *nameLabel, Gtk::PositionType::POS_RIGHT);

        signals.font_reloaded.connect([=](FT_Face face)
        {
            std::string res = fn(face);
            valLabel->set_label(res);
        });

        addSeparator();
    };

    // TODO normalize these
    addTitle("face");
    addProp("BBox xMin",          [](FT_Face f) { return fmtFixed26_6(f->bbox.xMin           ); });
    addProp("BBox yMin",          [](FT_Face f) { return fmtFixed26_6(f->bbox.yMin           ); });
    addProp("BBox xMax",          [](FT_Face f) { return fmtFixed26_6(f->bbox.xMax           ); });
    addProp("BBox yMax",          [](FT_Face f) { return fmtFixed26_6(f->bbox.yMax           ); });
    addProp("Units per EM",       [](FT_Face f) { return fmtFixed26_6(f->units_per_EM        ); });
    addProp("Ascender",           [](FT_Face f) { return fmtFixed26_6(f->ascender            ); });
    addProp("Descender",          [](FT_Face f) { return fmtFixed26_6(f->descender           ); });
    addProp("Height",             [](FT_Face f) { return fmtFixed26_6(f->height              ); });
    addProp("MaxAdvanceWidth",    [](FT_Face f) { return fmtFixed26_6(f->max_advance_width   ); });
    addProp("MaxAdvanceHeight",   [](FT_Face f) { return fmtFixed26_6(f->max_advance_height  ); });
    addProp("UnderlinePosition",  [](FT_Face f) { return fmtFixed26_6(f->underline_position  ); });
    addProp("UnderlineThickness", [](FT_Face f) { return fmtFixed26_6(f->underline_thickness ); });

    addTitle("face->glyph");
    addProp("glyph-index",       [](FT_Face f) { return fmtInt(        f->glyph->glyph_index       ); });
    addProp("linearHoriAdvance", [](FT_Face f) { return fmtFixed16_16( f->glyph->linearHoriAdvance ); });
    addProp("linearVertAdvance", [](FT_Face f) { return fmtFixed16_16( f->glyph->linearVertAdvance ); });
    addProp("advance.x",         [](FT_Face f) { return fmtFixed26_6(  f->glyph->advance.x         ); });
    addProp("advance.y",         [](FT_Face f) { return fmtFixed26_6(  f->glyph->advance.y         ); });
    addProp("format",            [](FT_Face f) { return fmtGlyphFormat(f->glyph->format            ); });
    addProp("bitmap_left",       [](FT_Face f) { return fmtInt(        f->glyph->bitmap_left       ); });
    addProp("bitmap_top",        [](FT_Face f) { return fmtInt(        f->glyph->bitmap_top        ); });
    // outline
    // num_subglyphs;
    // subglyphs;
    addProp("lsb_delta",         [](FT_Face f) { return fmtFixed26_6(  f->glyph->lsb_delta         ); });
    addProp("rsb_delta",         [](FT_Face f) { return fmtFixed26_6(  f->glyph->rsb_delta         ); });

    addTitle("face->glyph->bitmap");
    addProp("rows" ,      [](FT_Face f) { return fmtInt(       f->glyph->bitmap.rows        ); });
    addProp("width",      [](FT_Face f) { return fmtInt(       f->glyph->bitmap.width       ); });
    addProp("pitch",      [](FT_Face f) { return fmtInt(       f->glyph->bitmap.pitch       ); });
    addProp("num_grays",  [](FT_Face f) { return fmtInt(       f->glyph->bitmap.num_grays   ); });
    addProp("pixel_mode", [](FT_Face f) { return fmtPixelMode( f->glyph->bitmap.pixel_mode  ); });

    addTitle("face->glyph->metrics");
    addProp("width",        [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.width        ); });
    addProp("height",       [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.height       ); });
    addProp("horiBearingX", [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.horiBearingX ); });
    addProp("horiBearingY", [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.horiBearingY ); });
    addProp("horiAdvance",  [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.horiAdvance  ); });
    addProp("vertBearingX", [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.vertBearingX ); });
    addProp("vertBearingY", [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.vertBearingY ); });
    addProp("vertAdvance",  [](FT_Face f) { return fmtFixed26_6( f->glyph->metrics.vertAdvance  ); });

    return *propsWrap;
}
