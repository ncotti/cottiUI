#ifndef LAYOUT_H_
#define LAYOUT_H_

/***[Includes]****************************************************************/

#include "widget.h"
#include <ncurses.h>
#include <stdint.h>

/***[Macros]******************************************************************/

/***[Types]*******************************************************************/

typedef enum layout_dir_t {
    LAYOUT_UP,
    LAYOUT_DOWN,
    LAYOUT_LEFT,
    LAYOUT_RIGHT,
} layout_dir_t;

typedef struct layout_t {
    uint8_t rows;
    uint8_t cols;
    widget_t **widgets;
    uint8_t focus_row;
    uint8_t focus_col;
    uint8_t focus_idx;
} layout_t;

/***[Extern variables]********************************************************/

/***[Functions prototypes]****************************************************/

/**
 * @brief Creates a new layout inside "parent" window.
 * A layout may hold any other widget, including other layouts, in a (rows X
 * cols) grid. Whenever an action is requested from the widget, e.g. refresh,
 * resize, delete, etc; it is passed along to all child widgets.
 * @param parent Parent window where the layout will be built.
 * If "parent" is NULL, then ncurses will be initialized and "stdscr" will be
 * used.
 * @param rows Number of rows. Height will be equally distributed between
 * widgets.
 * @param cols Number of columns. Width will be equally distributed between
 * widgets.
 * @return The newly created layout, or NULL in case of error.
 */
widget_t *layout_new(WINDOW *parent, uint8_t rows, uint8_t cols);

/**
 * @brief Add or replace a widget into an existing layout. If the location
 * is already occupied by another widget, it will be deleted by its
 * "widget_del()" function.
 * @param widget Layout where to place the widget.
 * @param new_widget Widget to be placed.
 * @param row Row position for the new widget.
 * @param col Column position for the new widget.
 */
widget_status_t layout_add(widget_t *parent, widget_t *child, uint8_t row,
                           uint8_t col);

/**
 * @brief Removes the widget in the designated row and column position. It
 * calls the widget's "widget_del()" function and frees up the memory pointer.
 * @param widget Layout widget.
 * @param row Row position.
 * @param col Column position.
 * @return WIDGET_OK on success.
 */
widget_status_t layout_rm(widget_t *widget, uint8_t row, uint8_t col);

/**
 * @brief Frees resources from layout. It will also call the "widget_del()"
 * function of any child widget.
 * @param widget Layout to be deleted. If the layout's parent window is
 * "stdscr", then ncurses will be uninitialized.
 * @return WIDGET_OK if the deletion was successful, or an error code.
 */
widget_status_t layout_del(widget_t *widget);

/// @brief Refresh all widgets inside the layout.
/// @param layout Layout to be refreshed.

/**
 * @brief Starts the top-level TUI application. It will continuously catch any
 * stdin inputs and do the following operations:
 *  - Pass down the input to the focused widget.
 *  - Process the input itself, if it was not already consumed by the focused
 * widget.
 *  - Refresh any widget marked as "dirty".
 * @param widget Layout widget, which must have been created with NULL as its
 * parent, i.e., using "stdscr".
 * @return WIDGET_OK, unless the widget does not have "stdscr" as its parent.
 */
widget_status_t layout_show(widget_t *widget);

bool layout_change_focus(widget_t *widget, layout_dir_t dir);

void layout_on_lose_focus(widget_t *widget);

#endif // LAYOUT_H_
