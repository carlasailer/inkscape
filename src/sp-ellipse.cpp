#define __SP_ELLIPSE_C__

/*
 * SVG <ellipse> and related implementations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <math.h>
#include <string.h>

#include "libnr/nr-matrix.h"
#include "svg/svg.h"
#include "attributes.h"
#include "style.h"
#include "version.h"

#include "sp-ellipse.h"

/* Common parent class */

#define noELLIPSE_VERBOSE

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SP_2PI (2 * M_PI)

#if 1
/* Hmmm... shouldn't this also qualify */
/* Whether it is faster or not, well, nobody knows */
#define sp_round(v,m) (((v) < 0.0) ? ((ceil ((v) / (m) - 0.5)) * (m)) : ((floor ((v) / (m) + 0.5)) * (m)))
#else
/* we do not use C99 round(3) function yet */
static double sp_round (double x, double y)
{
	double remain;

	g_assert (y > 0.0);

	/* return round(x/y) * y; */

	remain = fmod(x, y);

	if (remain >= 0.5*y)
		return x - remain + y;
	else
		return x - remain;
}
#endif

static void sp_genericellipse_class_init (SPGenericEllipseClass *klass);
static void sp_genericellipse_init (SPGenericEllipse *ellipse);

static void sp_genericellipse_update (SPObject *object, SPCtx *ctx, guint flags);

static int sp_genericellipse_snappoints(SPItem *item, NR::Point p[], int size);

static void sp_genericellipse_set_shape (SPShape *shape);
static SPRepr *sp_genericellipse_write (SPObject *object, SPRepr *repr, guint flags);

static gboolean sp_arc_set_elliptical_path_attribute (SPArc *arc, SPRepr *repr);

static SPShapeClass *ge_parent_class;

GType
sp_genericellipse_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPGenericEllipseClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_genericellipse_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPGenericEllipse),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_genericellipse_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_SHAPE, "SPGenericEllipse", &info, (GTypeFlags)0);
	}
	return type;
}

static void sp_genericellipse_class_init(SPGenericEllipseClass *klass)
{
	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	SPItemClass *item_class = (SPItemClass *) klass;
	SPShapeClass *shape_class = (SPShapeClass *) klass;

	ge_parent_class = (SPShapeClass*) g_type_class_ref(SP_TYPE_SHAPE);

	sp_object_class->update = sp_genericellipse_update;
	sp_object_class->write = sp_genericellipse_write;

	item_class->snappoints = sp_genericellipse_snappoints;

	shape_class->set_shape = sp_genericellipse_set_shape;
}

static void
sp_genericellipse_init (SPGenericEllipse *ellipse)
{
	sp_svg_length_unset (&ellipse->cx, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&ellipse->cy, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&ellipse->rx, SP_SVG_UNIT_NONE, 0.0, 0.0);
	sp_svg_length_unset (&ellipse->ry, SP_SVG_UNIT_NONE, 0.0, 0.0);

	ellipse->start = 0.0;
	ellipse->end = SP_2PI;
	ellipse->closed = TRUE;
}

static void
sp_genericellipse_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		SPGenericEllipse *ellipse;
		SPStyle *style;
		double d;
		ellipse = (SPGenericEllipse *) object;
		style = object->style;
		d = 1.0 / NR_MATRIX_DF_EXPANSION (&((SPItemCtx *) ctx)->i2vp);
		sp_svg_length_update (&ellipse->cx, style->font_size.computed, style->font_size.computed * 0.5, d);
		sp_svg_length_update (&ellipse->cy, style->font_size.computed, style->font_size.computed * 0.5, d);
		sp_svg_length_update (&ellipse->rx, style->font_size.computed, style->font_size.computed * 0.5, d);
		sp_svg_length_update (&ellipse->ry, style->font_size.computed, style->font_size.computed * 0.5, d);
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) ge_parent_class)->update)
		((SPObjectClass *) ge_parent_class)->update (object, ctx, flags);
}

#define C1 0.552

/* fixme: Think (Lauris) */

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_bpath.h>

static void sp_genericellipse_set_shape(SPShape *shape)
{
	double cx, cy, rx, ry, s, e;
	double x0, y0, x1, y1, x2, y2, x3, y3;
	double len;
	double affine[6];
	gint slice = FALSE;
	gint i;

	SPGenericEllipse *ellipse = (SPGenericEllipse *) shape;

	if ((ellipse->rx.computed < 1e-18) || (ellipse->ry.computed < 1e-18)) return;
	if (fabs (ellipse->end - ellipse->start) < 1e-9) return;

	sp_genericellipse_normalize (ellipse);

	cx = 0.0;
	cy = 0.0;
	rx = ellipse->rx.computed;
	ry = ellipse->ry.computed;

	// figure out if we have a slice, guarding against rounding errors
	len = fmod (ellipse->end - ellipse->start, SP_2PI);
	if (len < 0.0) len += SP_2PI;
	if (fabs (len) < 1e-8 || fabs (len - SP_2PI) < 1e-8) {
		slice = FALSE;
		ellipse->end = ellipse->start + SP_2PI;
	} else {
		slice = TRUE;
	}

	nr_matrix_set_scale (NR_MATRIX_D_FROM_DOUBLE (affine), rx, ry);
	affine[4] = ellipse->cx.computed;
	affine[5] = ellipse->cy.computed;

	ArtBpath bpath[16];
	i = 0;
	if (ellipse->closed) {
		bpath[i].code = ART_MOVETO;
	} else {
		bpath[i].code = ART_MOVETO_OPEN;
	}
	bpath[i].x3 = cos (ellipse->start);
	bpath[i].y3 = sin (ellipse->start);
	i++;

	for (s = ellipse->start; s < ellipse->end; s += M_PI_2) {
		e = s + M_PI_2;
		if (e > ellipse->end)
			e = ellipse->end;
		len = C1 * (e - s) / M_PI_2;
		x0 = cos (s);
		y0 = sin (s);
		x1 = x0 + len * cos (s + M_PI_2);
		y1 = y0 + len * sin (s + M_PI_2);
		x3 = cos (e);
		y3 = sin (e);
		x2 = x3 + len * cos (e - M_PI_2);
		y2 = y3 + len * sin (e - M_PI_2);
#ifdef ELLIPSE_VERBOSE
g_print ("step %d s %f e %f coords %f %f %f %f %f %f\n",
	i, s, e, x1, y1, x2, y2, x3, y3);
#endif
		bpath[i].code = ART_CURVETO;
		bpath[i].x1 = x1;
		bpath[i].y1 = y1;
		bpath[i].x2 = x2;
		bpath[i].y2 = y2;
		bpath[i].x3 = x3;
		bpath[i].y3 = y3;
		i++;
	}

	if (slice && ellipse->closed) {
		bpath[i].code = ART_LINETO;
		bpath[i].x3 = 0.0;
		bpath[i].y3 = 0.0;
		i++;
		bpath[i].code = ART_LINETO;
		bpath[i].x3 = bpath[0].x3;
		bpath[i].y3 = bpath[0].y3;
		i++;
	} else if (ellipse->closed) {
		bpath[i-1].x3 = bpath[0].x3;
		bpath[i-1].y3 = bpath[0].y3;
	}

	bpath[i].code = ART_END;

	ArtBpath *abp = art_bpath_affine_transform(bpath, affine);
	SPCurve *c = sp_curve_new_from_bpath(abp);
	sp_shape_set_curve_insync ((SPShape *) ellipse, c, TRUE);
	sp_curve_unref (c);
}

static int sp_genericellipse_snappoints(SPItem *item, NR::Point p[], int size)
{
	SPGenericEllipse *ge = SP_GENERICELLIPSE (item);

	/* We use corners of item and center of ellipse. */
	int pos = 0;
	if (((SPItemClass *) ge_parent_class)->snappoints) {
		pos = ((SPItemClass *) ge_parent_class)->snappoints (item, p, size);
	}
	if (pos < size) {
		p[pos++] = ( NR::Point(ge->cx.computed,
				       ge->cy.computed)
			     * sp_item_i2d_affine(item) );
	}

	return pos;
}

void
sp_genericellipse_normalize (SPGenericEllipse *ellipse)
{
	ellipse->start = fmod (ellipse->start, SP_2PI);
	ellipse->end = fmod (ellipse->end, SP_2PI);

	if (ellipse->start < 0.0)
		ellipse->start += SP_2PI;
	double diff = ellipse->start - ellipse->end;
	if (diff >= 0.0)
		ellipse->end += diff - fmod(diff, SP_2PI) + SP_2PI;

	/* Now we keep: 0 <= start < end <= 2*PI */
}

static SPRepr *sp_genericellipse_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPGenericEllipse *ellipse = SP_GENERICELLIPSE (object);

	if (flags & SP_OBJECT_WRITE_EXT) {
		if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
			repr = sp_repr_new ("path");
		}

		sp_repr_set_double_attribute (repr, "sodipodi:cx", ellipse->cx.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:cy", ellipse->cy.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:rx", ellipse->rx.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:ry", ellipse->ry.computed);

		sp_arc_set_elliptical_path_attribute (SP_ARC (object), SP_OBJECT_REPR (object));
	}

	if (((SPObjectClass *) ge_parent_class)->write)
		((SPObjectClass *) ge_parent_class)->write (object, repr, flags);

	return repr;
}

/* SVG <ellipse> element */

static void sp_ellipse_class_init (SPEllipseClass *klass);
static void sp_ellipse_init (SPEllipse *ellipse);

static void sp_ellipse_build(SPObject *object, SPDocument *document, SPRepr *repr);
static SPRepr *sp_ellipse_write(SPObject *object, SPRepr *repr, guint flags);
static void sp_ellipse_set(SPObject *object, unsigned int key, gchar const *value);
static gchar *sp_ellipse_description(SPItem *item);

static SPGenericEllipseClass *ellipse_parent_class;

GType
sp_ellipse_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPEllipseClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_ellipse_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPEllipse),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_ellipse_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_GENERICELLIPSE, "SPEllipse", &info, (GTypeFlags)0);
	}
	return type;
}

static void sp_ellipse_class_init(SPEllipseClass *klass)
{
	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	SPItemClass *item_class = (SPItemClass *) klass;

	ellipse_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

	sp_object_class->build = sp_ellipse_build;
	sp_object_class->write = sp_ellipse_write;
	sp_object_class->set = sp_ellipse_set;

	item_class->description = sp_ellipse_description;
}

static void
sp_ellipse_init (SPEllipse *ellipse)
{
	/* Nothing special */
}

static void
sp_ellipse_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) ellipse_parent_class)->build)
		(* ((SPObjectClass *) ellipse_parent_class)->build) (object, document, repr);

	sp_object_read_attr (object, "cx");
	sp_object_read_attr (object, "cy");
	sp_object_read_attr (object, "rx");
	sp_object_read_attr (object, "ry");
}

static SPRepr *
sp_ellipse_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPGenericEllipse *ellipse;

	ellipse = SP_GENERICELLIPSE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("ellipse");
	}

	sp_repr_set_double_attribute (repr, "cx", ellipse->cx.computed);
	sp_repr_set_double_attribute (repr, "cy", ellipse->cy.computed);
	sp_repr_set_double_attribute (repr, "rx", ellipse->rx.computed);
	sp_repr_set_double_attribute (repr, "ry", ellipse->ry.computed);

	sp_arc_set_elliptical_path_attribute (SP_ARC (object), repr);
	
	if (((SPObjectClass *) ellipse_parent_class)->write)
		(* ((SPObjectClass *) ellipse_parent_class)->write) (object, repr, flags);

	return repr;
}

static void
sp_ellipse_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPGenericEllipse *ellipse;

	ellipse = SP_GENERICELLIPSE (object);

	switch (key) {
	case SP_ATTR_CX:
		if (!sp_svg_length_read (value, &ellipse->cx)) {
			sp_svg_length_unset (&ellipse->cx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_CY:
		if (!sp_svg_length_read (value, &ellipse->cy)) {
			sp_svg_length_unset (&ellipse->cy, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_RX:
		if (!sp_svg_length_read (value, &ellipse->rx) || (ellipse->rx.value <= 0.0)) {
			sp_svg_length_unset (&ellipse->rx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_RY:
		if (!sp_svg_length_read (value, &ellipse->ry) || (ellipse->ry.value <= 0.0)) {
			sp_svg_length_unset (&ellipse->ry, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) ellipse_parent_class)->set)
			((SPObjectClass *) ellipse_parent_class)->set (object, key, value);
		break;
	}
}

static gchar *sp_ellipse_description(SPItem *item)
{
	return g_strdup("Ellipse");
}


void
sp_ellipse_position_set (SPEllipse *ellipse, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
	SPGenericEllipse *ge;

	g_return_if_fail (ellipse != NULL);
	g_return_if_fail (SP_IS_ELLIPSE (ellipse));

	ge = SP_GENERICELLIPSE (ellipse);

	ge->cx.computed = x;
	ge->cy.computed = y;
	ge->rx.computed = rx;
	ge->ry.computed = ry;

	sp_object_request_update ((SPObject *) ge, SP_OBJECT_MODIFIED_FLAG);
}

/* SVG <circle> element */

static void sp_circle_class_init (SPCircleClass *klass);
static void sp_circle_init (SPCircle *circle);

static void sp_circle_build(SPObject *object, SPDocument *document, SPRepr *repr);
static SPRepr *sp_circle_write(SPObject *object, SPRepr *repr, guint flags);
static void sp_circle_set(SPObject *object, unsigned int key, const gchar *value);
static gchar *sp_circle_description(SPItem *item);

static SPGenericEllipseClass *circle_parent_class;

GType
sp_circle_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPCircleClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_circle_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPCircle),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_circle_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_GENERICELLIPSE, "SPCircle", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_circle_class_init (SPCircleClass *klass)
{
	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	SPItemClass *item_class = (SPItemClass *) klass;

	circle_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

	sp_object_class->build = sp_circle_build;
	sp_object_class->write = sp_circle_write;
	sp_object_class->set = sp_circle_set;

	item_class->description = sp_circle_description;
}

static void
sp_circle_init (SPCircle *circle)
{
	/* Nothing special */
}

static void
sp_circle_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) circle_parent_class)->build)
		(* ((SPObjectClass *) circle_parent_class)->build) (object, document, repr);

	sp_object_read_attr (object, "cx");
	sp_object_read_attr (object, "cy");
	sp_object_read_attr (object, "r");
}

static SPRepr *
sp_circle_write (SPObject *object, SPRepr *repr, guint flags)
{
	SPGenericEllipse *ellipse;

	ellipse = SP_GENERICELLIPSE (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = sp_repr_new ("circle");
	}

	sp_repr_set_double_attribute (repr, "cx", ellipse->cx.computed);
	sp_repr_set_double_attribute (repr, "cy", ellipse->cy.computed);
	sp_repr_set_double_attribute (repr, "r", ellipse->rx.computed);
	
	if (((SPObjectClass *) circle_parent_class)->write)
		((SPObjectClass *) circle_parent_class)->write (object, repr, flags);

	return repr;
}

static void
sp_circle_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPGenericEllipse *ge;

	ge = SP_GENERICELLIPSE (object);

	switch (key) {
	case SP_ATTR_CX:
		if (!sp_svg_length_read (value, &ge->cx)) {
			sp_svg_length_unset (&ge->cx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_CY:
		if (!sp_svg_length_read (value, &ge->cy)) {
			sp_svg_length_unset (&ge->cy, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_R:
		if (!sp_svg_length_read (value, &ge->rx) || (ge->rx.value <= 0.0)) {
			sp_svg_length_unset (&ge->rx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		ge->ry = ge->rx;
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) circle_parent_class)->set)
			((SPObjectClass *) circle_parent_class)->set (object, key, value);
		break;
	}
}

static gchar *sp_circle_description (SPItem *item)
{
	return g_strdup("Circle");
}

/* <path sodipodi:type="arc"> element */

static void sp_arc_class_init (SPArcClass *klass);
static void sp_arc_init (SPArc *arc);

static void sp_arc_build(SPObject *object, SPDocument *document, SPRepr *repr);
static SPRepr *sp_arc_write(SPObject *object, SPRepr *repr, guint flags);
static void sp_arc_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_arc_modified(SPObject *object, guint flags);

static gchar *sp_arc_description(SPItem *item);

static SPGenericEllipseClass *arc_parent_class;

GType
sp_arc_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPArcClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_arc_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPArc),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_arc_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_GENERICELLIPSE, "SPArc", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_arc_class_init (SPArcClass *klass)
{
	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	SPItemClass *item_class = (SPItemClass *) klass;

	arc_parent_class = (SPGenericEllipseClass*) g_type_class_ref(SP_TYPE_GENERICELLIPSE);

	sp_object_class->build = sp_arc_build;
	sp_object_class->write = sp_arc_write;
	sp_object_class->set = sp_arc_set;
	sp_object_class->modified = sp_arc_modified;

	item_class->description = sp_arc_description;
}

static void
sp_arc_init (SPArc *arc)
{
	/* Nothing special */
}

static void
sp_arc_build (SPObject *object, SPDocument *document, SPRepr *repr)
{
	if (((SPObjectClass *) arc_parent_class)->build)
		(* ((SPObjectClass *) arc_parent_class)->build) (object, document, repr);

	SPVersion version = sp_object_get_sodipodi_version (object);

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		/* Old spec violating arc attributes */
		sp_object_read_attr (object, "cx");
		sp_object_read_attr (object, "cy");
		sp_object_read_attr (object, "rx");
		sp_object_read_attr (object, "ry");
	} else {
		/* New attributes */
		sp_object_read_attr (object, "sodipodi:cx");
		sp_object_read_attr (object, "sodipodi:cy");
		sp_object_read_attr (object, "sodipodi:rx");
		sp_object_read_attr (object, "sodipodi:ry");
	}

	sp_object_read_attr (object, "sodipodi:start");
	sp_object_read_attr (object, "sodipodi:end");
	sp_object_read_attr (object, "sodipodi:open");

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		/* fixme: I am 99.9% sure we can do this here safely, but check nevertheless (Lauris) */
		sp_arc_write (object, repr, SP_OBJECT_WRITE_EXT);
		sp_repr_set_attr (repr, "cx", NULL);
		sp_repr_set_attr (repr, "cy", NULL);
		sp_repr_set_attr (repr, "rx", NULL);
		sp_repr_set_attr (repr, "ry", NULL);
	}
}

/*
 * sp_arc_set_elliptical_path_attribute:
 *
 * Convert center to endpoint parameterization and set it to repr.
 *
 * See SVG 1.0 Specification W3C Recommendation
 * ``F.6 Ellptical arc implementation notes'' for more detail.
 */
static gboolean
sp_arc_set_elliptical_path_attribute (SPArc *arc, SPRepr *repr)
{
#define ARC_BUFSIZE 256
	gint fa, fs;
	gdouble  dt;
	gchar c[ARC_BUFSIZE];

	SPGenericEllipse *ge = SP_GENERICELLIPSE (arc);

	NRPoint p1 = sp_arc_get_xy (arc, ge->start);
	NRPoint p2 = sp_arc_get_xy (arc, ge->end);

	dt = fmod (ge->end - ge->start, SP_2PI);
	if (fabs (dt) < 1e-6) {
		NRPoint ph = sp_arc_get_xy (arc, (ge->start + ge->end) / 2.0);
		g_snprintf (c, ARC_BUFSIZE, "M %f %f A %f %f 0 %d %d %f,%f A %g %g 0 %d %d %g %g L %f %f z",
			    p1.x, p1.y,
			    ge->rx.computed, ge->ry.computed,
			    1, (dt > 0),
			    ph.x, ph.y,
			    ge->rx.computed, ge->ry.computed,
			    1, (dt > 0),
			    p2.x, p2.y,
			    ge->cx.computed, ge->cy.computed);
	} else {
		fa = (fabs (dt) > M_PI) ? 1 : 0;
		fs = (dt > 0) ? 1 : 0;
#ifdef ARC_VERBOSE
		g_print ("start:%g end:%g fa=%d fs=%d\n", ge->start, ge->end, fa, fs);
#endif
		if (ge->closed) {
			g_snprintf (c, ARC_BUFSIZE, "M %f,%f A %f,%f 0 %d %d %f,%f L %f,%f z",
				    p1.x, p1.y,
				    ge->rx.computed, ge->ry.computed,
				    fa, fs,
				    p2.x, p2.y,
				    ge->cx.computed, ge->cy.computed);
		} else {
			g_snprintf (c, ARC_BUFSIZE, "M %f,%f A %f,%f 0 %d %d %f,%f",
				    p1.x, p1.y,
				    ge->rx.computed, ge->ry.computed,
				    fa, fs, p2.x, p2.y);
		}
	}
	return sp_repr_set_attr (repr, "d", c);
}

static SPRepr *
sp_arc_write (SPObject *object, SPRepr *repr, guint flags)
{
	gdouble len;

	SPGenericEllipse *ge = SP_GENERICELLIPSE (object);
	SPArc *arc = SP_ARC (object);

	if (flags & SP_OBJECT_WRITE_EXT) {
		if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
			repr = sp_repr_new ("path");
		}
		sp_repr_set_attr (repr, "sodipodi:type", "arc");
		sp_repr_set_double_attribute (repr, "sodipodi:cx", ge->cx.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:cy", ge->cy.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:rx", ge->rx.computed);
		sp_repr_set_double_attribute (repr, "sodipodi:ry", ge->ry.computed);

		// write start and end only if they are non-trivial; otherwise remove
		len = fmod (ge->end - ge->start, SP_2PI);
		if (len < 0.0) len += SP_2PI;
		if (!(fabs (len) < 1e-8 || fabs (len - SP_2PI) < 1e-8)) {
			sp_repr_set_double_attribute (repr, "sodipodi:start", ge->start);
			sp_repr_set_double_attribute (repr, "sodipodi:end", ge->end);
			sp_repr_set_attr (repr, "sodipodi:open", (!ge->closed) ? "true" : NULL);
		} else {
			sp_repr_set_attr (repr, "sodipodi:end", NULL);
			sp_repr_set_attr (repr, "sodipodi:start", NULL);
			sp_repr_set_attr (repr, "sodipodi:open", NULL);
		}

		sp_arc_set_elliptical_path_attribute (arc, repr);

	} else {
		gdouble dt;
		dt = fmod (ge->end - ge->start, SP_2PI);
		if (fabs (dt) < 1e-6) {
			if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
				repr = sp_repr_new ("ellipse");
			}
			sp_repr_set_double_attribute (repr, "cx", ge->cx.computed);
			sp_repr_set_double_attribute (repr, "cy", ge->cy.computed);
			sp_repr_set_double_attribute (repr, "rx", ge->rx.computed);
			sp_repr_set_double_attribute (repr, "ry", ge->ry.computed);
		} else {
			if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
				repr = sp_repr_new ("path");
			}
			sp_arc_set_elliptical_path_attribute (arc, repr);
		}
	}

	if (((SPObjectClass *) arc_parent_class)->write)
		((SPObjectClass *) arc_parent_class)->write (object, repr, flags);

	return repr;
}

static void
sp_arc_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPGenericEllipse *ge = SP_GENERICELLIPSE (object);
	
	SPVersion version = sp_object_get_sodipodi_version (object);

	if (sp_version_inside_range (version, 0, 0, 0, 25)) {
		switch (key) {
		case SP_ATTR_CX:
			key = SP_ATTR_SODIPODI_CX;
			break;
		case SP_ATTR_CY:
			key = SP_ATTR_SODIPODI_CY;
			break;
		case SP_ATTR_RX:
			key = SP_ATTR_SODIPODI_RX;
			break;
		case SP_ATTR_RY:
			key = SP_ATTR_SODIPODI_RY;
			break;
		}
	}

	switch (key) {
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read (value, &ge->cx)) {
			sp_svg_length_unset (&ge->cx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read (value, &ge->cy)) {
			sp_svg_length_unset (&ge->cy, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_RX:
		if (!sp_svg_length_read (value, &ge->rx) || (ge->rx.computed <= 0.0)) {
			sp_svg_length_unset (&ge->rx, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_RY:
		if (!sp_svg_length_read (value, &ge->ry) || (ge->ry.computed <= 0.0)) {
			sp_svg_length_unset (&ge->ry, SP_SVG_UNIT_NONE, 0.0, 0.0);
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_START:
		if (value) {
			sp_svg_number_read_d (value, &ge->start);
		} else {
			ge->start = 0;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_END:
		if (value) {
			sp_svg_number_read_d (value, &ge->end);
		} else {
			ge->end = 2 * M_PI;
		}
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_OPEN:
		ge->closed = (!value);
		sp_object_request_update (object, SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) arc_parent_class)->set)
			((SPObjectClass *) arc_parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_arc_modified (SPObject *object, guint flags)
{
	if (flags & SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) arc_parent_class)->modified)
		((SPObjectClass *) arc_parent_class)->modified (object, flags);
}

static gchar *sp_arc_description(SPItem *item)
{
	return g_strdup("Arc");
}

void
sp_arc_position_set (SPArc *arc, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
	g_return_if_fail (arc != NULL);
	g_return_if_fail (SP_IS_ARC (arc));

	SPGenericEllipse *ge = SP_GENERICELLIPSE (arc);

	ge->cx.computed = x;
	ge->cy.computed = y;
	ge->rx.computed = rx;
	ge->ry.computed = ry;

	sp_object_request_update ((SPObject *) arc, SP_OBJECT_MODIFIED_FLAG);
}

NR::Point sp_arc_get_xy (SPArc *arc, gdouble arg)
{
	SPGenericEllipse *ge = SP_GENERICELLIPSE (arc);

	return NR::Point(ge->rx.computed * cos(arg) + ge->cx.computed,
			 ge->ry.computed * sin(arg) + ge->cy.computed);
}

