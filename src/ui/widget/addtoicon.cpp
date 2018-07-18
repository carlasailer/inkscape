/*
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui/widget/addtoicon.h"

#include "helper/icon-loader.h"
#include "layertypeicon.h"
#include "ui/icon-names.h"
#include "widgets/toolbox.h"

namespace Inkscape {
namespace UI {
namespace Widget {

AddToIcon::AddToIcon() :
    Glib::ObjectBase(typeid(AddToIcon)),
    Gtk::CellRendererPixbuf(),
//    _pixAddName(INKSCAPE_ICON("layer-new")),
    _property_active(*this, "active", false)
//    _property_pixbuf_add(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(0))
{
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;


//    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();
//
//    if (!icon_theme->has_icon(_pixAddName)) {
//        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixAddName.data()), Inkscape::ICON_SIZE_DECORATION );
//    }
//    if (icon_theme->has_icon(_pixAddName)) {
//        _property_pixbuf_add = icon_theme->load_icon(_pixAddName, phys, (Gtk::IconLookupFlags)0);
//    }

    set_pixbuf();
}

void AddToIcon::get_preferred_height_vfunc(Gtk::Widget& widget,
                                              int& min_h,
                                              int& nat_h) const
{
    Gtk::CellRendererPixbuf::get_preferred_height_vfunc(widget, min_h, nat_h);

    if (min_h) {
        min_h += (min_h) >> 1;
    }
    
    if (nat_h) {
        nat_h += (nat_h) >> 1;
    }
}

void AddToIcon::get_preferred_width_vfunc(Gtk::Widget& widget,
                                             int& min_w,
                                             int& nat_w) const
{
    Gtk::CellRendererPixbuf::get_preferred_width_vfunc(widget, min_w, nat_w);

    if (min_w) {
        min_w += (min_w) >> 1;
    }
    
    if (nat_w) {
        nat_w += (nat_w) >> 1;
    }
}

void AddToIcon::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
{
    set_pixbuf();
    
    Gtk::CellRendererPixbuf::render_vfunc( cr, widget, background_area, cell_area, flags );
}

bool AddToIcon::activate_vfunc(GdkEvent* /*event*/,
                               Gtk::Widget& /*widget*/,
                               const Glib::ustring& /*path*/,
                               const Gdk::Rectangle& /*background_area*/,
                               const Gdk::Rectangle& /*cell_area*/,
                               Gtk::CellRendererState /*flags*/)
{
    return false;
}

void AddToIcon::set_pixbuf()
{
    bool active = property_active().get_value();
    property_pixbuf() = sp_get_icon_pixbuf((active ? "list-add" : "edit-delete"), GTK_ICON_SIZE_BUTTON);
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
