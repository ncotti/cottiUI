#ifndef TEXTBOX_H_
#define TEXTBOX_H_

/***[Includes]****************************************************************/
#include "color.h"
#include "widget.h"
#include <ncurses.h>
#include <stdint.h>

/***[Macros]******************************************************************/

/***[Types]*******************************************************************/

/** @brief Horizontal alignment for text. */
typedef enum text_h_align_t {
    TEXT_H_ALIGN_LEFT,
    TEXT_H_ALIGN_CENTER,
    TEXT_H_ALIGN_RIGHT,
} text_h_align_t;

/** @brief Vertical alignment for text. */
typedef enum text_v_align_t {
    TEXT_V_ALIGN_TOP,
    TEXT_V_ALIGN_MIDDLE,
    TEXT_V_ALIGN_BOTTOM,
} text_v_align_t;

/** @brief Textbox widget */
typedef struct textbox_t {
    char *text;                 /**< Text to be shown */
    color_t text_color;         /**< Text color */
    text_h_align_t h_alignment; /**< Text's horizontal alignment */
    text_v_align_t
        v_alignment; /**< Text's currently applied vertical alignment */
    text_v_align_t
        stored_v_alignment; /**< User-set text's vertical alignment */
    uint16_t
        text_rows; /**< Number of rows that can fit in the textbox's window */
    uint16_t text_words;         /**< Text's word count */
    color_t border_color;        /**< Border's currently applied color */
    color_t stored_border_color; /**< User-set border's color */
    uint16_t first_row_shown; /**< First row shown on screen, starting from 0 */
    uint16_t last_row_shown;  /**< Last row shown on screen, starting from 0 */
    bool row_counter_enabled; /**< Whether to show the number of rows in the
                                 text */
    bool word_counter_enabled; /**< Whether to show the number of words in the
                                  text */
} textbox_t;

/***[Extern variables]********************************************************/

/***[Functions prototypes]****************************************************/

/**
 * @brief Creates a new textbox widget, i.e., a placeholder for text.
 * @param text Text to be written.
 * @return The newly created textbox widget, or NULL in case of error.
 */
widget_t *textbox_new(const char *text);

/**
 * @brief Clears the written text and deletes the textbox window.
 * @param widget Textbox widget.
 * @return "WIDGET_OK" on success, or an error code on failure.
 */
widget_status_t textbox_del(widget_t *widget);

/**
 * @brief Sets the text to be displayed.
 * @param widget Textbox widget.
 * @param text Text.
 * @return WIDGET_OK on success.
 */
widget_status_t textbox_set_text(widget_t *widget, const char *text);

/**
 * @brief Sets the color of the text.
 * @param widget Textbox widget.
 * @param color Color pair to use (foreground, background)
 * @return WIDGET_OK on success.
 */
widget_status_t textbox_set_color(widget_t *widget, color_t color);

/**
 * @brief Changes the text's vertical and horizontal alignment.
 * @param widget Textbox widget.
 * @param v_alignment Vertical alignment.
 * @param h_alignment Horizontal alignment.
 * @return WIDGET_OK on success.
 */
widget_status_t textbox_set_alignment(widget_t *widget,
                                      text_v_align_t v_alignment,
                                      text_h_align_t h_alignment);

/**
 * @brief Changes the border color.
 * @param widget Textbox widget.
 * @param color Border color (foreground, background).
 * @return widget_status_t WIDGET_OK on success.
 */
widget_status_t textbox_set_border_color(widget_t *widget, color_t color);

widget_status_t textbox_enable_word_counter(widget_t *widget);
widget_status_t textbox_disable_word_counter(widget_t *widget);
widget_status_t textbox_enable_row_counter(widget_t *widget);
widget_status_t textbox_disable_row_counter(widget_t *widget);

#endif // TEXTBOX_H_
