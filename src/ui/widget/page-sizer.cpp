/** \file
 *
 * Paper-size widget and helper functions
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <glibmm/i18n.h>
#include <gtkmm/label.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/menuitem.h>

#include "ui/widget/scalar-unit.h"
#include "ui/widget/unit-menu.h"

#include "helper/units.h"
#include "util/units.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "document.h"
#include "page-sizer.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Widget {

struct PaperSize {
    char const * const name;
    double const smaller;
    double const larger;
    SPUnitId const unit;
};

    /** \note
     * The ISO page sizes in the table below differ from ghostscript's idea of page sizes (by
     * less than 1pt).  Being off by <1pt should be OK for most purposes, but may cause fuzziness
     * (antialiasing) problems when printing to 72dpi or 144dpi printers or bitmap files due to
     * postscript's different coordinate system (y=0 meaning bottom of page in postscript and top
     * of page in SVG).  I haven't looked into whether this does in fact cause fuzziness, I merely
     * note the possibility.  Rounding done by extension/internal/ps.cpp (e.g. floor/ceil calls)
     * will also affect whether fuzziness occurs.
     *
     * The remainder of this comment discusses the origin of the numbers used for ISO page sizes in
     * this table and in ghostscript.
     *
     * The versions here, in mm, are the official sizes according to
     * <a href="http://en.wikipedia.org/wiki/Paper_sizes">http://en.wikipedia.org/wiki/Paper_sizes</a> 
     * at 2005-01-25.  (The ISO entries in the below table
     * were produced mechanically from the table on that page.)
     *
     * (The rule seems to be that A0, B0, ..., D0. sizes are rounded to the nearest number of mm
     * from the "theoretical size" (i.e. 1000 * sqrt(2) or pow(2.0, .25) or the like), whereas
     * going from e.g. A0 to A1 always take the floor of halving -- which by chance coincides
     * exactly with flooring the "theoretical size" for n != 0 instead of the rounding to nearest
     * done for n==0.)
     *
     * Ghostscript paper sizes are given in gs_statd.ps according to gs(1).  gs_statd.ps always
     * uses an integer number of pt: sometimes gs_statd.ps rounds to nearest (e.g. a1), sometimes
     * floors (e.g. a10), sometimes ceils (e.g. a8).
     *
     * I'm not sure how ghostscript's gs_statd.ps was calculated: it isn't just rounding the
     * "theoretical size" of each page to pt (see a0), nor is it rounding the a0 size times an
     * appropriate power of two (see a1).  Possibly it was prepared manually, with a human applying
     * inconsistent rounding rules when converting from mm to pt.
     */
    /** \todo
     * Should we include the JIS B series (used in Japan)  
     * (JIS B0 is sometimes called JB0, and similarly for JB1 etc)?
     * Should we exclude B7--B10 and A7--10 to make the list smaller ?
     * Should we include any of the ISO C, D and E series (see below) ?
     */

static PaperSize const inkscape_papers[] = {
    { "A4", 210, 297, SP_UNIT_MM },
    { "US Letter", 8.5, 11, SP_UNIT_IN },
    { "US Legal", 8.5, 14, SP_UNIT_IN },
    { "US Executive", 7.25, 10.5, SP_UNIT_IN },
    { "A0", 841, 1189, SP_UNIT_MM },
    { "A1", 594, 841, SP_UNIT_MM },
    { "A2", 420, 594, SP_UNIT_MM },
    { "A3", 297, 420, SP_UNIT_MM },
    { "A5", 148, 210, SP_UNIT_MM },
    { "A6", 105, 148, SP_UNIT_MM },
    { "A7", 74, 105, SP_UNIT_MM },
    { "A8", 52, 74, SP_UNIT_MM },
    { "A9", 37, 52, SP_UNIT_MM },
    { "A10", 26, 37, SP_UNIT_MM },
    { "B0", 1000, 1414, SP_UNIT_MM },
    { "B1", 707, 1000, SP_UNIT_MM },
    { "B2", 500, 707, SP_UNIT_MM },
    { "B3", 353, 500, SP_UNIT_MM },
    { "B4", 250, 353, SP_UNIT_MM },
    { "B5", 176, 250, SP_UNIT_MM },
    { "B6", 125, 176, SP_UNIT_MM },
    { "B7", 88, 125, SP_UNIT_MM },
    { "B8", 62, 88, SP_UNIT_MM },
    { "B9", 44, 62, SP_UNIT_MM },
    { "B10", 31, 44, SP_UNIT_MM },

#if 0 /* Whether to include or exclude these depends on how big we mind our page size menu
         becoming.  C series is used for envelopes; don't know what D and E series are used for. */
    { "C0", 917, 1297, SP_UNIT_MM },
    { "C1", 648, 917, SP_UNIT_MM },
    { "C2", 458, 648, SP_UNIT_MM },
    { "C3", 324, 458, SP_UNIT_MM },
    { "C4", 229, 324, SP_UNIT_MM },
    { "C5", 162, 229, SP_UNIT_MM },
    { "C6", 114, 162, SP_UNIT_MM },
    { "C7", 81, 114, SP_UNIT_MM },
    { "C8", 57, 81, SP_UNIT_MM },
    { "C9", 40, 57, SP_UNIT_MM },
    { "C10", 28, 40, SP_UNIT_MM },
    { "D1", 545, 771, SP_UNIT_MM },
    { "D2", 385, 545, SP_UNIT_MM },
    { "D3", 272, 385, SP_UNIT_MM },
    { "D4", 192, 272, SP_UNIT_MM },
    { "D5", 136, 192, SP_UNIT_MM },
    { "D6", 96, 136, SP_UNIT_MM },
    { "D7", 68, 96, SP_UNIT_MM },
    { "E3", 400, 560, SP_UNIT_MM },
    { "E4", 280, 400, SP_UNIT_MM },
    { "E5", 200, 280, SP_UNIT_MM },
    { "E6", 140, 200, SP_UNIT_MM },
#endif

    { "CSE", 462, 649, SP_UNIT_PT },
    { "US #10 Envelope", 4.125, 9.5, SP_UNIT_IN }, // TODO: Select landscape by default.
    /* See http://www.hbp.com/content/PCR_envelopes.cfm for a much larger list of US envelope
       sizes. */
    { "DL Envelope", 110, 220, SP_UNIT_MM }, // TODO: Select landscape by default.
    { "Ledger/Tabloid", 11, 17, SP_UNIT_IN },
    /* Note that `Folio' (used in QPrinter/KPrinter) is deliberately absent from this list, as it
       means different sizes to different people: different people may expect the width to be
       either 8, 8.25 or 8.5 inches, and the height to be either 13 or 13.5 inches, even
       restricting our interpretation to foolscap folio.  If you wish to introduce a folio-like
       page size to the list, then please consider using a name more specific than just `Folio' or
       `Foolscap Folio'. */
    { "Banner 468x60", 60, 468, SP_UNIT_PX },  // TODO: Select landscape by default.
    { "Icon 16x16", 16, 16, SP_UNIT_PX },
    { "Icon 32x32", 32, 32, SP_UNIT_PX },
    { NULL, 0, 0, SP_UNIT_PX },
};

//===================================================
static const SPUnit _px_unit = sp_unit_get_by_id (SP_UNIT_PX);

class SizeMenuItem : public Gtk::MenuItem {
public:
    SizeMenuItem (PaperSize const * paper, PageSizer * widget)
    : Gtk::MenuItem (paper ? paper->name : _("Custom")), 
      _paper(paper), _parent(widget) {}
protected:
    PaperSize const * _paper;
    PageSizer *_parent;
    void on_activate();
};

void
SizeMenuItem::on_activate()
{
    if (_parent == 0) // handle Custom entry
        return;
        
    double w = _paper->smaller, h = _paper->larger;
    SPUnit const &src_unit = sp_unit_get_by_id (_paper->unit);
    sp_convert_distance (&w, &src_unit, &_px_unit);
    sp_convert_distance (&h, &src_unit, &_px_unit);
    if (_parent->_landscape)
        _parent->setDim (h, w);
    else
        _parent->setDim (w, h);
}

//---------------------------------------------------

PageSizer::PageSizer()
: Gtk::VBox(false,4)
{
    Gtk::HBox *hbox_size = manage (new Gtk::HBox (false, 4));
    pack_start (*hbox_size, false, false, 0);
    Gtk::Label *label_size = manage (new Gtk::Label (_("Page size:"), 1.0, 0.5)); 
    hbox_size->pack_start (*label_size, false, false, 0);
    _omenu_size = manage (new Gtk::OptionMenu);
    hbox_size->pack_start (*_omenu_size, true, true, 0);
    Gtk::Menu *menu_size = manage (new Gtk::Menu);

    for (PaperSize const *paper = inkscape_papers; paper->name; paper++) {
        SizeMenuItem *item = manage (new SizeMenuItem (paper, this));
        menu_size->append (*item);
    }
    SizeMenuItem *item = manage (new SizeMenuItem (0, 0));
    menu_size->prepend (*item);
    _omenu_size->set_menu (*menu_size);

    Gtk::HBox *hbox_ori = manage (new Gtk::HBox (false, 4));
    pack_start (*hbox_ori, false, false, 0);
    Gtk::Label *label_ori = manage (new Gtk::Label (_("Page orientation:"), 1.0, 0.5)); 
    hbox_ori->pack_start (*label_ori, false, false, 0);
    _omenu_ori = manage (new Gtk::OptionMenu);
    hbox_ori->pack_start (*_omenu_ori, true, true, 0);
    Gtk::Menu *menu_ori = manage (new Gtk::Menu);

    Gtk::MenuItem *oitem;
    oitem = manage (new Gtk::MenuItem (_("Landscape")));
    _landscape_connection = oitem->signal_activate().connect (sigc::mem_fun (*this, &PageSizer::on_landscape));
    menu_ori->prepend (*oitem);
    oitem = manage (new Gtk::MenuItem (_("Portrait")));
    _portrait_connection = oitem->signal_activate().connect (sigc::mem_fun (*this, &PageSizer::on_portrait));
    menu_ori->prepend (*oitem);
    _omenu_ori->set_menu (*menu_ori);
}

PageSizer::~PageSizer()
{
    _portrait_connection.disconnect();
    _landscape_connection.disconnect();
    _changedw_connection.disconnect();
    _changedh_connection.disconnect();
}

void
PageSizer::init (Registry& reg)
{
    /* Custom paper frame */
    Gtk::Frame *frame = manage (new Gtk::Frame(_("Custom size")));
    pack_start (*frame, false, false, 0);
    Gtk::Table *table = manage (new Gtk::Table (4, 2, false));
    table->set_border_width (4);
    table->set_row_spacings (4);
    table->set_col_spacings (4);
    frame->add (*table);
    
    _wr = &reg;

    _rum.init (_("Units:"), "units", *_wr);
    _rusw.init (_("Width:"), _("Width of paper"), "width", _rum, *_wr);
    _rush.init (_("Height:"), _("Height of paper"), "height", _rum, *_wr);

    table->attach (*_rum._label, 0,1,0,1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
    table->attach (*_rum._sel, 1,2,0,1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
    table->attach (*_rusw.getSU(), 0,2,1,2, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
    table->attach (*_rush.getSU(), 0,2,2,3, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);

    _changedw_connection = _rusw.getSU()->signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _changedh_connection = _rush.getSU()->signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    
    show_all_children();
}

/**
 * \param w, h given in px
 */
void 
PageSizer::setDim (double w, double h)
{
    _landscape = w>h;
    _omenu_ori->set_history (_landscape ? 1 : 0);
    _omenu_size->set_history (1 + find_paper_size (w, h));
    
    Unit const& unit = _rum._sel->getUnit();
    _rusw.setValue (w / unit.factor);
    _rush.setValue (h / unit.factor);
}

void
PageSizer::setDoc (double w, double h)
{
    setDim (w, h);
    if (!SP_ACTIVE_DESKTOP)
        return;

    SPDocument *doc = SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP);
    sp_document_set_width (doc, _rusw.getSU()->getValue("px"), &_px_unit);
    sp_document_set_height (doc, _rush.getSU()->getValue("px"), &_px_unit);
    sp_document_done (doc);
}

/** 
 * Returns an index into inkscape_papers of a paper of the specified 
 * size (specified in px), or -1 if there's no such paper.
 */
int
PageSizer::find_paper_size (double w, double h) const
{
    double given[2];
    if ( w < h ) {
        given[0] = w; given[1] = h;
    } else {
        given[0] = h; given[1] = w;
    }
    g_return_val_if_fail(given[0] <= given[1], -1);
    for (unsigned i = 0; i < G_N_ELEMENTS(inkscape_papers) - 1; ++i) {
        SPUnit const &i_unit = sp_unit_get_by_id(inkscape_papers[i].unit);
        double const i_sizes[2] = { sp_units_get_pixels(inkscape_papers[i].smaller, i_unit),
                                    sp_units_get_pixels(inkscape_papers[i].larger, i_unit) };
        g_return_val_if_fail(i_sizes[0] <= i_sizes[1], -1);
        if ((std::abs(given[0] - i_sizes[0]) <= .1) &&
            (std::abs(given[1] - i_sizes[1]) <= .1)   )
        {
            return (int) i;
        }
    }
    return -1;
}

void
PageSizer::on_portrait()
{
    double w = _rusw.getSU()->getValue ("px");
    double h = _rush.getSU()->getValue ("px");
    if (h<w) setDoc (h, w);
}

void
PageSizer::on_landscape()
{
    double w = _rusw.getSU()->getValue ("px");
    double h = _rush.getSU()->getValue ("px");
    if (w<h) setDoc (h, w);
}

void
PageSizer::on_value_changed()
{
    setDoc (_rusw.getSU()->getValue("px"), _rush.getSU()->getValue("px"));
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
