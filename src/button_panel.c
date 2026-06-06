/***[Includes]****************************************************************/
#include "button_panel.h"
#include "button.h"
#include "color.h"
#include "widget.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/***[Static variables]********************************************************/

/***[Static functions prototypes]*********************************************/

static bool button_panel_on_refresh(widget_t *widget);
static void button_panel_on_resize(widget_t *widget, int height, int width,
                                   int y, int x);
static bool button_panel_on_focus(widget_t *widget, int key);
static void button_panel_on_lose_focus(widget_t *widget);

static void button_redraw(button_t *button);

static void negotiate_position_and_dimension(widget_t *widget);

/***[Static functions]********************************************************/

static bool button_panel_on_refresh(widget_t *widget) {
    // Draw a small box around each button, and write its text.

    button_panel_t *panel = (button_panel_t *)widget->data;

    if (!widget->base.dirty) {
        return false;
    }

    wclear(widget->base.window);

    wattron(widget->base.window, COLOR_PAIR(panel->border_color));
    wborder(widget->base.window, 0, 0, 0, 0, 0, 0, 0, 0);
    wattroff(widget->base.window, COLOR_PAIR(panel->border_color));

    negotiate_position_and_dimension(widget);

    for (uint8_t i = 0; i < panel->button_qtty; i++) {
        // TODO redraw does nothing here
        button_redraw(panel->buttons[i]);
    }

    wnoutrefresh(widget->base.window);
    widget->base.dirty = false;
    return true;
}
static void button_panel_on_resize(widget_t *widget, int height, int width,
                                   int y, int x) {
    wresize(widget->base.window, height, width);
    mvwin(widget->base.window, y, x);
    widget->base.dirty = true;
}
static bool button_panel_on_focus(widget_t *widget, int key) {
    button_panel_t *panel = widget->data;
    bool key_was_used = false;

    // for (uint8_t i = 0; i < panel->button_qtty; i++) {
    //     if (key == panel->buttons[i]->key) {
    //         // TODO edge case: two buttons with the same key
    //         panel->buttons[i]->on_press_fn();
    //         key_was_used = true;
    //         break;
    //     }
    // }

    // TODO move between buttons with arrow keys, and be able to press them
    // spacebar or enter
    // if (!key_was_used) {
    //     switch (key) {
    //         case()
    //     }
    // }

    button_panel_set_border_color(widget, COLOR_RED_BLACK);
    return key_was_used;
}
static void button_panel_on_lose_focus(widget_t *widget) {
    button_panel_t *panel = widget->data;
    button_panel_set_border_color(widget, panel->stored_border_color);
}

static void button_redraw(button_t *button) {}

static void negotiate_position_and_dimension(widget_t *widget) {
    button_panel_t *panel = (button_panel_t *)widget->data;
}

/***[Public functions]********************************************************/

widget_t *button_panel_new(int height, int width, button_t *button) {
    widget_t *widget = (widget_t *)malloc(sizeof(widget_t));
    button_panel_t *button_panel =
        (button_panel_t *)malloc(sizeof(button_panel_t));

    if ((widget == NULL) || (button_panel == NULL)) {
        free((void *)button_panel);
        free((void *)widget);
        return NULL;
    }

    widget_init(widget, (void *)button_panel, WIDGET_BUTTON_PANEL,
                newwin(1, 1, 1, 1), button_panel_on_refresh, button_panel_del,
                button_panel_on_focus, button_panel_on_lose_focus,
                button_panel_on_resize);

    button_panel->border_color = COLOR_WHITE_BLACK;
    button_panel->buttons = NULL;
    button_panel->button_qtty = 0;
    button_panel->height = height;
    button_panel->width = width;
    widget_status_t ret = button_panel_add(widget, button);
    if (ret != WIDGET_OK) {
        button_panel_del(widget);
        return NULL;
    }

    return widget;
}

widget_status_t button_panel_add(widget_t *widget, button_t *button) {
    button_panel_t *panel = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&panel, WIDGET_BUTTON_PANEL);
    if (ret != WIDGET_OK) {
        return ret;
    }

    // TODO check for invalid keys, such as ESC or arrow keys

    button_t **tmp_ptr = (button_t **)realloc(
        (void *)panel->buttons, sizeof(button_t *) * (panel->button_qtty + 1));

    if (tmp_ptr == NULL) {
        return WIDGET_ENOMEM;
    }

    panel->buttons = tmp_ptr;

    panel->buttons[panel->button_qtty] = button;

    panel->button_qtty++;

    widget->base.dirty = true;
    return WIDGET_OK;
}

widget_status_t button_panel_set_border_color(widget_t *widget, color_t color) {
    button_panel_t *panel = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&panel, WIDGET_BUTTON_PANEL);

    if (ret != WIDGET_OK) {
        return ret;
    }

    if (panel->border_color != color) {
        panel->stored_border_color = panel->border_color;
        panel->border_color = color;
        widget->base.dirty = true;
    }
    return WIDGET_OK;
}

widget_status_t button_panel_del(widget_t *widget) {
    button_panel_t *button_panel = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&button_panel, WIDGET_BUTTON_PANEL);

    if (ret != WIDGET_OK) {
        return ret;
    }

    wclear(widget->base.window);
    delwin(widget->base.window);
    free((void *)button_panel->buttons);
    free((void *)button_panel);
    free((void *)widget);

    return WIDGET_OK;
}
