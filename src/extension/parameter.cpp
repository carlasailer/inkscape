/** \file
 * Parameters for extensions.
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>

#include <glibmm/i18n.h>

#include "extension.h"
#include "prefs-utils.h"

#include "parameter.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

/** \brief  A boolean parameter */
class ParamBool : public Parameter {
private:
    /** \brief  Internal value. */
    bool _value;
public:
    ParamBool(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    bool get (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node) { return _value; }
    bool set (bool in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(void);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamBool::ParamBool (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, ext), _value(false)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL && (!strcmp(defaultval, "TRUE") || !strcmp(defaultval, "true") || !strcmp(defaultval, "1"))) {
        _value = true;
    } else {
        _value = false;
    }

    gchar * pref_name = this->pref_name();
    _value = (bool)prefs_get_int_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    return;
}

class ParamInt : public Parameter {
private:
    /** \brief  Internal value. */
    int _value;
    int _min;
    int _max;
public:
    ParamInt (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    int get (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node) { return _value; }
    int set (int in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node);
    int max (void) { return _max; }
    int min (void) { return _min; }
    Gtk::Widget * get_widget(void);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamInt::ParamInt (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, ext), _value(0), _min(0), _max(10)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();
    if (defaultval != NULL) {
        _value = atoi(defaultval);
    }

    const char * maxval = xml->attribute("max");
    if (maxval != NULL)
        _max = atoi(maxval);

    const char * minval = xml->attribute("min");
    if (minval != NULL)
        _min = atoi(minval);

    /* We're handling this by just killing both values */
    if (_max < _min) {
        _max = 10;
        _min = 0;
    }

    gchar * pref_name = this->pref_name();
    _value = prefs_get_int_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    // std::cout << "New Int::  value: " << _value << "  max: " << _max << "  min: " << _min << std::endl;

    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    return;
}

class ParamFloat : public Parameter {
private:
    /** \brief  Internal value. */
    float _value;
    float _min;
    float _max;
public:
    ParamFloat (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    float get (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node) { return _value; }
    float set (float in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node);
    float max (void) { return _max; }
    float min (void) { return _min; }
    Gtk::Widget * get_widget(void);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamFloat::ParamFloat (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, ext), _value(0.0), _min(0.0), _max(10.0)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();
    if (defaultval != NULL) {
        _value = atof(defaultval);
    }

    const char * maxval = xml->attribute("max");
    if (maxval != NULL)
        _max = atof(maxval);

    const char * minval = xml->attribute("min");
    if (minval != NULL)
        _min = atof(minval);

    /* We're handling this by just killing both values */
    if (_max < _min) {
        _max = 10.0;
        _min = 0.0;
    }

    gchar * pref_name = this->pref_name();
    _value = prefs_get_double_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    // std::cout << "New Float::  value: " << _value << "  max: " << _max << "  min: " << _min << std::endl;

    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    return;
}

class ParamString : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd. */
    gchar * _value;
public:
    ParamString(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamString(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    const gchar * get (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node) { return _value; }
    const gchar * set (const gchar * in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(void);
    Glib::ustring * string (void);
};

/**
    \return None
    \brief  This function creates a parameter that can be used later.  This
            is typically done in the creation of the extension and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  in_repr  The XML describing the parameter

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually, these are just pointers, and the
    data is not duplicated so we need to be careful with it.  If there
    isn't a name or a type in the XML, then no parameter is created as
    the function just returns.

    From this point on, we're pretty committed as we've allocated an
    object and we're starting to fill it.  The name is set first, and
    is created with a strdup to actually allocate memory for it.  Then
    there is a case statement (roughly because strcmp requires 'ifs')
    based on what type of parameter this is.  Depending which type it
    is, the value is interpreted differently, but they are relatively
    straight forward.  In all cases the value is set to the default
    value from the XML and the type is set to the interpreted type.
*/
Parameter *
Parameter::make (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext)
{
    const char * name;
    const char * type;
    const char * guitext;

    name = in_repr->attribute("name");
    type = in_repr->attribute("type");
    guitext = in_repr->attribute("gui-text");
    if (guitext == NULL)
        guitext = in_repr->attribute("_gui-text");

    /* In this case we just don't have enough information */
    if (name == NULL || type == NULL) {
        return NULL;
    }

    Parameter * param = NULL;
    if (!strcmp(type, "boolean")) {
        param = new ParamBool(name, guitext, in_ext, in_repr);
    } else if (!strcmp(type, "int")) {
        param = new ParamInt(name, guitext, in_ext, in_repr);
    } else if (!strcmp(type, "float")) {
        param = new ParamFloat(name, guitext, in_ext, in_repr);
    } else if (!strcmp(type, "string")) {
        param = new ParamString(name, guitext, in_ext, in_repr);
    }

    /* Note: param could equal NULL */
    return param;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
bool
ParamBool::set (bool in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value == true ? 1 : 0);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
int
ParamInt::set (int in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    _value = in;
    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
float
ParamFloat::set (float in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    _value = in;
    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    gchar * prefname = this->pref_name();
    prefs_set_double_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.

    To copy the data into _value the old memory must be free'd first.
    It is important to note that \c g_free handles \c NULL just fine.  Then
    the passed in value is duplicated using \c g_strdup().
*/
const gchar *
ParamString::set (const gchar * in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    if (in == NULL) return NULL; /* Can't have NULL string */

    if (_value != NULL)
        g_free(_value);
    _value = g_strdup(in);

    gchar * prefname = this->pref_name();
    prefs_set_string_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::get_bool (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::get_int (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::get_float (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::get_string (const Inkscape::XML::Document * doc, const Inkscape::XML::Node * node)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::set_bool (bool in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::set_int (int in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::set_float (float in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::set_string (const gchar * in, Inkscape::XML::Document * doc, Inkscape::XML::Node * node)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->set(in, doc, node);
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamString::ParamString (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, ext), _value(NULL)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    gchar * pref_name = this->pref_name();
    const gchar * paramval = prefs_get_string_attribute(PREF_DIR, pref_name);
    g_free(pref_name);

    if (paramval != NULL)
        defaultval = paramval;
    if (defaultval != NULL)
        _value = g_strdup(defaultval);

    return;
}

/** \brief  Free the allocated data. */
ParamString::~ParamString(void)
{
    g_free(_value);
}

/** \brief  Oop, now that we need a parameter, we need it's name.  */
Parameter::Parameter (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext) :
    extension(ext), _name(NULL), _desc(NULL), _scope(scope), _text(NULL)
{
    if (name != NULL)
        _name = g_strdup(name);
    if (desc != NULL)
        _desc = g_strdup(desc);


    if (guitext != NULL)
        _text = g_strdup(guitext);
    else
        _text = g_strdup(name);

    return;
}

/** \brief  Just free the allocated name. */
Parameter::~Parameter (void)
{
    g_free(_name);
    g_free(_text);
}

/** \brief  Build the name to write the parameter from the extension's
            ID and the name of this parameter. */
gchar *
Parameter::pref_name (void)
{
    return g_strdup_printf("%s.%s", extension->get_id(), _name);
}

/** \brief  Basically, if there is no widget pass a NULL. */
Gtk::Widget *
Parameter::get_widget (void)
{
    return NULL;
}

/** \brief  If I'm not sure which it is, just don't return a value. */
Glib::ustring *
Parameter::string (void)
{
    Glib::ustring * mystring = new Glib::ustring("");
    return mystring;
}

/** \brief  A class to make an adjustment that uses Extension params */
class ParamFloatAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamFloat * _pref;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamFloatAdjustment (ParamFloat * param) :
            Gtk::Adjustment(0.0, param->min(), param->max(), 0.1), _pref(param) {
        this->set_value(_pref->get(NULL, NULL) /* \todo fix */);
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamFloatAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamFloatAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamFloatAdjustment::val_changed (void)
{
    // std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set(this->get_value(), NULL /* \todo fix */, NULL);
    return;
}

/** \brief  A class to make an adjustment that uses Extension params */
class ParamIntAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamInt * _pref;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamIntAdjustment (ParamInt * param) :
            Gtk::Adjustment(0.0, param->min(), param->max(), 1.0), _pref(param) {
        this->set_value(_pref->get(NULL, NULL) /* \todo fix */);
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamIntAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamIntAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamIntAdjustment::val_changed (void)
{
    // std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set((int)this->get_value(), NULL /* \todo fix */, NULL);
    return;
}

/**
    \brief  Creates a Float Adjustment for a float parameter

    Builds a hbox with a label and a float adjustment in it.
*/
Gtk::Widget *
ParamFloat::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamFloatAdjustment * fadjust = new ParamFloatAdjustment(this);
    Gtk::SpinButton * spin = new Gtk::SpinButton(*fadjust, 0.1, 1);
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/**
    \brief  Creates a Int Adjustment for a int parameter

    Builds a hbox with a label and a int adjustment in it.
*/
Gtk::Widget *
ParamInt::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamIntAdjustment * fadjust = new ParamIntAdjustment(this);
    Gtk::SpinButton * spin = new Gtk::SpinButton(*fadjust, 1.0, 0);
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  A check button which is Param aware.  It works with the
            parameter to change it's value as the check button changes
            value. */
class ParamBoolCheckButton : public Gtk::CheckButton {
private:
    /** \brief  Param to change */
    ParamBool * _pref;
public:
    /** \brief  Initialize the check button
        \param  param  Which parameter to adjust on changing the check button

        This function sets the value of the checkbox to be that of the
        parameter, and then sets up a callback to \c on_toggle.
    */
    ParamBoolCheckButton (ParamBool * param) :
            Gtk::CheckButton(), _pref(param) {
        this->set_active(_pref->get(NULL, NULL) /**\todo fix */);
        this->signal_toggled().connect(sigc::mem_fun(this, &ParamBoolCheckButton::on_toggle));
        return;
    }
    void on_toggle (void);
};

/**
    \brief  A function to respond to the check box changing

    Adjusts the value of the preference to match that in the check box.
*/
void
ParamBoolCheckButton::on_toggle (void)
{
    _pref->set(this->get_active(), NULL /**\todo fix this */, NULL);
    return;
}

/**
    \brief  Creates a bool check button for a bool parameter

    Builds a hbox with a label and a check button in it.
*/
Gtk::Widget *
ParamBool::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamBoolCheckButton * checkbox = new ParamBoolCheckButton(this);
    checkbox->show();
    hbox->pack_start(*checkbox, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  A special category of Gtk::Entry to handle string parameteres */
class ParamStringEntry : public Gtk::Entry {
private:
    ParamString * _pref;
public:
    /** \brief  Build a string preference for the given parameter
        \param  pref  Where to get the string from, and where to put it
                      when it changes.
    */
    ParamStringEntry (ParamString * pref) :
        Gtk::Entry(), _pref(pref) {
        if (_pref->get(NULL, NULL) != NULL)
            this->set_text(Glib::ustring(_pref->get(NULL, NULL)));
        this->signal_changed().connect(sigc::mem_fun(this, &ParamStringEntry::changed_text));
    };
    void changed_text (void);
};

/** \brief  Respond to the text box changing

    This function responds to the box changing by grabbing the value
    from the text box and putting it in the parameter.
*/
void
ParamStringEntry::changed_text (void)
{
    Glib::ustring data = this->get_text();
    _pref->set(data.c_str(), NULL, NULL);
    return;
}

/**
    \brief  Creates a text box for the string parameter

    Builds a hbox with a label and a text box in it.
*/
Gtk::Widget *
ParamString::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamStringEntry * textbox = new ParamStringEntry(this);
    textbox->show();
    hbox->pack_start(*textbox, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  Return 'true' or 'false' */
Glib::ustring *
ParamBool::string (void)
{
    Glib::ustring * mystring;

    if (_value)
        mystring = new Glib::ustring("true");
    else
        mystring = new Glib::ustring("false");

    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamInt::string (void)
{
    char startstring[32];
    sprintf(startstring, "%d", _value);
    Glib::ustring * mystring = new Glib::ustring(startstring);
    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamFloat::string (void)
{
    char startstring[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(startstring, G_ASCII_DTOSTR_BUF_SIZE, _value);
    Glib::ustring * mystring = new Glib::ustring(startstring);
    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamString::string (void)
{
    Glib::ustring * mystring = new Glib::ustring("");
    *mystring += "\"";
    *mystring += _value;
    *mystring += "\"";
    return mystring;
}


}  /* namespace Extension */
}  /* namespace Inkscape */

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
