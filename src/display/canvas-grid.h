#ifndef SP_CANVAS_GRID_H
#define SP_CANVAS_GRID_H

/*
 * SPCGrid
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <display/sp-canvas.h>



#define SP_TYPE_CGRID            (sp_cgrid_get_type ())
#define SP_CGRID(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CGRID, SPCGrid))
#define SP_CGRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CGRID, SPCGridClass))
#define SP_IS_CGRID(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CGRID))
#define SP_IS_CGRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CGRID))


struct SPCGrid : public SPCanvasItem{
	NR::Point origin;
	NR::Point spacing;
	guint32 color;
	guint32 empcolor;
	gint empspacing;
	bool scaled[2];

	NR::Point ow, sw;
};

struct SPCGridClass {
	SPCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType sp_cgrid_get_type (void);



#endif
