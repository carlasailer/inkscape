// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INK_EXTENSION_PARAMSTRING_H_SEEN
#define INK_EXTENSION_PARAMSTRING_H_SEEN

/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 *   Jon A. Cruz <jon@joncruz.org>
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "parameter.h"

namespace Glib {
class ustring;
}

namespace Inkscape {
namespace Extension {

class ParamString : public Parameter {
public:
    ParamString(Inkscape::XML::Node *xml, Inkscape::Extension::Extension *ext);

    /** \brief  Returns \c _value, with a \i const to protect it. */
    const Glib::ustring& get(SPDocument const * /*doc*/, Inkscape::XML::Node const * /*node*/) const { return _value; }
    const Glib::ustring& set(const Glib::ustring in, SPDocument *doc, Inkscape::XML::Node *node);

    Gtk::Widget *get_widget(SPDocument *doc, Inkscape::XML::Node *node, sigc::signal<void> *changeSignal) override;

    // Explicitly call superclass version to avoid method being hidden.
    void string(std::list <std::string> &list) const override { return Parameter::string(list); }

    void string(std::string &string) const override;

    void setMaxLength(int maxLength) { _max_length = maxLength; }
    int getMaxLength() { return _max_length; }

private:
    /** \brief  Internal value. */
    Glib::ustring _value;

    /** \brief Maximum length of the string in characters (zero meaning unlimited).
      */
    int _max_length = 0;
};


}  // namespace Extension
}  // namespace Inkscape

#endif /* INK_EXTENSION_PARAMSTRING_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
