#ifndef SP_FEDIFFUSELIGHTING_H_SEEN
#define SP_FEDIFFUSELIGHTING_H_SEEN

/** \file
 * SVG <feDiffuseLighting> implementation, see sp-feDiffuseLighting.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *               2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "sp-fediffuselighting-fns.h"

namespace NR {
class FilterDiffuseLighting;
}

/* FeDiffuseLighting base class */
class SPFeDiffuseLightingClass;

struct SPFeDiffuseLighting : public SPFilterPrimitive {
    /** DIFFUSELIGHTING ATTRIBUTES HERE */
    gfloat surfaceScale;
    gfloat diffuseConstant;
    NumberOptNumber kernelUnitLength;
    guint32 lighting_color;

    NR::FilterDiffuseLighting *renderer;
};

struct SPFeDiffuseLightingClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feDiffuseLighting_get_type();


#endif /* !SP_FEDIFFUSELIGHTING_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
