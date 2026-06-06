/***[Includes]****************************************************************/
#include "textbox.h"
#include "color.h"
#include "logger.h"
#include "widget.h"
#include <math.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/***[Static variables]********************************************************/

static const char g_null_char = '\0';

/***[Static functions prototypes]*********************************************/

static void textbox_on_resize(widget_t *widget, int height, int width, int y,
                              int x);
static bool textbox_on_refresh(widget_t *widget);
static bool textbox_on_focus(widget_t *widget, int key);
static void textbox_on_lose_focus(widget_t *widget);

static void calculate_rows_and_words(widget_t *widget);

static void draw_border(widget_t *widget);

/***[Static functions]********************************************************/

static void draw_border(widget_t *widget) {
    textbox_t *textbox = (textbox_t *)widget->data;

    const size_t word_counter_width =
        strlen("Words: ") + (size_t)ceilf(log10f(textbox->text_words));

    wattron(widget->base.window, COLOR_PAIR(textbox->border_color));
    wborder(widget->base.window, 0, 0, 0, 0, 0, 0, 0, 0);
    wattroff(widget->base.window, COLOR_PAIR(textbox->border_color));

    wattron(widget->base.window,
            COLOR_PAIR(COLOR_NEGATE(textbox->border_color)));

    if (textbox->row_counter_enabled) {
        mvwprintw(widget->base.window, getmaxy(widget->base.window) - 1, 1,
                  "Rows: %d", textbox->text_rows);
    }

    if (textbox->word_counter_enabled) {
        mvwprintw(widget->base.window, getmaxy(widget->base.window) - 1,
                  getmaxx(widget->base.window) - word_counter_width - 1,
                  "Words: %d", textbox->text_words);
    }

    wattroff(widget->base.window,
             COLOR_NEGATE(COLOR_PAIR(textbox->border_color)));
}

static void alignment_to_position(widget_t *widget) {
    textbox_t *textbox = (textbox_t *)widget->data;
    int ypos = 0;
    int xpos = 0;
    uint16_t line_counter = 0;
    uint16_t lines_written = 0;

    // Minus two because of the borders.
    const int max_width = getmaxx(widget->base.window) - 2;
    const int max_height = getmaxy(widget->base.window) - 2;

    // Vertical alignment
    switch (textbox->v_alignment) {
    case TEXT_V_ALIGN_TOP: {
        ypos = 1;
        break;
    }

    case TEXT_V_ALIGN_MIDDLE: {
        ypos = (getmaxy(widget->base.window) - textbox->text_rows) / 2;
        break;
    }

    case TEXT_V_ALIGN_BOTTOM: {
        ypos = getmaxy(widget->base.window) - 1 - textbox->text_rows;
        break;
    }
    }

    // TODO right line size
    char *line = (char *)malloc((size_t)max_width * 2U);
    line[0] = g_null_char;
    char *copy = strdup(textbox->text);

    char *word = strtok(copy, " \n\t");
    while (word && (ypos <= max_height)) {
        // Add words to the current line while its width does not surpasses the
        // window's width.
        if (strlen(line) + strlen(word) + 1 <= max_width) {
            // Add space between words
            if (strlen(line) > 0) {
                strncat(line, " ", 2);
            }
            strncat(line, word, strlen(word));
        } else {
            // Print and erase line

            // Horizontal alignment
            switch (textbox->h_alignment) {
            case TEXT_H_ALIGN_LEFT: {
                xpos = 1;
                break;
            }

            case TEXT_H_ALIGN_CENTER: {
                xpos = ((max_width - (int)strlen(line)) / 2) + 1;
                break;
            }

            case TEXT_H_ALIGN_RIGHT: {
                xpos = getmaxx(widget->base.window) - (int)strlen(line) - 1;
                break;
            }
            }

            if (line_counter >= textbox->first_row_shown) {
                wattron(widget->base.window, COLOR_PAIR(textbox->text_color));
                mvwprintw(widget->base.window, ypos, xpos, "%s", line);
                wattroff(widget->base.window, COLOR_PAIR(textbox->text_color));
                ypos++;
                lines_written++;
            }

            line[0] = g_null_char;
            line_counter++;
            strncat(line, word, strlen(word) + 1);
        }

        word = strtok(NULL, " \n\t");
    }

    if ((strlen(line) > 0) && (ypos <= max_height)) {
        // Print and erase line

        // Horizontal alignment
        switch (textbox->h_alignment) {
        case TEXT_H_ALIGN_LEFT: {
            xpos = 1;
            break;
        }

        case TEXT_H_ALIGN_CENTER: {
            xpos = ((max_width - (int)strlen(line)) / 2) + 1;
            break;
        }

        case TEXT_H_ALIGN_RIGHT: {
            xpos = getmaxx(widget->base.window) - (int)strlen(line) - 1;
            break;
        }
        }

        wattron(widget->base.window, COLOR_PAIR(textbox->text_color));
        mvwprintw(widget->base.window, ypos, xpos, "%s", line);
        wattroff(widget->base.window, COLOR_PAIR(textbox->text_color));
        lines_written++;
    }

    textbox->last_row_shown = textbox->first_row_shown + lines_written - 1;

    if (textbox->first_row_shown > 0) {
        wattron(widget->base.window, COLOR_PAIR(textbox->text_color));
        mvwprintw(widget->base.window, 0, max_width / 2 - 1, "%s",
                  ((max_width % 2) == 0) ? "^^^^" : "^^^^^");
        wattroff(widget->base.window, COLOR_PAIR(textbox->text_color));
    }
    if (textbox->last_row_shown < textbox->text_rows - 1) {
        mvwprintw(widget->base.window, getmaxy(widget->base.window) - 1,
                  max_width / 2 - 1, "%s",
                  ((max_width % 2) == 0) ? "vvvv" : "vvvvv");
    }

    free(line);
    free(copy);
}

static void calculate_rows_and_words(widget_t *widget) {
    textbox_t *textbox = (textbox_t *)widget->data;

    // Minus one because of the left border.
    // If the last character is a space or newline, we can afford not to
    // print it, that's why the right border is ignored.
    const uint16_t max_width = getmaxx(widget->base.window) - 1;
    uint16_t chars_in_line = 0;
    uint16_t chars_in_word = 0;

    textbox->text_rows = 0;
    textbox->text_words = 0;

    // The white-space is counted as a character that prefixes words.
    // The "+1" is to count the "'\0'" as if it were a white-space.
    // It is considered an skippable character, such as " " or "\n".
    for (size_t i = 0; i < strlen(textbox->text) + 1; i++) {
        chars_in_word++;
        if ((textbox->text[i] == ' ') || (textbox->text[i] == '\n') ||
            (textbox->text[i] == '\t') || (textbox->text[i] == '\0')) {
            textbox->text_words++;
            if ((chars_in_line + chars_in_word) <= max_width) {
                chars_in_line += chars_in_word;
            } else {
                textbox->text_rows++;
                chars_in_line = chars_in_word;
            }
            chars_in_word = 0;
        }
    }

    if ((chars_in_line > 0)) {
        textbox->text_rows++;
    }
}

static void textbox_on_resize(widget_t *widget, int height, int width, int y,
                              int x) {
    textbox_t *textbox = (textbox_t *)widget->data;
    wresize(widget->base.window, height, width);
    mvwin(widget->base.window, y, x);
    calculate_rows_and_words(widget);
    textbox_set_alignment(widget, textbox->stored_v_alignment,
                          textbox->h_alignment);
    textbox->first_row_shown = 0;
    widget->base.dirty = true;
}

static bool textbox_on_refresh(widget_t *widget) {
    if (!widget->base.dirty) {
        return false;
    }

    // TODO werase or wclear ?
    // TODO erasing the whole screen shows like a jitter in the terminal.
    // Be smarter about it
    wclear(widget->base.window);

    draw_border(widget);

    alignment_to_position(widget);

    wnoutrefresh(widget->base.window);
    widget->base.dirty = false;
    return true;
}

static bool textbox_on_focus(widget_t *widget, int key) {
    textbox_t *textbox = widget->data;
    bool key_was_consumed = false;

    switch (key) {
    case 's': {
        textbox->first_row_shown =
            (textbox->last_row_shown < textbox->text_rows - 1)
                ? textbox->first_row_shown + 1
                : textbox->first_row_shown;
        key_was_consumed = true;
        widget->base.dirty = true;
        break;
    }

    case 'w': {
        textbox->first_row_shown = (textbox->first_row_shown > 0)
                                       ? textbox->first_row_shown - 1
                                       : textbox->first_row_shown;
        key_was_consumed = true;
        widget->base.dirty = true;
        break;
    }

    default: {
        break;
    }
    }

    textbox_set_border_color(widget, COLOR_RED_BLACK);
    return key_was_consumed;
}

static void textbox_on_lose_focus(widget_t *widget) {
    textbox_t *textbox = widget->data;
    textbox_set_border_color(widget, textbox->stored_border_color);
}

/***[Public functions]********************************************************/
widget_t *textbox_new(const char *text) {
    widget_t *widget = (widget_t *)malloc(sizeof(widget_t));
    textbox_t *textbox = (textbox_t *)malloc(sizeof(textbox_t));

    if ((widget == NULL) || (textbox == NULL)) {
        free(widget);
        free(textbox);
        return NULL;
    }

    widget_init(widget, (void *)textbox, WIDGET_TEXTBOX, newwin(1, 1, 1, 1),
                textbox_on_refresh, textbox_del, textbox_on_focus,
                textbox_on_lose_focus, textbox_on_resize);

    textbox->text_color = COLOR_WHITE_BLACK;
    textbox->text = NULL;
    textbox_set_text(widget, text);
    textbox_set_alignment(widget, TEXT_V_ALIGN_TOP, TEXT_H_ALIGN_LEFT);
    textbox_enable_row_counter(widget);
    textbox_enable_word_counter(widget);

    textbox->border_color = COLOR_WHITE_BLACK;
    textbox->stored_border_color = COLOR_WHITE_BLACK;

    return widget;
}

widget_status_t textbox_del(widget_t *widget) {

    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    wclear(widget->base.window);
    delwin(widget->base.window);
    free((void *)textbox->text);
    free((void *)textbox);
    free((void *)widget);

    return WIDGET_OK;
}

widget_status_t textbox_set_text(widget_t *widget, const char *text) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    free((void *)textbox->text);
    textbox->text = (char *)malloc(strlen(text) + 1);

    if (textbox->text == NULL) {
        // Point to a safety "\0" to avoid a NULL pointer in text's accesses
        textbox->text = (char *)&g_null_char;
        return WIDGET_ENOMEM;
    }

    strncpy(textbox->text, text, strlen(text) + 1);
    calculate_rows_and_words(widget);

    textbox->first_row_shown = 0;

    widget->base.dirty = true;
    return WIDGET_OK;
}

widget_status_t textbox_set_color(widget_t *widget, color_t color) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->text_color = color;
    widget->base.dirty = true;
    return WIDGET_OK;
}

widget_status_t textbox_set_alignment(widget_t *widget,
                                      text_v_align_t v_alignment,
                                      text_h_align_t h_alignment) {
    textbox_t *textbox = NULL;
    const int max_height = getmaxy(widget->base.window) - 2;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->h_alignment = h_alignment;
    textbox->stored_v_alignment = v_alignment;

    // If the text does not fit in the textbox, the alignment is forced to be
    // from the top.
    textbox->v_alignment = (textbox->text_rows < max_height)
                               ? textbox->stored_v_alignment
                               : TEXT_V_ALIGN_TOP;
    widget->base.dirty = true;
    return WIDGET_OK;
}

widget_status_t textbox_set_border_color(widget_t *widget, color_t color) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    if (textbox->border_color != color) {
        textbox->stored_border_color = textbox->border_color;
        textbox->border_color = color;
        widget->base.dirty = true;
    }
    return WIDGET_OK;
}

widget_status_t textbox_enable_word_counter(widget_t *widget) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->word_counter_enabled = true;
    widget->base.dirty = true;

    return WIDGET_OK;
}

widget_status_t textbox_disable_word_counter(widget_t *widget) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->word_counter_enabled = false;
    widget->base.dirty = true;

    return WIDGET_OK;
}

widget_status_t textbox_enable_row_counter(widget_t *widget) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->row_counter_enabled = true;
    widget->base.dirty = true;

    return WIDGET_OK;
}

widget_status_t textbox_disable_row_counter(widget_t *widget) {
    textbox_t *textbox = NULL;
    widget_status_t ret =
        widget_cast(widget, (void **)&textbox, WIDGET_TEXTBOX);

    if (ret != WIDGET_OK) {
        return ret;
    }

    textbox->row_counter_enabled = false;
    widget->base.dirty = true;

    return WIDGET_OK;
}
