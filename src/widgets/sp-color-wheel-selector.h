#ifndef __SP_COLOR_WHEEL_SELECTOR_H__
#define __SP_COLOR_WHEEL_SELECTOR_H__

#include <gtk/gtkvbox.h>
#include "../color.h"
#include "sp-color-wheel.h"
#include "sp-color-slider.h"
#include "sp-color-selector.h"

#include <glib.h>



typedef struct _SPColorWheelSelector SPColorWheelSelector;
typedef struct _SPColorWheelSelectorClass SPColorWheelSelectorClass;


class ColorWheelSelector: public ColorSelector
{
public:
    ColorWheelSelector( SPColorSelector* csel );
    virtual ~ColorWheelSelector();

    virtual void init();

protected:
    virtual void _colorChanged( const SPColor& color, gfloat alpha );

    static void _adjustmentChanged ( GtkAdjustment *adjustment, SPColorWheelSelector *cs );

    static void _sliderGrabbed( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _sliderReleased( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _sliderChanged( SPColorSlider *slider, SPColorWheelSelector *cs );
    static void _wheelChanged( SPColorWheel* wheel, SPColorWheelSelector *cs );

    static void _fooChanged( GtkWidget foo, SPColorWheelSelector *cs );

    void _recalcColor( gboolean changing );

    gboolean _updating : 1;
    gboolean _dragging : 1;
    GtkAdjustment* _adj; /* Channel adjustment */
    GtkWidget* _wheel;
    GtkWidget* _slider;
    GtkWidget* _sbtn; /* Spinbutton */
    GtkWidget* _label; /* Label */

private:
    // By default, disallow copy constructor and assignment operator
    ColorWheelSelector( const ColorWheelSelector& obj );
    ColorWheelSelector& operator=( const ColorWheelSelector& obj );
};



namespace Inkscape {
    namespace UI {
        namespace Widgets {
#define SP_TYPE_COLOR_WHEEL_SELECTOR (sp_color_wheel_selector_get_type ())
#define SP_COLOR_WHEEL_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_WHEEL_SELECTOR, SPColorWheelSelector))
#define SP_COLOR_WHEEL_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_WHEEL_SELECTOR, SPColorWheelSelectorClass))
#define SP_IS_COLOR_WHEEL_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_WHEEL_SELECTOR))
#define SP_IS_COLOR_WHEEL_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_WHEEL_SELECTOR))
        }
    }
}

struct _SPColorWheelSelector {
    SPColorSelector parent;
};

struct _SPColorWheelSelectorClass {
    SPColorSelectorClass parent_class;
};

GType sp_color_wheel_selector_get_type (void);

GtkWidget *sp_color_wheel_selector_new (void);



#endif
