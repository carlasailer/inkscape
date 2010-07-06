/** @file
 * @brief Minimal LPE effect, see lpe-skeleton.cpp.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_TESTCOPY_H
#define INKSCAPE_LPE_TESTCOPY_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include "live_effects/lpegroupbbox.h"


namespace Inkscape {
namespace LivePathEffect {

// each knotholder handle for your LPE requires a separate class derived from KnotHolderEntity;
// define it in lpe-skeleton.cpp and register it in the effect's constructor
/**
namespace Skeleton {
  // we need a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityMyHandle;
}
**/

class LPETestcopy : public Effect {
public:
    LPETestcopy(LivePathEffectObject *lpeobject);
    virtual ~LPETestcopy();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    virtual void doEffect (SPCurve * curve);
//    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /* the knotholder entity classes (if any) must be declared friends */
    //friend class Skeleton::KnotHolderEntityMyHandle;

private:
    // add the parameters for your effect here:
    ScalarParam number;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPETestcopy(const LPETestcopy&);
    LPETestcopy& operator=(const LPETestcopy&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

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
