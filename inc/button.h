#ifndef BUTTON_H_
#define BUTTON_H_

/***[Includes]****************************************************************/
#include "color.h"
#include "widget.h"
#include <stdint.h>

/***[Macros]******************************************************************/
#define BUTTON_MAX_TEXT_LENGTH (uint16_t)32

/***[Types]*******************************************************************/

typedef void (*button_on_press_fn_t)(void *);

typedef struct button_t {
    color_t color;
    char text[BUTTON_MAX_TEXT_LENGTH];
    button_on_press_fn_t on_press_fn;
    int key; // Shortcut key that presses the button
    color_t border_color;
    color_t stored_border_color;
    void *params;
} button_t;

/***[Extern variables]********************************************************/

/***[Functions prototypes]****************************************************/

widget_t *button_new(button_on_press_fn_t on_press_fn, void *params,
                     const char *text, int key, color_t color);
widget_status_t button_del(widget_t *widget);
widget_status_t button_set_border_color(widget_t *widget, color_t color);

#endif // BUTTON_H_
