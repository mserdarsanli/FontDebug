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

#include "drawer.hpp"

#include <iostream>

#include <glibmm/main.h>

#include <gtkmm/checkbutton.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>

#include <freetype/ftoutln.h>

bool FreetypeBitmapDrawer::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    lastWidth = allocation.get_width();
    lastHeight = allocation.get_height();

    int pitch = m_face->glyph->bitmap.pitch;

    auto &bitmap = m_face->glyph->bitmap;

    double pixelWidth = 1.0;
    double pixelHeight = 1.0;
    int bitmapWidth = bitmap.width;
    int bitmapHeight = bitmap.rows;

    switch (bitmap.pixel_mode)
    {
    case FT_PIXEL_MODE_LCD:
        pixelWidth /= 3;
        bitmapWidth /= 3;
        break;
    case FT_PIXEL_MODE_LCD_V:
        pixelHeight /= 3;
        bitmapHeight /= 3;
        break;
    case FT_PIXEL_MODE_GRAY:
    case FT_PIXEL_MODE_MONO:
    case FT_PIXEL_MODE_BGRA:
        break;
    default:
        std::cerr << "Unhandled pixel mode: " << (int)bitmap.pixel_mode << "\n";
        return true;
    };

    if (!m_transformMatrixInitialized)
    {
        // Set initial transform matrix such that initial glyph is centered and scaled to view
        m_transformMatrixInitialized = true;

        if (bitmapWidth == 0 || bitmapHeight == 0)
        {
            m_transformMatrix = Cairo::identity_matrix();
            m_transformMatrix.scale(30, 30);
        }
        else
        {
            double scale = std::min(lastWidth * 0.75 / bitmapWidth, lastHeight * 0.75 / bitmapHeight);

            m_transformMatrix = Cairo::identity_matrix();
            m_transformMatrix.scale(scale, scale);
            m_transformMatrix.translate(
                -m_face->glyph->bitmap_left -bitmapWidth / 2.0,
                m_face->glyph->bitmap_top - bitmapHeight / 2.0);
        }
    }

    cr->transform(m_transformMatrix * Cairo::translation_matrix(lastWidth * 0.5, lastHeight * 0.5));
    cr->set_line_width(0.1);

    cr->save();
    cr->set_source_rgba(0, 0, 0, 1);
    cr->paint();
    cr->restore();

    cr->save();
    cr->translate(m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top);

    for (int y = 0; y < bitmap.rows; ++y)
    {
        const uint8_t *row_buf = bitmap.buffer + y * bitmap.pitch;

        if (bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
        {
            for (int x = 0; x < bitmap.width; ++x)
            {
                double b = row_buf[x*4 + 0] / 255.0;
                double g = row_buf[x*4 + 1] / 255.0;
                double r = row_buf[x*4 + 2] / 255.0;
                double a = row_buf[x*4 + 3] / 255.0;

                // Make pixels overlap a bit to get rid of rendering artifacts
                double overlap = 0.01;
                cr->set_source_rgb(r, g, b);
                cr->move_to(-overlap + pixelWidth * x, -overlap + pixelHeight * y);
                cr->rel_line_to(0, pixelHeight + 2*overlap);
                cr->rel_line_to(pixelWidth + 2*overlap, 0);
                cr->rel_line_to(0, -pixelHeight - 2*overlap);
                cr->fill();
            }
        }
        else
        {
            for (int x = 0; x < bitmap.width; ++x)
            {
                double gray = 0;

                double r, g, b;
                if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    uint32_t byte = row_buf[x/8];
                    byte &= (((uint32_t)1) << (7-(x%8)));
                    gray = byte ? 1.0 : 0.0;
                    r = b = g= gray;
                }
                else
                {
                    gray = row_buf[x] / 255.0;
                    if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY || m_drawGrayscaleLCD)
                    {
                        r = g = b = gray;
                    }
                    else if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
                    {
                        r = (x % 3 == 0) * gray;
                        g = (x % 3 == 1) * gray;
                        b = (x % 3 == 2) * gray;
                    }
                    else if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD_V)
                    {
                        r = (y % 3 == 0) * gray;
                        g = (y % 3 == 1) * gray;
                        b = (y % 3 == 2) * gray;
                    }
                }

                double overlap = 0.01;
                cr->set_source_rgb(r, g, b);
                cr->move_to(-overlap + pixelWidth * x, -overlap + pixelHeight * y);
                cr->rel_line_to(0, pixelHeight + 2*overlap);
                cr->rel_line_to(pixelWidth + 2*overlap, 0);
                cr->rel_line_to(0, -pixelHeight - 2*overlap);
                cr->fill();
            }
        }
    }
    cr->restore();

    if (m_drawGrid)
    {
        int extend = 10;
        double x1 = 0 - extend;
        double y1 = 0 - extend;
        double x2 = bitmapWidth + extend;
        double y2 = bitmapHeight + extend;

        cr->save();
        cr->set_source_rgb(0.15, 0.15, 0.15);
        cr->translate(m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top);

        for (int i = x1; i <= x2; ++i)
        {
            cr->move_to(i, y1);
            cr->line_to(i, y2);
        }
        for (int i = y1; i <= y2; ++i)
        {
            cr->move_to(x1, i);
            cr->line_to(x2, i);
        }
        cr->stroke();
        cr->restore();
    }

    {
        double x1 = 0;
        double y1 = 0;
        double x2 = bitmapWidth;
        double y2 = bitmapHeight;
        cr->save();
        cr->translate(m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top);

        cr->set_source_rgb(0.4, 0.4, 0.4);
        cr->move_to(x1, y1);
        cr->line_to(x2, y1);
        cr->line_to(x2, y2);
        cr->line_to(x1, y2);
        cr->close_path();
        cr->stroke();

        cr->restore();
    }

    if (m_drawOutline)
    {
        FT_Outline &outline = m_face->glyph->outline;
        if (outline.n_points)
        {
            cr->save();
            cr->set_source_rgb(45.0/255, 206.0/255, 160.0/255);

            bool path_open = true;

            struct UserData
            {
                const Cairo::RefPtr<Cairo::Context>* cr;
                std::function<void()> close_current_path;
            };

            FT_Outline_Funcs funcs;
            funcs.move_to = [](const FT_Vector *to, void *user_) -> int
            {
                UserData *user = reinterpret_cast<UserData*>(user_);

                user->close_current_path();
                const Cairo::RefPtr<Cairo::Context> &cr = *user->cr;
                cr->move_to(to->x / 64.0, -to->y / 64.0);
                return 0;
            };
            funcs.line_to = [](const FT_Vector *to, void *user_) -> int
            {
                UserData *user = reinterpret_cast<UserData*>(user_);

                const Cairo::RefPtr<Cairo::Context> &cr = *user->cr;
                cr->line_to(to->x / 64.0, -to->y / 64.0);
                return 0;
            };
            funcs.conic_to = [](const FT_Vector *control, const FT_Vector *to, void *user_) -> int
            {
                UserData *user = reinterpret_cast<UserData*>(user_);

                const Cairo::RefPtr<Cairo::Context> &cr = *user->cr;

                // Elevate degree to use with cubic cairo curve fn
                // Adapted from https://lists.cairographics.org/archives/cairo/2010-April/019691.html

                double x0, y0;
                cr->get_current_point(x0, y0);
                double x1 = control->x / 64.0;
                double y1 = -control->y / 64.0;
                double x2 = to->x / 64.0;
                double y2 = -to->y / 64.0;

                cr->curve_to(
                  2.0 / 3.0 * x1 + 1.0 / 3.0 * x0,
                  2.0 / 3.0 * y1 + 1.0 / 3.0 * y0,
                  2.0 / 3.0 * x1 + 1.0 / 3.0 * x2,
                  2.0 / 3.0 * y1 + 1.0 / 3.0 * y2,
                  x2, y2);

                return 0;
            };
            funcs.cubic_to = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user_) -> int
            {
                UserData *user = reinterpret_cast<UserData*>(user_);

                const Cairo::RefPtr<Cairo::Context> &cr = *user->cr;
                cr->curve_to(control1->x / 64.0, -control1->y / 64.0, control2->x / 64.0, -control2->y / 64.0, to->x / 64.0, -to->y / 64.0);
                return 0;
            };
            funcs.shift = 0;
            funcs.delta = 0;


            UserData user_data;
            user_data.cr = &cr;
            user_data.close_current_path = [&]()
            {
                if (path_open)
                {
                    cr->close_path();
                    cr->stroke();
                }
                path_open = true;
            };

            FT_Outline_Decompose(&outline, &funcs, (void*)&user_data);

            user_data.close_current_path();
            cr->restore();
        }
    }

    if (m_drawBaseline)
    {
        cr->save();

        cr->set_source_rgb(0, 1, 1);

        if (m_isHorizontal)
        {
            cr->arc(0, 0, 0.1, 0, 2 * M_PI);
            cr->fill();

            cr->move_to(0, 0);
            cr->line_to(m_face->glyph->advance.x * (1.0 / 64), -m_face->glyph->advance.y * (1.0 / 64));
            cr->stroke();
        }
        else
        {
            cr->translate(-m_face->glyph->metrics.vertBearingX / 64,
                -m_face->glyph->bitmap_top - m_face->glyph->metrics.vertBearingY / 64);
            cr->arc(0, 0, 0.1, 0, 2 * M_PI);
            cr->fill();

            cr->move_to(0, 0);
            cr->line_to(-m_face->glyph->advance.x * (1.0 / 64), m_face->glyph->advance.y * (1.0 / 64));
            cr->stroke();
        }

        cr->restore();
    }

    if (pointSelected)
    {
        cr->save();
        cr->translate(selX, selY);

        cr->set_source_rgb(0.0, 1.0, 0.0);
        cr->move_to(0, 0);
        cr->line_to(1, 0);
        cr->line_to(1, 1);
        cr->line_to(0, 1);
        cr->close_path();
        cr->stroke();

        cr->restore();

        if (pointSignalEmitted == false)
        {
            int imgX = selX - m_face->glyph->bitmap_left;
            int imgY = selY + m_face->glyph->bitmap_top;

            if (imgY < 0 || imgY >= bitmapHeight || imgX < 0 || imgX >= bitmapWidth)
            {
                m_signals.pixel_selected.emit(0, 0);
            }
            else
            {
                uint32_t red   = 0;
                uint32_t green = 0;
                uint32_t blue  = 0;
                uint32_t alpha = 255;

                switch (bitmap.pixel_mode)
                {
                case FT_PIXEL_MODE_LCD:
                {
                    red   = (bitmap.buffer + imgY * bitmap.pitch)[3*imgX+0];
                    green = (bitmap.buffer + imgY * bitmap.pitch)[3*imgX+1];
                    blue  = (bitmap.buffer + imgY * bitmap.pitch)[3*imgX+2];
                    break;
                }
                case FT_PIXEL_MODE_LCD_V:
                {
                    red   = (bitmap.buffer + (3*imgY+0) * bitmap.pitch)[imgX];
                    green = (bitmap.buffer + (3*imgY+1) * bitmap.pitch)[imgX];
                    blue  = (bitmap.buffer + (3*imgY+2) * bitmap.pitch)[imgX];
                    break;
                }
                case FT_PIXEL_MODE_GRAY:
                {
                    red = green = blue = (bitmap.buffer + (imgY) * bitmap.pitch)[imgX];
                    break;
                }
                case FT_PIXEL_MODE_MONO:
                {

                    uint32_t byte = (bitmap.buffer + imgY * bitmap.pitch)[imgX/8];
                    byte &= (((uint32_t)1) << (7-(imgX%8)));
                    if (byte)
                    {
                        red = green = blue = 255;
                    }
                    else
                    {
                        red = green = blue = 0;
                    }
                    break;
                }
                case FT_PIXEL_MODE_BGRA:
                {
                    red   = (bitmap.buffer + imgY * bitmap.pitch)[4*imgX+2];
                    green = (bitmap.buffer + imgY * bitmap.pitch)[4*imgX+1];
                    blue  = (bitmap.buffer + imgY * bitmap.pitch)[4*imgX+0];
                    alpha = (bitmap.buffer + imgY * bitmap.pitch)[4*imgX+3];
                    break;
                }
                }

                m_signals.pixel_selected.emit(1, (red << 24) | (green << 16) | (blue << 8) | alpha);
            }

            pointSignalEmitted = true;
        }
    }

    return true;
}


static
Gtk::Widget& makeBoldLabel(const std::string &text)
{
    auto *lbl = Gtk::make_managed<Gtk::Label>();
    lbl->set_markup(("<b>" + text + "</b>").c_str());
    lbl->set_xalign(0);
    lbl->show();
    return *lbl;
}


FreetypeBitmapDrawer::FreetypeBitmapDrawer(FT_Face &face, Signals &signals)
    : m_face(face)
    , m_signals(signals)
{
    set_size_request(700, 700);
    set_hexpand(true);
    set_vexpand(true);

    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
             | Gdk::EventMask::BUTTON_RELEASE_MASK
             | Gdk::EventMask::BUTTON1_MOTION_MASK
             | Gdk::EventMask::SCROLL_MASK);

    signal_button_press_event().connect([this](GdkEventButton *ev) -> bool
    {
        lastX = ev->x;
        lastY = ev->y;
        hadMotion = false;
        return true;
    });

    signal_button_release_event().connect([this](GdkEventButton *but) -> bool
    {
        if (!hadMotion)
        {
            double x = but->x;
            double y = but->y;

            Cairo::Matrix inv = (m_transformMatrix * Cairo::translation_matrix(lastWidth * 0.5, lastHeight * 0.5));
            inv.invert();
            inv.transform_point(x, y);

            pointSelected = true;
            pointSignalEmitted = false;
            selX = floor(x);
            selY = floor(y);
            queue_draw();
        }
        return true;
    });

    signal_motion_notify_event().connect([this](GdkEventMotion *ev) -> bool
    {
        hadMotion = true;

        double dx = ev->x - lastX;
        double dy = ev->y - lastY;
        lastX = ev->x;
        lastY = ev->y;

        m_transformMatrix = m_transformMatrix * Cairo::translation_matrix(dx, dy);

        queue_draw();
        return true;
    });

    signal_scroll_event().connect([this](GdkEventScroll *ev) -> bool
    {
        double factor;
        switch (ev->direction)
        {
        case GDK_SCROLL_UP:   factor = 1.1; break;
        case GDK_SCROLL_DOWN: factor = (1.0 / 1.1); break;
        default: return true;
        };

        double x = ev->x;
        double y = ev->y;

        // screen space -> coord space
        Cairo::Matrix inv = m_transformMatrix * Cairo::translation_matrix(lastWidth * 0.5, lastHeight * 0.5);
        inv.invert();
        inv.transform_point(x, y);

        m_transformMatrix.scale(factor, factor);

        // coord space -> new screen space
        (m_transformMatrix * Cairo::translation_matrix(lastWidth * 0.5, lastHeight * 0.5)).transform_point(x, y);

        // offset extra translation caused by the zoom
        m_transformMatrix = m_transformMatrix * Cairo::translation_matrix(ev->x - x, ev->y - y);

        queue_draw();
        return true;
    });

    show();
}

