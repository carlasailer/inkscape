#ifndef __SP_SELECT_CONTEXT_H__
#define __SP_SELECT_CONTEXT_H__

/*
 * Select tool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include "selection-describer.h"
#include "seltrans.h"

#define SP_TYPE_SELECT_CONTEXT            (sp_select_context_get_type ())
#define SP_SELECT_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_SELECT_CONTEXT, SPSelectContext))
#define SP_SELECT_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_SELECT_CONTEXT, SPSelectContextClass))
#define SP_IS_SELECT_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_SELECT_CONTEXT))
#define SP_IS_SELECT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_SELECT_CONTEXT))

class SPSelectContext;
class SPSelectContextClass;

namespace Inkscape { class MessageContext; }

struct SPSelectContext : public SPEventContext {
	guint dragging : 1;
	guint moved : 1;
	guint button_press_shift : 1;
	guint button_press_ctrl : 1;
	SPItem *item;
	SPCanvasItem *grabbed;
	SPSelTrans *_seltrans;
	Inkscape::SelectionDescriber *_describer;
};

struct SPSelectContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_select_context_get_type (void);

#endif
