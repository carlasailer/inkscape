// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Parameters for extensions.
 */
/* Authors:
 *   Ted Gould <ted@gould.cx>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_INK_EXTENSION_PARAM_H__
#define SEEN_INK_EXTENSION_PARAM_H__

#include <string>

#include <sigc++/sigc++.h>
#include <glibmm/ustring.h>

class SPDocument;

namespace Gtk {
class Widget;
}

namespace Inkscape {
namespace XML {
class Node;
}

namespace Extension {

class Extension;


/**
 * The root directory in the preferences database for extension-related parameters.
 *
 * The directory path has both a leading and a trailing slash, so that extension_pref_root + pref_name works
 * without having to append a separator.
 */
extern Glib::ustring const extension_pref_root;

/**
 * A class to represent the parameter of an extension.
 *
 * This is really a super class that allows them to abstract all
 * the different types of parameters into some that can be passed
 * around.  There is also a few functions that are used by all the
 * different parameters.
 */
class Parameter {
public:

    enum Translatable {
        UNSET, YES, NO
    };

    Parameter(gchar const *name,
              gchar const *text,
              gchar const *description,
              bool hidden,
              int indent,
              Inkscape::Extension::Extension *ext);

    Parameter(gchar const *name,
              gchar const *text,
              Inkscape::Extension::Extension *ext);

    Parameter(Inkscape::XML::Node *in_repr,
              Inkscape::Extension::Extension *ext);

    virtual ~Parameter();

    /** Wrapper to cast to the object and use its function. */
    bool get_bool(SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    int get_int(SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    float get_float(SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    const char *get_string(SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    const char *get_optiongroup(SPDocument const * doc, Inkscape::XML::Node const *node) const;
    bool get_optiongroup_contains(const char *value, SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    guint32 get_color(SPDocument const *doc, Inkscape::XML::Node const *node) const;

    /** Wrapper to cast to the object and use it's function. */
    bool set_bool(bool in, SPDocument *doc, Inkscape::XML::Node *node);

    /** Wrapper to cast to the object and use it's function. */
    int set_int(int  in, SPDocument *doc, Inkscape::XML::Node *node);

    /** Wrapper to cast to the object and use it's function. */
    float set_float(float in, SPDocument *doc, Inkscape::XML::Node *node);

    /** Wrapper to cast to the object and use it's function. */
    const char *set_string(const char *in, SPDocument *doc, Inkscape::XML::Node *node);

    /** Wrapper to cast to the object and use it's function. */
    const char *set_optiongroup(const char *in, SPDocument *doc, Inkscape::XML::Node *node);

    /** Wrapper to cast to the object and use it's function. */
    guint32 set_color(guint32 in, SPDocument *doc, Inkscape::XML::Node *node);

    gchar const *name() const { return _name; }

    /**
     * This function creates a parameter that can be used later.  This
     * is typically done in the creation of the extension and defined
     * in the XML file describing the extension (it's private so people
     * have to use the system) :)
     *
     * This function first grabs all of the data out of the Repr and puts
     * it into local variables.  Actually, these are just pointers, and the
     * data is not duplicated so we need to be careful with it.  If there
     * isn't a name or a type in the XML, then no parameter is created as
     * the function just returns.
     *
     * From this point on, we're pretty committed as we've allocated an
     * object and we're starting to fill it.  The name is set first, and
     * is created with a strdup to actually allocate memory for it.  Then
     * there is a case statement (roughly because strcmp requires 'ifs')
     * based on what type of parameter this is.  Depending which type it
     * is, the value is interpreted differently, but they are relatively
     * straight forward.  In all cases the value is set to the default
     * value from the XML and the type is set to the interpreted type.
     *
     * @param  in_repr The XML describing the parameter.
     * @return a pointer to a new Parameter if applicable, null otherwise..
     */
    static Parameter *make(Inkscape::XML::Node *in_repr, Inkscape::Extension::Extension *in_ext);

    virtual Gtk::Widget *get_widget(SPDocument *doc, Inkscape::XML::Node *node, sigc::signal<void> *changeSignal);

    const gchar *get_tooltip() const { return _description; }

    /** Indicates if the GUI for this parameter is hidden or not */
    bool get_hidden() const { return _hidden; }

    /** Indentation level of the parameter */
    int get_indent() const { return _indent; }

    virtual void string(std::list <std::string> &list) const;

    /**
     * Gets the current value of the parameter in a string form.
     * @return A string with the 'value'.
     */
    virtual void string(std::string &string) const;

    /** All the code in Notebook::get_param to get the notebook content. */
    virtual Parameter *get_param(gchar const *name);


    /** Recommended margin of boxes containing multiple Parameters (in px) */
    const static int GUI_BOX_MARGIN = 10;
    /** Recommended spacing between multiple Parameters packed into a box (in px) */
    const static int GUI_BOX_SPACING = 4;
    /** Recommended spacing between the widgets making up a single Parameter (e.g. label and input) (in px) */
    const static int GUI_PARAM_WIDGETS_SPACING = 4;
    /** Recommended indentation width of parameters (in px) */
    const static int GUI_INDENTATION = 12;
    /** Recommended maximum line length for wrapping textual parameters (in chars) */
    const static int GUI_MAX_LINE_LENGTH = 60;


    /** An error class for when a parameter is called on a type it is not */
    class param_no_name {};
    class param_no_type {};
    class param_not_bool_param {};
    class param_not_color_param {};
    class param_not_float_param {};
    class param_not_int_param {};
    class param_not_optiongroup_param {};
    class param_not_string_param {};


protected:
    /** Which extension is this parameter attached to. */
    Inkscape::Extension::Extension *_extension = nullptr;

    /** The name of this parameter. */
    gchar *_name = nullptr;

    /** Parameter text to show as the GUI label. */
    gchar *_text = nullptr;

    /** Extended description of the parameter (currently shown as tooltip on hover). */
    gchar *_description = nullptr;

    /** Whether the parameter is visible. */
    bool _hidden = false;

    /** Indentation level of the parameter. */
    int _indent = 0;

    /** Appearance of the parameter (not used by all Parameters). */
    gchar *_appearance = nullptr;

    /** Is parameter translatable? */
    Translatable _translatable = UNSET;

    /** context for translation of translatable strings. */
    gchar *_context = nullptr;


    /* **** funcs **** */

    /**
     * Build the name to write the parameter from the extension's ID and the name of this parameter.
     */
    gchar *pref_name() const;
};

}  // namespace Extension
}  // namespace Inkscape

#endif // SEEN_INK_EXTENSION_PARAM_H__

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
