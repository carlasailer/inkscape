// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Inkscape color swatch UI item.
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_DIALOGS_COLOR_ITEM_H
#define SEEN_DIALOGS_COLOR_ITEM_H

#include <boost/ptr_container/ptr_vector.hpp>

#include "widgets/ege-paint-def.h"
#include "ui/previewable.h"

class SPGradient;

namespace Inkscape {
namespace UI {
namespace Dialog {

class ColorItem;

class SwatchPage
{
public:
    SwatchPage();
    ~SwatchPage();

    Glib::ustring _name;
    int _prefWidth;
    boost::ptr_vector<ColorItem> _colors;
};


/**
 * The color swatch you see on screen as a clickable box.
 */
class ColorItem : public Inkscape::UI::Previewable
{
    friend void _loadPaletteFile( gchar const *filename );
public:
    ColorItem( ege::PaintDef::ColorType type );
    ColorItem( unsigned int r, unsigned int g, unsigned int b,
               Glib::ustring& name );
    ~ColorItem() override;
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);
    Gtk::Widget* getPreview(UI::Widget::PreviewStyle style,
                            UI::Widget::ViewType     view,
                            UI::Widget::PreviewSize  size,
                            guint                    ratio,
                            guint                    border) override;
    void buttonClicked(bool secondary = false);

    void setGradient(SPGradient *grad);
    SPGradient * getGradient() const { return _grad; }
    void setPattern(cairo_pattern_t *pattern);
    void setName(const Glib::ustring name);

    void setState( bool fill, bool stroke );
    bool isFill() { return _isFill; }
    bool isStroke() { return _isStroke; }

    ege::PaintDef def;

    Gtk::Widget* createWidget();

private:
    Gtk::Widget* _getPreview(UI::Widget::PreviewStyle style,
		  UI::Widget::ViewType view, UI::Widget::PreviewSize size,
		  guint ratio, guint border);

    static void _dropDataIn( GtkWidget *widget,
                             GdkDragContext *drag_context,
                             gint x, gint y,
                             GtkSelectionData *data,
                             guint info,
                             guint event_time,
                             gpointer user_data);

    void _dragGetColorData(const Glib::RefPtr<Gdk::DragContext> &drag_context,
                           Gtk::SelectionData                   &data,
                           guint                                 info,
                           guint                                 time);

    static void _wireMagicColors( SwatchPage *colorSet );
    static void _colorDefChanged(void* data);

    void _updatePreviews();
    void _regenPreview(UI::Widget::Preview * preview);

    void _linkTint( ColorItem& other, int percent );
    void _linkTone( ColorItem& other, int percent, int grayLevel );
    void drag_begin(const Glib::RefPtr<Gdk::DragContext> &dc);
    void handleClick();
    void handleSecondaryClick(gint arg1);
    bool handleEnterNotify(GdkEventCrossing* event);
    bool handleLeaveNotify(GdkEventCrossing* event);

    std::vector<Gtk::Widget*> _previews;

    bool _isFill;
    bool _isStroke;
    bool _isLive;
    bool _linkIsTone;
    int _linkPercent;
    int _linkGray;
    ColorItem* _linkSrc;
    SPGradient* _grad;
    cairo_pattern_t *_pattern;
    std::vector<ColorItem*> _listeners;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // SEEN_DIALOGS_COLOR_ITEM_H

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
