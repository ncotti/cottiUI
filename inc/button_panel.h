#ifndef BUTTON_PANEL_H_
#define BUTTON_PANEL_H_

/***[Includes]****************************************************************/
#include "button.h"
#include "color.h"
#include "widget.h"
#include <ncurses.h>
#include <stdint.h>

/***[Macros]******************************************************************/

/***[Types]*******************************************************************/

typedef struct button_panel_t {
    button_t **buttons;
    uint8_t button_qtty;
    color_t border_color;
    color_t stored_border_color;
    int height;
    int width;
} button_panel_t;

/***[Extern variables]********************************************************/

/***[Functions prototypes]****************************************************/

widget_t *button_panel_new(int height, int width, button_t *button);

widget_status_t button_panel_del(widget_t *widget);

widget_status_t button_panel_add(widget_t *widget, button_t *button);

widget_status_t button_panel_set_border_color(widget_t *widget, color_t color);

#endif // BUTTON_PANEL_H_
