#ifndef WIDGET_H_
#define WIDGET_H_

/***[Includes]****************************************************************/
#include <ncurses.h>

/***[Macros]******************************************************************/

/***[Types]*******************************************************************/
/// @brief Widgets are defined by a list of common attributes in their
/// "base", and a list of custom attributes in their "data".
typedef struct widget_t widget_t;

/// @brief Common attributes to all widgets.
typedef struct widget_base_t widget_base_t;

/// @brief Return values for widget functions that may fail.
typedef enum widget_status_t {
    WIDGET_OK = 0,
    WIDGET_NULL = -1,
    WIDGET_WRONG_TYPE = -2,
    WIDGET_ENOMEM = -3,
    WIDGET_OUT_OF_BOUNDS = -4,
    WIDGET_NOT_STDSCR = -5,
} widget_status_t;

typedef widget_status_t (*widget_del_fn_t)(widget_t *);
typedef bool (*widget_on_refresh_fn_t)(widget_t *);
typedef bool (*widget_on_focus_fn_t)(widget_t *, int);
typedef void (*widget_on_lose_focus_fn_t)(widget_t *);
typedef void (*widget_on_resize_t)(widget_t *, int, int, int, int);

/// @brief Widget types
typedef enum widget_type_t {
    WIDGET_TEXTBOX,
    WIDGET_BUTTON,
    WIDGET_LAYOUT,
    WIDGET_BUTTON_PANEL,
} widget_type_t;

typedef struct widget_border_t {
    chtype ls;
    chtype rs;
    chtype ts;
    chtype bs;
    chtype tl;
    chtype tr;
    chtype bl;
    chtype br;
} widget_border_t;

struct widget_base_t {
    widget_type_t type;
    bool dirty;
    WINDOW *window;
    widget_on_refresh_fn_t on_refresh_fn;
    widget_del_fn_t del_fn;
    widget_on_focus_fn_t on_focus_fn;
    widget_on_lose_focus_fn_t on_lose_focus_fn;
    widget_on_resize_t on_resize_fn;
};

struct widget_t {
    widget_base_t base;
    void *data;
};

/***[Extern variables]********************************************************/

/***[Functions prototypes]****************************************************/

void widget_init(widget_t *widget, void *data, widget_type_t type,
                 WINDOW *window, widget_on_refresh_fn_t on_refresh_fn,
                 widget_del_fn_t del_fn, widget_on_focus_fn_t on_focus_fn,
                 widget_on_lose_focus_fn_t on_lose_focus_fn,
                 widget_on_resize_t on_resize_fn);

__attribute__((warn_unused_result)) widget_status_t
widget_cast(widget_t *widget, void **data, widget_type_t type);

#endif // WIDGET_H
