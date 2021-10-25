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

#include <ctime>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>

#include <unicode/utypes.h>
#include <unicode/uchar.h>
#include <unicode/uclean.h>
#include <cinttypes>

#include <cairomm/context.h>
#include <glibmm/main.h>

#include <gtkmm/adjustment.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/application.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/treerowreference.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/grid.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/separator.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/window.h>
#include <gtkmm/paned.h>
#include <gtkmm/textview.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolitem.h>
#include <gtkmm/stock.h>
#include <gdkmm/pixbufloader.h>

#include "common.hpp"

#include "drawer.hpp"


#if (FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 11)
#define FONTDEBUG_HAS_SDF 1
#endif

static const char *gpl3_notice = R"(This file is part of FontDebug.

FontDebug is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FontDebug is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FontDebug.  If not, see <https://www.gnu.org/licenses/>.)";

extern unsigned char resources_app_icon_png[];
extern unsigned int resources_app_icon_png_len;

std::runtime_error FreetypeError(FT_Error errorCode, const char *method)
{
    char buf[1000];
    sprintf(buf, "FreeType error: %s: (%d) %s", method, errorCode, FT_Error_String(errorCode));
    return std::runtime_error(buf);
}

struct FontGlyphSelectorColumns : public Gtk::TreeModel::ColumnRecord
{
    FontGlyphSelectorColumns()
    {
        add(colCode);
        add(colName);
        add(colCharCodeInt);
    }

    Gtk::TreeModelColumn<Glib::ustring> colCode;
    Gtk::TreeModelColumn<Glib::ustring> colName;
    Gtk::TreeModelColumn<int> colCharCodeInt;
};

static
const char* BlockCodeToString(int blockCode)
{
    // libicu does not seem to expose this data..
    switch (blockCode)
    {
        default:  return "Unknown";
        case  -1: return "Invalid Code";
        case   0: return "No Block";
        case   1: return "Basic Latin";
        case   2: return "Latin-1 Supplement";
        case   3: return "Latin Extended-A";
        case   4: return "Latin Extended-B";
        case   5: return "IPA Extensions";
        case   6: return "Spacing Modifier Letters";
        case   7: return "Combining Diacritical Marks";
        case   8: return "Greek and Coptic";
        case   9: return "Cyrillic";
        case  10: return "Armenian";
        case  11: return "Hebrew";
        case  12: return "Arabic";
        case  13: return "Syriac";
        case  14: return "Thaana";
        case  15: return "Devanagari";
        case  16: return "Bengali";
        case  17: return "Gurmukhi";
        case  18: return "Gujarati";
        case  19: return "Oriya";
        case  20: return "Tamil";
        case  21: return "Telugu";
        case  22: return "Kannada";
        case  23: return "Malayalam";
        case  24: return "Sinhala";
        case  25: return "Thai";
        case  26: return "Lao";
        case  27: return "Tibetan";
        case  28: return "Myanmar";
        case  29: return "Georgian";
        case  30: return "Hangul Jamo";
        case  31: return "Ethiopic";
        case  32: return "Cherokee";
        case  33: return "Unified Canadian Aboriginal Syllabics";
        case  34: return "Ogham";
        case  35: return "Runic";
        case  36: return "Khmer";
        case  37: return "Mongolian";
        case  38: return "Latin Extended Additional";
        case  39: return "Greek Extended";
        case  40: return "General Punctuation";
        case  41: return "Superscripts and Subscripts";
        case  42: return "Currency Symbols";
        case  43: return "Combining Diacritical Marks for Symbols";
        case  44: return "Letterlike Symbols";
        case  45: return "Number Forms";
        case  46: return "Arrows";
        case  47: return "Mathematical Operators";
        case  48: return "Miscellaneous Technical";
        case  49: return "Control Pictures";
        case  50: return "Optical Character Recognition";
        case  51: return "Enclosed Alphanumerics";
        case  52: return "Box Drawing";
        case  53: return "Block Elements";
        case  54: return "Geometric Shapes";
        case  55: return "Miscellaneous Symbols";
        case  56: return "Dingbats";
        case  57: return "Braille Patterns";
        case  58: return "CJK Radicals Supplement";
        case  59: return "Kangxi Radicals";
        case  60: return "Ideographic Description Characters";
        case  61: return "CJK Symbols and Punctuation";
        case  62: return "Hiragana";
        case  63: return "Katakana";
        case  64: return "Bopomofo";
        case  65: return "Hangul Compatibility Jamo";
        case  66: return "Kanbun";
        case  67: return "Bopomofo Extended";
        case  68: return "Enclosed CJK Letters and Months";
        case  69: return "CJK Compatibility";
        case  70: return "CJK Unified Ideographs Extension A";
        case  71: return "CJK Unified Ideographs";
        case  72: return "Yi Syllables";
        case  73: return "Yi Radicals";
        case  74: return "Hangul Syllables";
        case  75: return "High Surrogates";
        case  76: return "High Private Use Surrogates";
        case  77: return "Low Surrogates";
        case  78: return "Private Use Area";
        case  79: return "CJK Compatibility Ideographs";
        case  80: return "Alphabetic Presentation Forms";
        case  81: return "Arabic Presentation Forms-A";
        case  82: return "Combining Half Marks";
        case  83: return "CJK Compatibility Forms";
        case  84: return "Small Form Variants";
        case  85: return "Arabic Presentation Forms-B";
        case  86: return "Specials";
        case  87: return "Halfwidth and Fullwidth Forms";
        case  88: return "Old Italic";
        case  89: return "Gothic";
        case  90: return "Deseret";
        case  91: return "Byzantine Musical Symbols";
        case  92: return "Musical Symbols";
        case  93: return "Mathematical Alphanumeric Symbols";
        case  94: return "CJK Unified Ideographs Extension B";
        case  95: return "CJK Compatibility Ideographs Supplement";
        case  96: return "Tags";
        case  97: return "Cyrillic Supplement";
        case  98: return "Tagalog";
        case  99: return "Hanunoo";
        case 100: return "Buhid";
        case 101: return "Tagbanwa";
        case 102: return "Miscellaneous Mathematical Symbols-A";
        case 103: return "Supplemental Arrows-A";
        case 104: return "Supplemental Arrows-B";
        case 105: return "Miscellaneous Mathematical Symbols-B";
        case 106: return "Supplemental Mathematical Operators";
        case 107: return "Katakana Phonetic Extensions";
        case 108: return "Variation Selectors";
        case 109: return "Supplementary Private Use Area-A";
        case 110: return "Supplementary Private Use Area-B";
        case 111: return "Limbu";
        case 112: return "Tai Le";
        case 113: return "Khmer Symbols";
        case 114: return "Phonetic Extensions";
        case 115: return "Miscellaneous Symbols and Arrows";
        case 116: return "Yijing Hexagram Symbols";
        case 117: return "Linear B Syllabary";
        case 118: return "Linear B Ideograms";
        case 119: return "Aegean Numbers";
        case 120: return "Ugaritic";
        case 121: return "Shavian";
        case 122: return "Osmanya";
        case 123: return "Cypriot Syllabary";
        case 124: return "Tai Xuan Jing Symbols";
        case 125: return "Variation Selectors Supplement";
        case 126: return "Ancient Greek Musical Notation";
        case 127: return "Ancient Greek Numbers";
        case 128: return "Arabic Supplement";
        case 129: return "Buginese";
        case 130: return "CJK Strokes";
        case 131: return "Combining Diacritical Marks Supplement";
        case 132: return "Coptic";
        case 133: return "Ethiopic Extended";
        case 134: return "Ethiopic Supplement";
        case 135: return "Georgian";
        case 136: return "Glagolitic";
        case 137: return "Kharoshthi";
        case 138: return "Modifier Tone Letters";
        case 139: return "New Tai Lue";
        case 140: return "Old Persian";
        case 141: return "Phonetic Extensions Supplement";
        case 142: return "Supplemental Punctuation";
        case 143: return "Syloti Nagri";
        case 144: return "Tifinagh";
        case 145: return "Vertical Forms";
        case 146: return "NKo";
        case 147: return "Balinese";
        case 148: return "Latin Extended-C";
        case 149: return "Latin Extended-D";
        case 150: return "Phags-pa";
        case 151: return "Phoenician";
        case 152: return "Cuneiform";
        case 153: return "Cuneiform Numbers and Punctuation";
        case 154: return "Counting Rod Numerals";
        case 155: return "Sundanese";
        case 156: return "Lepcha";
        case 157: return "Ol Chiki";
        case 158: return "Cyrillic Extended-A";
        case 159: return "Vai";
        case 160: return "Cyrillic Extended-B";
        case 161: return "Saurashtra";
        case 162: return "Kayah Li";
        case 163: return "Rejang";
        case 164: return "Cham";
        case 165: return "Ancient Symbols";
        case 166: return "Phaistos Disc";
        case 167: return "Lycian";
        case 168: return "Carian";
        case 169: return "Lydian";
        case 170: return "Mahjong Tiles";
        case 171: return "Domino Tiles";
        case 172: return "Samaritan";
        case 173: return "Unified Canadian Aboriginal Syllabics Extended";
        case 174: return "Tai Tham";
        case 175: return "Vedic Extensions";
        case 176: return "Lisu";
        case 177: return "Bamum";
        case 178: return "Common Indic Number Forms";
        case 179: return "Devanagari Extended";
        case 180: return "Hangul Jamo Extended-A";
        case 181: return "Javanese";
        case 182: return "Myanmar Extended-A";
        case 183: return "Tai Viet";
        case 184: return "Meetei Mayek";
        case 185: return "Hangul Jamo Extended-B";
        case 186: return "Imperial Aramaic";
        case 187: return "Old South Arabian";
        case 188: return "Avestan";
        case 189: return "Inscriptional Parthian";
        case 190: return "Inscriptional Pahlavi";
        case 191: return "Old Turkic";
        case 192: return "Rumi Numeral Symbols";
        case 193: return "Kaithi";
        case 194: return "Egyptian Hieroglyphs";
        case 195: return "Enclosed Alphanumeric Supplement";
        case 196: return "Enclosed Ideographic Supplement";
        case 197: return "CJK Unified Ideographs Extension C";
        case 198: return "Mandaic";
        case 199: return "Batak";
        case 200: return "Ethiopic Extended-A";
        case 201: return "Brahmi";
        case 202: return "Bamum Supplement";
        case 203: return "Kana Supplement";
        case 204: return "Playing Cards";
        case 205: return "Miscellaneous Symbols and Pictographs";
        case 206: return "Emoticons";
        case 207: return "Transport and Map Symbols";
        case 208: return "Alchemical Symbols";
        case 209: return "CJK Unified Ideographs Extension D";
        case 210: return "Arabic Extended-A";
        case 211: return "Arabic Mathematical Alphabetic Symbols";
        case 212: return "Chakma";
        case 213: return "Meetei Mayek Extensions";
        case 214: return "Meroitic Cursive";
        case 215: return "Meroitic Hieroglyphs";
        case 216: return "Miao";
        case 217: return "Sharada";
        case 218: return "Sora Sompeng";
        case 219: return "Sundanese Supplement";
        case 220: return "Takri";
        case 221: return "Bassa Vah";
        case 222: return "Caucasian Albanian";
        case 223: return "Coptic Epact Numbers";
        case 224: return "Combining Diacritical Marks Extended";
        case 225: return "Duployan";
        case 226: return "Elbasan";
        case 227: return "Geometric Shapes";
        case 228: return "Grantha";
        case 229: return "Khojki";
        case 230: return "Khudawadi";
        case 231: return "Latin Extended-E";
        case 232: return "Linear A";
        case 233: return "Mahajani";
        case 234: return "Manichaean";
        case 235: return "Mende Kikakui";
        case 236: return "Modi";
        case 237: return "Mro";
        case 238: return "Myanmar Extended-B";
        case 239: return "Nabataean";
        case 240: return "Old North Arabian";
        case 241: return "Old Permic";
        case 242: return "Ornamental Dingbats";
        case 243: return "Pahawh Hmong";
        case 244: return "Palmyrene";
        case 245: return "Pau Cin Hau";
        case 246: return "Psalter Pahlavi";
        case 247: return "Shorthand Format Controls";
        case 248: return "Siddham";
        case 249: return "Sinhala Archaic Numbers";
        case 250: return "Supplemental Arrows-C";
        case 251: return "Tirhuta";
        case 252: return "Warang Citi";
        case 253: return "Ahom";
        case 254: return "Anatolian Hieroglyphs";
        case 255: return "Cherokee Supplement";
        case 256: return "CJK Unified Ideographs Extension E";
        case 257: return "Early Dynastic Cuneiform";
        case 258: return "Hatran";
        case 259: return "Multani";
        case 260: return "Old Hungarian";
        case 261: return "Supplemental Symbols and Pictographs";
        case 262: return "Sutton SignWriting";
        case 263: return "Adlam";
        case 264: return "Bhaiksuki";
        case 265: return "Cyrillic Extended-C";
        case 266: return "Glagolitic Supplement";
        case 267: return "Ideographic Symbols and Punctuation";
        case 268: return "Marchen";
        case 269: return "Mongolian Supplement";
        case 270: return "Newa";
        case 271: return "Osage";
        case 272: return "Tangut";
        case 273: return "Tangut Components";
        case 274: return "CJK Unified Ideographs Extension F";
        case 275: return "Kana Extended-A";
        case 276: return "Masaram Gondi";
        case 277: return "Nushu";
        case 278: return "Soyombo";
        case 279: return "Syriac Supplement";
        case 280: return "Zanabazar Square";
        case 281: return "Chess Symbols";
        case 282: return "Dogra";
        case 283: return "Georgian";
        case 284: return "Gunjala Gondi";
        case 285: return "Hanifi Rohingya";
        case 286: return "Indic Siyaq Numbers";
        case 287: return "Makasar";
        case 288: return "Mayan Numerals";
        case 289: return "Medefaidrin";
        case 290: return "Old Sogdian";
        case 291: return "Sogdian";
        case 292: return "Egyptian Hieroglyph Format Controls";
        case 293: return "Elymaic";
        case 294: return "Nandinagari";
        case 295: return "Nyiakeng Puachue Hmong";
        case 296: return "Ottoman Siyaq Numbers";
        case 297: return "Small Kana Extension";
        case 298: return "Symbols and Pictographs Extended-A";
        case 299: return "Tamil Supplement";
        case 300: return "Wancho";
        case 301: return "Chorasmian";
        case 302: return "CJK Unified Ideographs Extension G";
        case 303: return "Dives Akuru";
        case 304: return "Khitan Small Script";
        case 305: return "Lisu Supplement";
        case 306: return "Symbols for Legacy Computing";
        case 307: return "Tangut Supplement";
        case 308: return "Yezidi";
    }
}

struct FontDebug : public Gtk::Window
{
    FontDebug()
    {
        if (FT_Init_FreeType(&_ft)) throw std::runtime_error("FT_Init_FreeType");

        set_border_width(5);

        {
            auto *hdrbar = Gtk::make_managed<Gtk::HeaderBar>();
            hdrbar->set_title("FontDebug");

            FT_Int major, minor, patch;
            FT_Library_Version(_ft, &major, &minor, &patch);
            char buf[100];
            sprintf(buf, "Using FreeType %d.%d.%d", major, minor, patch);
            hdrbar->set_subtitle(buf);
            hdrbar->set_show_close_button(true);
            hdrbar->show();


            auto menuu = menu({
                menuItem("About", [this](){
                    Gtk::AboutDialog abt;

                    abt.set_transient_for(*this);

                    {
                        auto loader = Gdk::PixbufLoader::create("png");
                        loader->write(resources_app_icon_png, resources_app_icon_png_len);
                        loader->close();
                        abt.set_logo(loader->get_pixbuf());
                    }

                    abt.set_program_name("FontDebug");
                    abt.set_version("1.0.0");
                    abt.set_copyright("Serdar Sanli");
                    abt.set_website("https://github.com/mserdarsanli/FontDebug");
                    abt.set_comments("A utility for exploring FreeType.");
                    abt.set_license(gpl3_notice);

                    std::vector<Glib::ustring> list_authors;
                    list_authors.push_back("Serdar Sanli");
                    abt.set_authors(list_authors);

                    abt.run();
                })
            });


            auto *menubtn = Gtk::make_managed<Gtk::MenuButton>();
            menubtn->set_menu(*menuu);
            menubtn->show();
            hdrbar->pack_start(*menubtn);


            set_titlebar(*hdrbar);
        }

        // TODO better default font search?
        for (const auto &p : std::filesystem::recursive_directory_iterator("/usr/share/fonts"))
        {
            std::string path = p.path();
            size_t idx = path.rfind('.');
            if (idx == std::string::npos) continue;

            std::string extension = path.substr(idx);
            if (extension == ".ttf" || extension == ".otf")
            {
                std::string name = path.substr(path.rfind('/') + 1);

                if (m_selectedFontName == "" || name == "DejaVuSans.ttf")
                {
                    m_selectedFontName = name;
                    m_selectedFontPath = path;
                }
            }
        }
        if (m_selectedFontName == "") throw std::runtime_error("No font found");

        auto *paned2 = Gtk::make_managed<Gtk::Paned>();
        paned2->show();

        auto *paned = Gtk::make_managed<Gtk::Paned>();
        paned->pack1(makeConfigGrid(), false, false);
        paned->pack2(*paned2, true, false);
        paned->show();

        auto *mainGrid = Gtk::make_managed<Gtk::Grid>();
        mainGrid->attach_next_to(makeToolbar(), Gtk::PositionType::POS_BOTTOM);
        mainGrid->attach_next_to(*paned, Gtk::PositionType::POS_BOTTOM);
        mainGrid->show();
        this->add(*mainGrid);


        m_drawer = Gtk::make_managed<FreetypeBitmapDrawer>(_face, signals);
        m_drawer->set_margin_start(5);
        m_drawer->set_margin_end(5);
        m_drawer->set_margin_top(5);
        m_drawer->set_margin_bottom(5);

        m_drawer->show();
        paned2->pack1(*m_drawer, true, false);
        m_onFontReload.push_back([this](FT_Face)
        {
            m_drawer->pointSelected = false;
            signals.pixel_selected.emit(-1, 0);
            m_drawer->queue_draw();
        });

        paned2->pack2(makePropertiesWidget(signals), false, false);

        font_redraw();
    }

private:

    Gtk::Menu* menu(std::initializer_list<Gtk::MenuItem*> items)
    {
        Gtk::Menu *menu = Gtk::make_managed<Gtk::Menu>();
        menu->show();
        for (Gtk::MenuItem *item : items)
        {
            menu->append(*item);
        }
        return menu;
    }

    Gtk::MenuItem* menuItem(const char *label, Gtk::Menu *menu)
    {
        auto *item = Gtk::make_managed<Gtk::MenuItem>(label);
        item->show();
        item->set_submenu(*menu);
        return item;
    }

    Gtk::MenuItem* menuItem(const char *label, std::function<void()> action)
    {
        auto *item = Gtk::make_managed<Gtk::MenuItem>(label);
        item->show();
        item->signal_activate().connect([action]()
        {
            action();
        });
        return item;
    }

    Gtk::CheckMenuItem* checkMenuItem(const char *label, bool active, std::function<void(bool)> action)
    {
        auto *item = Gtk::make_managed<Gtk::CheckMenuItem>(label);
        item->show();
        item->set_active(active);
        item->signal_activate().connect([item, action]()
        {
            action(item->get_active());
        });
        return item;
    }

    Gtk::SeparatorMenuItem* separatorMenuItem()
    {
        auto *item = Gtk::make_managed<Gtk::SeparatorMenuItem>();
        item->show();
        return item;
    }


    bool pickFont()
    {
        Gtk::FileChooserDialog dialog("Choose Font", Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(*this);
        dialog.set_filename(m_selectedFontPath);

        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Open", Gtk::RESPONSE_OK);

        auto filter = Gtk::FileFilter::create();
        filter->set_name("Font files");
        filter->add_mime_type("application/x-font-ttf");
        filter->add_mime_type("application/x-font-opentype");
        dialog.add_filter(filter);

        if (dialog.run() == Gtk::RESPONSE_OK)
        {
            m_selectedFontPath = dialog.get_filename();
            size_t idx = m_selectedFontPath.rfind('/');
            m_selectedFontName = m_selectedFontPath.substr(idx+1);
            return true;
        }
        return false;
    }

    Gtk::Widget& makeBoldLabel(const std::string &text)
    {
        auto *lbl = Gtk::make_managed<Gtk::Label>();
        lbl->set_markup(("<b>" + text + "</b>").c_str());
        lbl->set_xalign(0);
        lbl->show();
        return *lbl;
    }

    Gtk::Widget& makeSeparator()
    {
        auto *sep = Gtk::make_managed<Gtk::HSeparator>();
        sep->set_margin_top(3);
        sep->set_margin_bottom(3);
        sep->show();
        return *sep;
    }

    Gtk::Widget& makeConfigGrid()
    {
        auto *cfgGrid = Gtk::make_managed<Gtk::Grid>();
        cfgGrid->set_size_request(300, -1);
        cfgGrid->set_margin_start(5);
        cfgGrid->set_margin_end(5);
        cfgGrid->set_margin_top(5);
        cfgGrid->set_margin_bottom(5);

        int curRow = 3;
        cfgGrid->show();
        cfgGrid->set_column_spacing(5);

        cfgGrid->attach(makeBoldLabel("Glyphs"), 1, curRow++);

        {
            auto *m_ScrolledWindow = Gtk::make_managed<Gtk::ScrolledWindow>();
            auto *m_TreeView = Gtk::make_managed<Gtk::TreeView>();

            m_ScrolledWindow->add(*m_TreeView);
            m_ScrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

            Glib::RefPtr<Gtk::TreeStore> fontGlyphsModel = Gtk::TreeStore::create(columns);

            m_onFaceReload.push_back([this, fontGlyphsModel, m_TreeView](FT_Face face)
            {
                beingCleared = true;
                fontGlyphsModel->clear();
                beingCleared = false;

                // Char picking logic, highest is preferable
                // 1 - Pick first non-control charcode in the font
                // 2 - Default char 'A' is available, pick that
                // 3 - m_charCode (from last font) is available, pick that
                int charSelectionType = 0;
                int charSelection = -1;

                int lastBlockCode = -2;
                Gtk::TreeModel::Row lastBlockRow;

                FT_UInt gindex;
                FT_ULong charCode = FT_Get_First_Char(face, &gindex);
                while (gindex != 0)
                {
                    char charNameBuf[100];
                    UErrorCode errorCode = U_ZERO_ERROR;
                    u_charName(charCode, U_EXTENDED_CHAR_NAME, charNameBuf, sizeof(charNameBuf), &errorCode);
                    if (errorCode != U_ZERO_ERROR) throw std::runtime_error("u_charName");

                    int blockCode = ublock_getCode(charCode);

                    if (blockCode != lastBlockCode)
                    {
                        lastBlockCode = blockCode;
                        lastBlockRow = *(fontGlyphsModel->append());
                        lastBlockRow[columns.colName] = BlockCodeToString(blockCode);
                        lastBlockRow[columns.colCharCodeInt] = -1;
                    }

                    auto rowIt = fontGlyphsModel->append(lastBlockRow.children());
                    (*rowIt)[columns.colCode] = std::to_string(charCode);
                    (*rowIt)[columns.colName] = charNameBuf;
                    (*rowIt)[columns.colCharCodeInt] = charCode;

                    charCode = FT_Get_Next_Char(face, charCode, &gindex);

                    auto selectChar = [&](int selectionType, int charCode) {
                        charSelectionType = selectionType;
                        charSelection = charCode;
                    };

                    if (charSelectionType < 1 && charCode > 32) {
                        selectChar(1, charCode);
                    }
                    if (charSelectionType < 2 && charCode == 'A') {
                        selectChar(2, charCode);
                    }
                    if (charSelectionType < 3 && charCode == m_charCode) {
                        selectChar(3, charCode);
                    }
                }

                m_charCode = charSelection;


                // Rescan the tree and find the selected glyph
                // Storing a refence to glyph row and selecting that is simpler
                // but doing that selects the row above and I can't figure out why
                std::string charCodeStr = std::to_string(m_charCode);
                for (Gtk::TreeRow groupRow : fontGlyphsModel->children())
                {
                    for (Gtk::TreeRow glyphRow : groupRow.children())
                    {
                        if (charCodeStr == glyphRow.get_value(columns.colCode))
                        {
                            auto path = fontGlyphsModel->get_path(glyphRow);
                            m_TreeView->expand_to_path(path);
                            m_TreeView->scroll_to_row(path);
                            m_TreeView->get_selection()->select(path); // TODO XXX this causes double redraw
                        }
                    }
                }
            });

            m_TreeView->set_model(fontGlyphsModel);

            m_TreeView->append_column("ID", columns.colCode);
            m_TreeView->append_column("Name", columns.colName);
            m_TreeView->show();
            m_ScrolledWindow->show();


            cfgGrid->attach(*m_ScrolledWindow, 1, curRow++);

            m_TreeView->signal_cursor_changed().connect([this, m_TreeView, fontGlyphsModel]()
            {
                if (beingCleared) return;
                Gtk::TreeModel::Path path;
                Gtk::TreeViewColumn *col;
                m_TreeView->get_cursor(path, col);

                Gtk::TreeModel::Row selectedRow = *(fontGlyphsModel->get_iter(path));
                int charCode = selectedRow.get_value(columns.colCharCodeInt);
                if (charCode != -1 && charCode != m_charCode)
                {
                    m_charCode = charCode;
                    font_redraw();
                }
            });

            m_ScrolledWindow->set_vexpand();
            m_ScrolledWindow->set_hexpand();
        }

        cfgGrid->set_vexpand();
        cfgGrid->set_hexpand();

        cfgGrid->attach(makeSeparator(), 1, curRow++);

        return *cfgGrid;
    }

    Widget& makeToolbar()
    {
        auto *toolbarr = Gtk::make_managed<Gtk::Grid>();
        toolbarr->show();

        auto toolbarAdd = [&](const std::string &label, Gtk::Widget &w)
        {
            auto *lbl = Gtk::make_managed<Gtk::Label>();
            lbl->set_markup(("<b>" + label + "</b>").c_str());
            lbl->set_margin_right(3);
            lbl->show();

            w.set_margin_right(6);

            toolbarr->attach_next_to(*lbl, Gtk::PositionType::POS_RIGHT);
            toolbarr->attach_next_to(w, Gtk::PositionType::POS_RIGHT);
        };

        {
            auto *fontBox = Gtk::make_managed<Gtk::Button>();
            fontBox->set_label(m_selectedFontName);
            fontBox->signal_clicked().connect([this, fontBox]()
            {
                if (pickFont())
                {
                    fontBox->set_label(m_selectedFontName);
                    if (_face != nullptr)
                    {
                        if (FT_Done_Face(_face)) throw std::runtime_error("FT_Done_Face");
                        _face = nullptr;
                    }

                    font_redraw();
                }
            });
            fontBox->show();

            toolbarAdd("Font", *fontBox);
        }

        {
            Glib::RefPtr<Gtk::Adjustment> adj = Gtk::Adjustment::create(13.0, 1.0, 128.0, 1.0, 5.0, 0.0);

            auto *btn = Gtk::make_managed<Gtk::SpinButton>(adj, 1.0, 0);
            btn->signal_value_changed().connect([this, btn]()
            {
                m_charSize = btn->get_value();
                font_redraw();
            });
            btn->show();

            toolbarAdd("Char Size", *btn);
        }

        {
            auto *modeBtn = Gtk::make_managed<Gtk::ComboBoxText>();
            modeBtn->append("FT_RENDER_MODE_NORMAL");
            modeBtn->append("FT_RENDER_MODE_LIGHT");
            modeBtn->append("FT_RENDER_MODE_MONO");
            modeBtn->append("FT_RENDER_MODE_LCD");
            modeBtn->append("FT_RENDER_MODE_LCD_V");
            #ifdef FONTDEBUG_HAS_SDF
            modeBtn->append("FT_RENDER_MODE_SDF");
            #endif
            modeBtn->set_active(3);
            modeBtn->show();
            modeBtn->signal_changed().connect([this, modeBtn]()
            {
                auto text = modeBtn->get_active_text();
                if (text == "FT_RENDER_MODE_NORMAL") m_renderMode = FT_RENDER_MODE_NORMAL;
                if (text == "FT_RENDER_MODE_LIGHT")  m_renderMode = FT_RENDER_MODE_LIGHT;
                if (text == "FT_RENDER_MODE_MONO")   m_renderMode = FT_RENDER_MODE_MONO;
                if (text == "FT_RENDER_MODE_LCD")    m_renderMode = FT_RENDER_MODE_LCD;
                if (text == "FT_RENDER_MODE_LCD_V")  m_renderMode = FT_RENDER_MODE_LCD_V;
                #ifdef FONTDEBUG_HAS_SDF
                if (text == "FT_RENDER_MODE_SDF")    m_renderMode = FT_RENDER_MODE_SDF;
                #endif
                font_redraw();
            });

            toolbarAdd("Render Mode", *modeBtn);
        }


        auto *toolbarr2 = Gtk::make_managed<Gtk::Grid>();
        toolbarr2->show();

        {
            auto *lbl = Gtk::make_managed<Gtk::Label>();
            lbl->set_markup("<b>Load Flags</b>");
            lbl->set_margin_right(3);
            lbl->show();

            toolbarr2->attach(*lbl, 1, 1, 1, 2);

            auto addLoadFlagsButton = [&](int left, int top, const char *name, int value)
            {
                auto *but = Gtk::make_managed<Gtk::CheckButton>(name);
                but->show();
                but->signal_toggled().connect([this, but, value]()
                {
                    if (value == FT_LOAD_VERTICAL_LAYOUT)
                    {
                        m_drawer->m_isHorizontal = !but->get_active();
                    }

                    if (but->get_active())
                    {
                        m_loadFlags |= value;
                    }
                    else
                    {
                        m_loadFlags &= ~value;
                    }
                    font_redraw();
                });

                toolbarr2->attach(*but, left, top);
            };

            addLoadFlagsButton(2, 1, "COLOR", FT_LOAD_COLOR);
            addLoadFlagsButton(3, 1, "NO_SCALE", FT_LOAD_NO_SCALE);
            addLoadFlagsButton(4, 1, "NO_BITMAP", FT_LOAD_NO_BITMAP);
            addLoadFlagsButton(5, 1, "NO_HINTING", FT_LOAD_NO_HINTING);
            addLoadFlagsButton(2, 2, "NO_AUTOHINT", FT_LOAD_NO_AUTOHINT);
            addLoadFlagsButton(3, 2, "LINEAR_DESIGN", FT_LOAD_LINEAR_DESIGN);
            addLoadFlagsButton(4, 2, "FORCE_AUTOHINT", FT_LOAD_FORCE_AUTOHINT);
            addLoadFlagsButton(5, 2, "VERTICAL_LAYOUT", FT_LOAD_VERTICAL_LAYOUT);
        }

        auto *toolbarr3 = Gtk::make_managed<Gtk::Grid>();
        toolbarr3->show();

        {
            Gtk::CheckButton *btn;

            btn = Gtk::make_managed<Gtk::CheckButton>("Show Baseline");
            btn->signal_toggled().connect([this, btn]()
            {
                m_drawer->m_drawBaseline = btn->get_active();
                m_drawer->queue_draw();
            });
            btn->show();
            toolbarr3->attach_next_to(*btn, Gtk::PositionType::POS_BOTTOM);

            btn = Gtk::make_managed<Gtk::CheckButton>("Show Grid");
            btn->signal_toggled().connect([this, btn]()
            {
                m_drawer->m_drawGrid = btn->get_active();
                m_drawer->queue_draw();
            });
            btn->show();
            toolbarr3->attach_next_to(*btn, Gtk::PositionType::POS_BOTTOM);

            btn = Gtk::make_managed<Gtk::CheckButton>("Show Glyph Outline");
            btn->signal_toggled().connect([this, btn]()
            {
                m_drawer->m_drawOutline = btn->get_active();
                m_drawer->queue_draw();
            });
            btn->show();
            toolbarr3->attach_next_to(*btn, Gtk::PositionType::POS_BOTTOM);

            btn = Gtk::make_managed<Gtk::CheckButton>("Grayscale LCD");
            btn->signal_toggled().connect([this, btn]()
            {
                m_drawer->m_drawGrayscaleLCD = btn->get_active();
                m_drawer->queue_draw();
            });
            btn->show();
            toolbarr3->attach_next_to(*btn, Gtk::PositionType::POS_BOTTOM);
        }

        auto *tbGrid = Gtk::make_managed<Gtk::Grid>();
        tbGrid->show();
        // tbGrid->set_row_spacing(3);
        tbGrid->attach_next_to(*toolbarr, Gtk::PositionType::POS_BOTTOM, 1, 1);

        {
            auto *tsep = Gtk::make_managed<Gtk::VSeparator>();
            tsep->set_margin_top(3);
            tsep->set_margin_bottom(3);
            tsep->set_valign(Gtk::Align::ALIGN_START);
            tsep->show();
            tbGrid->attach_next_to(*tsep, Gtk::PositionType::POS_BOTTOM, 1, 1);
        }

        tbGrid->attach_next_to(*toolbarr2, Gtk::PositionType::POS_BOTTOM, 1, 1);

        {
            auto *tsep = Gtk::make_managed<Gtk::HSeparator>();
            tsep->set_margin_left(3);
            tsep->set_margin_right(3);
            tsep->set_halign(Gtk::Align::ALIGN_START);
            tsep->show();
            tbGrid->attach_next_to(*tsep, Gtk::PositionType::POS_RIGHT, 1, 3);
        }

        tbGrid->attach_next_to(*toolbarr3, Gtk::PositionType::POS_RIGHT, 1, 3);

        {
            auto *tsep = Gtk::make_managed<Gtk::HSeparator>();
            tsep->set_margin_left(3);
            tsep->set_margin_right(3);
            tsep->set_halign(Gtk::Align::ALIGN_START);
            tsep->show();
            tbGrid->attach_next_to(*tsep, Gtk::PositionType::POS_RIGHT, 1, 3);
        }

        tbGrid->attach_next_to(makeTransformWidget(), Gtk::PositionType::POS_RIGHT, 1, 3);

        {
            auto *tsep = Gtk::make_managed<Gtk::HSeparator>();
            tsep->set_margin_left(3);
            tsep->set_margin_right(3);
            tsep->set_halign(Gtk::Align::ALIGN_START);
            tsep->show();
            tbGrid->attach_next_to(*tsep, Gtk::PositionType::POS_RIGHT, 1, 3);
        }

        {
            auto *wGrid = Gtk::make_managed<Gtk::Grid>();
            wGrid->show();

            Gtk::Label *e;
            e = Gtk::make_managed<Gtk::Label>();
            e->set_markup("<b>Pixel Info</b>");
            e->show();
            wGrid->attach_next_to(*e, Gtk::PositionType::POS_BOTTOM);

            auto tw = Gtk::make_managed<Gtk::TextView>();
            tw->set_editable(false);
            tw->set_monospace();
            tw->get_buffer()->set_text(" (none selected) \n\n");
            tw->show();
            wGrid->attach_next_to(*tw, Gtk::PositionType::POS_BOTTOM);

            signals.pixel_selected.connect([tw](int mode, uint32_t rgba)
            {
                if (mode == -1)
                {
                    tw->get_buffer()->set_text(" (none selected) \n\n");
                }
                else if (mode == 0)
                {
                    tw->get_buffer()->set_text("       OOB       \n\n");
                }
                else
                {
                    char buf[1000];
                    sprintf(buf, "   Color RGBA:   \n    #%02x%02x%02x%02x    \n%d, %d, %d, %d",
                        rgba >> 24, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff,
                        rgba >> 24, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff);
                    tw->get_buffer()->set_text(buf);
                }
            });

            tbGrid->attach_next_to(*wGrid, Gtk::PositionType::POS_RIGHT, 1, 3);
        }

        // TODO only for layout..
        auto *l = Gtk::make_managed<Gtk::Label>();
        l->show();
        l->set_hexpand(true);
        tbGrid->attach_next_to(*l, Gtk::PositionType::POS_RIGHT);

        return *tbGrid;
    }

    Widget& makeTransformWidget()
    {
        auto *wGrid = Gtk::make_managed<Gtk::Grid>();
        wGrid->show();

        Gtk::Label *e;

        e = Gtk::make_managed<Gtk::Label>();
        e->set_markup("<b>Transform</b>");
        e->show();
        wGrid->attach_next_to(*e, Gtk::PositionType::POS_BOTTOM);


        auto tw = Gtk::make_managed<Gtk::TextView>();
        tw->set_editable(false);
        tw->set_monospace();
        tw->get_buffer()->set_text("  1.000   0.000   0.000\n  0.000   1.000   0.000");
        tw->show();

        signals.glyph_transform_updated.connect([this, tw](Cairo::Matrix m)
        {
            char buf[1000];
            sprintf(buf, "%6.3f %6.3f %6.3f\n%6.3f %6.3f %6.3f", m.xx, m.xy, m.x0, m.yx, m.yy, m.y0);
            tw->get_buffer()->set_text(buf);
            m_glyphTransform = m;
            font_redraw();
        });

        wGrid->attach_next_to(*tw, Gtk::PositionType::POS_BOTTOM);

        auto menuu = menu({
            menuItem("Reset", [this](){
                signals.glyph_transform_updated.emit(Cairo::identity_matrix());
            }),
            separatorMenuItem(),
            menuItem("Translate", [this]()
            {
                Gtk::Dialog d("Translation", Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_USE_HEADER_BAR);

                const Cairo::Matrix origTransform = m_glyphTransform;

                {
                    auto *grid = Gtk::make_managed<Gtk::Grid>();

                    Glib::RefPtr<Gtk::Adjustment> adjX = Gtk::Adjustment::create(0.0, -1.0, 1.0, 0.01, 0.1, 0.0);
                    auto *scaleX = Gtk::make_managed<Gtk::Scale>(adjX, Gtk::ORIENTATION_HORIZONTAL);
                    scaleX->set_hexpand();
                    scaleX->set_digits(3);
                    scaleX->set_value_pos(Gtk::POS_TOP);
                    scaleX->set_draw_value();
                    scaleX->show();

                    auto *labelX = Gtk::make_managed<Gtk::Label>("Shift X");
                    labelX->show();

                    Glib::RefPtr<Gtk::Adjustment> adjY = Gtk::Adjustment::create(0.0, -1.0, 1.0, 0.01, 0.1, 0.0);
                    auto *scaleY = Gtk::make_managed<Gtk::Scale>(adjY, Gtk::ORIENTATION_HORIZONTAL);
                    scaleY->set_hexpand();
                    scaleY->set_digits(3);
                    scaleY->set_value_pos(Gtk::POS_TOP);
                    scaleY->set_draw_value();
                    scaleY->show();

                    auto *labelY = Gtk::make_managed<Gtk::Label>("Shift Y");
                    labelY->show();

                    grid->attach(*labelX, 1, 1);
                    grid->attach(*scaleX, 2, 1);
                    grid->attach(*labelY, 1, 2);
                    grid->attach(*scaleY, 2, 2);
                    grid->show();
                    d.get_content_area()->add(*grid);

                    auto update_translation = [this, scaleX, scaleY, &origTransform]()
                    {
                        Cairo::Matrix transM = Cairo::translation_matrix(scaleX->get_value(), scaleY->get_value());
                        signals.glyph_transform_updated.emit(origTransform * transM);
                    };

                    scaleX->signal_value_changed().connect(update_translation);
                    scaleY->signal_value_changed().connect(update_translation);
                }

                d.add_button("Apply", Gtk::ResponseType::RESPONSE_OK);
                d.add_button("Cancel", Gtk::ResponseType::RESPONSE_CANCEL);

                d.signal_response().connect([this, &origTransform](int response)
                {
                    if (response == Gtk::ResponseType::RESPONSE_OK) {
                        // Transform done
                    } else {
                        signals.glyph_transform_updated.emit(origTransform);
                    }
                });

                d.run();
            }),
            menuItem("Rotate", [this]()
            {
                Gtk::Dialog d("Rotation", Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_USE_HEADER_BAR);

                const Cairo::Matrix origTransform = m_glyphTransform;

                {
                    auto *grid = Gtk::make_managed<Gtk::Grid>();

                    Glib::RefPtr<Gtk::Adjustment> adj = Gtk::Adjustment::create(0.0, -180, 180, 1.0, 5.0, 0.0);

                    auto *scale = Gtk::make_managed<Gtk::Scale>(adj, Gtk::ORIENTATION_HORIZONTAL);
                    scale->set_hexpand();
                    scale->set_digits(3);
                    scale->set_value_pos(Gtk::POS_TOP);
                    scale->set_draw_value();
                    scale->show();

                    auto *label = Gtk::make_managed<Gtk::Label>("Degrees");
                    label->show();

                    grid->attach(*label, 1, 1);
                    grid->attach(*scale, 2, 1);
                    grid->show();
                    d.get_content_area()->add(*grid);

                    scale->signal_value_changed().connect([this, scale, &origTransform]()
                    {
                        Cairo::Matrix rotM = Cairo::rotation_matrix(scale->get_value() * M_PI / 180);
                        signals.glyph_transform_updated.emit(origTransform * rotM);
                    });
                }

                d.add_button("Apply", Gtk::ResponseType::RESPONSE_OK);
                d.add_button("Cancel", Gtk::ResponseType::RESPONSE_CANCEL);

                d.signal_response().connect([this, &origTransform](int response)
                {
                    if (response == Gtk::ResponseType::RESPONSE_OK) {
                        // Transform done
                    } else {
                        signals.glyph_transform_updated.emit(origTransform);
                    }
                });

                d.run();
            }),
            menuItem("Scale", [this]()
            {
                Gtk::Dialog d("Scaling", Gtk::DialogFlags::DIALOG_MODAL | Gtk::DialogFlags::DIALOG_USE_HEADER_BAR);

                const Cairo::Matrix origTransform = m_glyphTransform;

                {
                    auto *grid = Gtk::make_managed<Gtk::Grid>();

                    Glib::RefPtr<Gtk::Adjustment> adjX = Gtk::Adjustment::create(1.0, -5.0, 5.0, 0.01, 0.1, 0.0);
                    auto *scaleX = Gtk::make_managed<Gtk::Scale>(adjX, Gtk::ORIENTATION_HORIZONTAL);
                    scaleX->set_hexpand();
                    scaleX->set_digits(3);
                    scaleX->set_value_pos(Gtk::POS_TOP);
                    scaleX->set_draw_value();
                    scaleX->show();

                    auto *labelX = Gtk::make_managed<Gtk::Label>("Scale X");
                    labelX->show();

                    Glib::RefPtr<Gtk::Adjustment> adjY = Gtk::Adjustment::create(1.0, -5.0, 5.0, 0.01, 0.1, 0.0);
                    auto *scaleY = Gtk::make_managed<Gtk::Scale>(adjY, Gtk::ORIENTATION_HORIZONTAL);
                    scaleY->set_hexpand();
                    scaleY->set_digits(3);
                    scaleY->set_value_pos(Gtk::POS_TOP);
                    scaleY->set_draw_value();
                    scaleY->show();

                    auto *labelY = Gtk::make_managed<Gtk::Label>("Scale Y");
                    labelY->show();

                    grid->attach(*labelX, 1, 1);
                    grid->attach(*scaleX, 2, 1);
                    grid->attach(*labelY, 1, 2);
                    grid->attach(*scaleY, 2, 2);
                    grid->show();
                    d.get_content_area()->add(*grid);

                    auto update_translation = [this, scaleX, scaleY, &origTransform]()
                    {
                        Cairo::Matrix scaleM = Cairo::scaling_matrix(scaleX->get_value(), scaleY->get_value());
                        signals.glyph_transform_updated.emit(origTransform * scaleM);
                    };

                    scaleX->signal_value_changed().connect(update_translation);
                    scaleY->signal_value_changed().connect(update_translation);
                }

                d.add_button("Apply", Gtk::ResponseType::RESPONSE_OK);
                d.add_button("Cancel", Gtk::ResponseType::RESPONSE_CANCEL);

                d.signal_response().connect([this, &origTransform](int response)
                {
                    if (response == Gtk::ResponseType::RESPONSE_OK) {
                        // Transform done
                    } else {
                        signals.glyph_transform_updated.emit(origTransform);
                    }
                });

                d.run();
            })
        });


        auto *menubtn = Gtk::make_managed<Gtk::MenuButton>();
        menubtn->set_always_show_image();
        menubtn->set_label("Update");
        menubtn->set_image_from_icon_name("pan-down-symbolic");
        menubtn->set_menu(*menuu);
        menubtn->show();

        wGrid->attach_next_to(*menubtn, Gtk::PositionType::POS_BOTTOM);

        return *wGrid;
    }

    void font_redraw()
    {
        if (_face == nullptr)
        {
            if (FT_New_Face(_ft, m_selectedFontPath.c_str(), 0, &_face)) throw std::runtime_error("FT_New_Face");
            for (const auto &f : m_onFaceReload) {
                f(_face);
            }

            hasFixedSizes = bool(_face->num_fixed_sizes);
            for (int i = 0; i < _face->num_fixed_sizes; ++i) {
                auto &sz = _face->available_sizes[i];
                std::cerr << sz.height << " " << sz.width << " " << sz.size << " " << sz.x_ppem << " " << sz.y_ppem << "\n";
            }
        }

        if (hasFixedSizes)
        {
            FT_Select_Size(_face, 0);
        }
        else
        {
            FT_Set_Char_Size(_face, m_charSize*64, m_charSize*64, 0, 0);
        }

        {
            FT_Matrix matrix = {};
            FT_Vector vector = {};
            matrix.xx = round(m_glyphTransform.xx * 65536.0);
            matrix.xy = round(m_glyphTransform.xy * 65536.0);
            matrix.yx = round(m_glyphTransform.yx * 65536.0);
            matrix.yy = round(m_glyphTransform.yy * 65536.0);
            vector.x  = round(m_glyphTransform.x0 * 64.0);
            vector.y  = round(m_glyphTransform.y0 * 64.0);

            FT_Set_Transform(_face, &matrix, &vector);
        }

        FT_Error errorCode = FT_Load_Char(_face, m_charCode, m_loadFlags);
        if (errorCode) throw FreetypeError(errorCode, "FT_Load_Char");
        errorCode = FT_Render_Glyph(_face->glyph, m_renderMode);
        if (errorCode)
        {
            // On FT-2.11.0, NotoColorEmoji.ttf seems to return error 19, but render fine??, TODO
            // throw FreetypeError(errorCode, "FT_Render_Glyph");
            std::cerr << "Failed rendering char " << m_charCode << " error code " << errorCode << "\n";
        }

        for (const auto &f : m_onFontReload) {
            f(_face);
        }

        signals.font_reloaded.emit(_face);
    }

    Signals signals;

    FontGlyphSelectorColumns columns;
    int m_charCode = -1;
    int m_charSize = 13;
    int m_loadFlags = 0;

    FreetypeBitmapDrawer *m_drawer = nullptr;

    Cairo::Matrix m_glyphTransform = Cairo::identity_matrix();

    FT_Render_Mode m_renderMode = FT_RENDER_MODE_LCD;

    std::string m_selectedFontPath;
    std::string m_selectedFontName;

    FT_Library _ft;
    FT_Face _face = nullptr;

    bool hasFixedSizes = false;
    bool beingCleared = false;

    std::vector<std::function<void(FT_Face)>> m_onFaceReload;
    std::vector<std::function<void(FT_Face)>> m_onFontReload;
};

int main(int argc, char** argv)
{
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);
    FontDebug win;
    return app->run(win);
}

