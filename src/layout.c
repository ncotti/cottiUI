/***[Includes]****************************************************************/
#include "layout.h"
#include "color.h"
#include "logger.h"
#include "textbox.h"
#include "widget.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>

/***[Static variables]********************************************************/

/***[Static functions prototypes]*********************************************/

/**
 * @brief Returns the index used to access a widget located at a certain
 * row and column in a layout.
 */
static uint8_t pos2idx(layout_t *layout, uint8_t row, uint8_t col);

/**
 * @brief Initializes ncurses TUI.
 * @return stdscr.
 */
static WINDOW *ncurses_init(void);

/**
 * @brief Called whenever a resize event is triggered. A resized widget always
 * flags itself as dirty, i.e., that it should be refreshed, but it does not
 * trigger a refresh event.
 * The layout will re-calculate the new dimensions and positions for each widget
 * and propagate the resize event to all its children.
 * @param widget Widget to be resized.
 * @param height Widget's new height.
 * @param width Widget's new width.
 * @param y Widget's new beginning "y" position.
 * @param x Widget's new beginning "x" position.
 * @param pos Widget's new position (y,x)
 */
static void layout_on_resize(widget_t *widget, int height, int width, int y,
                             int x);

/**
 * @brief Called whenever a refresh event is triggered. The refresh will only
 * happen if the "dirty" flag is set to true. Widgets should call
 * "wnoutrefresh()" and wait for the layout to call "doupdate()", instead
 * of refreshing themselves with "wrefresh()".
 * The layout will propagate the refresh event to all children widgets, refresh
 * itself, and then call "doupdate()" to show all changes in the screen at once.
 * @param widget Widget to be refreshed.
 * @return "true" if the widget was refreshed, "false" if it has not been
 * changed.
 */
static bool layout_on_refresh(widget_t *widget);

/**
 * @brief Called whenever an input key is pressed and the widget is focused.
 * Layouts have "low priority", i.e. they will always propagate the
 * pressed key to their focused children, and will only try to consume the key
 * if their children did not use it first.
 * @param widget Focused widget.
 * @param key Input key pressed by the user, usually returned by "getch()".
 * @return "true" if the widget used the key, false otherwise.
 */
static bool layout_on_focus(widget_t *widget, int key);

/***[Static functions]********************************************************/

static uint8_t pos2idx(layout_t *layout, uint8_t row, uint8_t col) {
    return row * layout->cols + col;
}

static WINDOW *ncurses_init(void) {
    WINDOW *parent = initscr();

    noecho();
    cbreak();
    keypad(stdscr, true);
    color_init();

    return parent;
}

static void layout_on_resize(widget_t *widget, int height, int width, int y,
                             int x) {
    uint8_t idx = 0;
    layout_t *layout = (layout_t *)widget->data;

    const int widget_height = height / layout->rows;
    const int widget_width = width / layout->cols;
    int widget_y = 0;
    int widget_x = 0;

    // Whenever you resize the window, a call to refresh() is needed to
    // sync the virtual/physical model. Otherwise, the screen may be blank
    // until updated a second time.
    if (widget->base.window == stdscr) {
        refresh();
    }

    for (uint8_t row = 0; row < layout->rows; row++) {
        for (uint8_t col = 0; col < layout->cols; col++) {
            widget_y = y + row * widget_height;
            widget_x = x + col * widget_width;

            idx = pos2idx(layout, row, col);
            layout->widgets[idx]->base.on_resize_fn(layout->widgets[idx],
                                                    widget_height, widget_width,
                                                    widget_y, widget_x);
        }
    }

    widget->base.dirty = true;
}

static bool layout_on_refresh(widget_t *widget) {
    uint8_t idx = 0;
    layout_t *layout = (layout_t *)widget->data;
    bool refreshed = false;

    if (widget->base.dirty) {
        // TODO clear or erase?
        // wclear(widget->base.window);
    }

    for (uint8_t row = 0; row < layout->rows; row++) {
        for (uint8_t col = 0; col < layout->cols; col++) {
            idx = pos2idx(layout, row, col);
            refreshed |=
                layout->widgets[idx]->base.on_refresh_fn(layout->widgets[idx]);
        }
    }

    if (widget->base.dirty) {
        // TODO, redraw whatever needs to be redrawn.
        wnoutrefresh(widget->base.window);
        refreshed = true;
    }

    if (refreshed) {
        doupdate();
    }

    widget->base.dirty = false;
    return refreshed;
}

static bool layout_on_focus(widget_t *widget, int key) {
    layout_t *layout = (layout_t *)widget->data;
    bool key_was_used = layout->widgets[layout->focus_idx]->base.on_focus_fn(
        layout->widgets[layout->focus_idx], key);

    if (key_was_used) {
        return key_was_used;
    }

    switch (key) {
    case KEY_LEFT: {
        key_was_used = layout_change_focus(widget, LAYOUT_LEFT);
        break;
    }

    case KEY_RIGHT: {
        key_was_used = layout_change_focus(widget, LAYOUT_RIGHT);
        break;
    }

    case KEY_UP: {
        key_was_used = layout_change_focus(widget, LAYOUT_UP);
        break;
    }

    case KEY_DOWN: {
        key_was_used = layout_change_focus(widget, LAYOUT_DOWN);
        break;
    }

    case KEY_RESIZE: {
        int height, width, y, x;
        getmaxyx(widget->base.window, height, width);
        getbegyx(widget->base.window, y, x);
        layout_on_resize(widget, height, width, y, x);
    }

    default: {
        key_was_used = false;
        break;
    }
    }

    return key_was_used;
}

/***[Public functions]********************************************************/
widget_t *layout_new(WINDOW *parent, uint8_t rows, uint8_t cols) {

    widget_t *widget = (widget_t *)malloc(sizeof(widget_t));
    layout_t *layout = (layout_t *)malloc(sizeof(layout_t));
    layout->widgets =
        (widget_t **)malloc((size_t)rows * cols * sizeof(widget_t *));

    if ((widget == NULL) || (layout == NULL) || (layout->widgets == NULL)) {
        free((void *)layout->widgets);
        free((void *)layout);
        free((void *)widget);
        return NULL;
    }

    if (parent == NULL) {
        parent = ncurses_init();
    }

    widget_init(widget, (void *)layout, WIDGET_LAYOUT, parent,
                layout_on_refresh, layout_del, layout_on_focus,
                layout_on_lose_focus, layout_on_resize);

    layout->rows = rows;
    layout->cols = cols;
    layout->focus_row = 0;
    layout->focus_col = 0;
    layout->focus_idx = 0;

    for (uint8_t idx = 0; idx < (uint8_t)(rows * cols); idx++) {
        layout->widgets[idx] = NULL;
    }

    return widget;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
widget_status_t layout_add(widget_t *parent, widget_t *child, uint8_t row,
                           uint8_t col) {
    layout_t *layout = NULL;
    widget_status_t ret = widget_cast(parent, (void **)&layout, WIDGET_LAYOUT);
    if (ret != WIDGET_OK) {
        return ret;
    }

    if (row >= layout->rows || col >= layout->cols) {
        return WIDGET_OUT_OF_BOUNDS;
    }

    if (child == NULL) {
        return WIDGET_NULL;
    }

    layout_rm(parent, row, col);

    uint8_t idx = pos2idx(layout, row, col);
    layout->widgets[idx] = child;

    return WIDGET_OK;
}

widget_status_t layout_rm(widget_t *widget, uint8_t row, uint8_t col) {
    layout_t *layout = NULL;
    widget_status_t ret = widget_cast(widget, (void **)&layout, WIDGET_LAYOUT);
    if (ret != WIDGET_OK) {
        return ret;
    }

    uint8_t idx = pos2idx(layout, row, col);

    if (layout->widgets[idx] != NULL) {
        layout->widgets[idx]->base.del_fn(layout->widgets[idx]);
        layout->widgets[idx] = NULL;
    }

    return WIDGET_OK;
}

widget_status_t layout_del(widget_t *widget) {
    layout_t *layout = NULL;
    widget_status_t ret = widget_cast(widget, (void **)&layout, WIDGET_LAYOUT);
    if (ret != WIDGET_OK) {
        return ret;
    }

    for (uint8_t row = 0; row < layout->rows; row++) {
        for (uint8_t col = 0; col < layout->cols; col++) {
            layout_rm(widget, row, col);
        }
    }

    if (widget->base.window == stdscr) {
        endwin();
    }

    free((void *)layout->widgets);
    free((void *)layout);
    free((void *)widget);

    return WIDGET_OK;
}

widget_status_t layout_show(widget_t *widget) {
    bool key_was_used = true;
    int key = 0;
    int height, width, y, x;

    layout_t *layout = NULL;
    widget_status_t ret = widget_cast(widget, (void **)&layout, WIDGET_LAYOUT);
    if (ret != WIDGET_OK) {
        return ret;
    }

    if (widget->base.window != stdscr) {
        return WIDGET_NOT_STDSCR;
    }

    getbegyx(widget->base.window, y, x);
    getmaxyx(widget->base.window, height, width);
    layout_on_resize(widget, height, width, y, x);
    layout_on_focus(widget, 0);
    layout_on_refresh(widget);

    do {
        key = getch();

        key_was_used = layout_on_focus(widget, key);

        layout_on_refresh(widget);
    } while (key_was_used || (key != 'q'));

    return WIDGET_OK;
}

// TODO layout_resize (change amount of rows and cols)

// TODO layout do not lose focus for now
void layout_on_lose_focus(widget_t *widget) {
    layout_t *layout = (layout_t *)widget->data;

    layout->widgets[layout->focus_idx]->base.on_lose_focus_fn(
        layout->widgets[layout->focus_idx]);
}

bool layout_change_focus(widget_t *widget, layout_dir_t dir) {

    bool lost_focus = false;
    layout_t *layout = (layout_t *)widget->data;

    switch (dir) {
    case LAYOUT_UP: {
        if (layout->focus_row > 0) {
            layout->focus_row--;
            lost_focus = true;
        }
        break;
    }

    case LAYOUT_DOWN: {
        if (layout->focus_row < (layout->rows - 1)) {
            layout->focus_row++;
            lost_focus = true;
        }
        break;
    }

    case LAYOUT_LEFT: {
        if (layout->focus_col > 0) {
            layout->focus_col--;
            lost_focus = true;
        }
        break;
    }

    case LAYOUT_RIGHT: {
        if (layout->focus_col < (layout->cols - 1)) {
            layout->focus_col++;
            lost_focus = true;
        }

        break;
    }
    }

    // When a focus change happens, you need to call the "on_lose_focus()" of
    // the previous widget, and the "on_focus()" of the new one with a dummy
    // '\0' key, just in case it flags itself dirty and needs refreshing.
    if (lost_focus) {
        layout->widgets[layout->focus_idx]->base.on_lose_focus_fn(
            layout->widgets[layout->focus_idx]);
        layout->focus_idx =
            pos2idx(layout, layout->focus_row, layout->focus_col);
        layout->widgets[layout->focus_idx]->base.on_focus_fn(
            layout->widgets[layout->focus_idx], '\0');
    }

    return lost_focus;
}
