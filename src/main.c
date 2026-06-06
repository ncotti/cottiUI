/***[Includes]****************************************************************/
#include "main.h"
#include "button.h"
#include "color.h"
#include "layout.h"
#include "logger.h"
#include "textbox.h"
#include "widget.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

/***[Static variables]********************************************************/

// 5 paragraphs, 365 words, (2462 + 4 \n + 1 '\0') bytes of Lorem Ipsum
static const char lorem_ipsum_365[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse ac "
    "auctor tellus. In hac habitasse platea dictumst. Proin sed tincidunt "
    "tortor. Donec dictum nibh id pharetra consequat. Vestibulum sit amet quam "
    "volutpat, rutrum nisi vitae, dictum eros. Etiam at mauris sem. Praesent "
    "sit amet ante justo. Duis porta urna est, ac imperdiet erat dictum "
    "eget.\n"
    "Phasellus suscipit dolor at risus efficitur sollicitudin. Quisque "
    "tellus sem, dapibus vitae molestie a, dignissim sed justo. Morbi interdum "
    "ex a ante aliquet tempus. Aliquam ac pulvinar nisl. Aenean efficitur "
    "purus vitae urna tincidunt, quis rhoncus sapien dictum. Praesent semper "
    "aliquet enim, vel dignissim felis ultricies nec. Donec viverra pretium "
    "nibh. Donec fringilla sodales malesuada. Cras venenatis dui ut justo "
    "tempor, lobortis rutrum ex tristique. Nam volutpat dolor diam, mollis "
    "vestibulum ipsum lacinia a.\n"
    "Morbi erat nisl, semper ut orci ut, finibus "
    "aliquam quam. Pellentesque mollis, ex quis ultricies gravida, ligula "
    "augue efficitur neque, id gravida ante ligula id felis. Fusce ullamcorper "
    "euismod ipsum. Sed interdum, leo ut maximus mollis, libero velit sagittis "
    "nunc, at maximus mi risus vel nisl. Duis sit amet leo nibh. Curabitur vel "
    "urna sed mauris tempor molestie in pulvinar velit. Proin semper felis nec "
    "leo laoreet, vitae dignissim justo aliquet. Suspendisse et nunc feugiat, "
    "molestie enim a, tincidunt diam. Proin ullamcorper lacus ut neque auctor "
    "tincidunt. Curabitur et sapien felis. Vestibulum a accumsan nunc, vel "
    "posuere purus. Integer felis sem, blandit a finibus in, fermentum non "
    "dui.\n"
    "Sed eleifend nisi lacus. Proin in laoreet risus. Vestibulum "
    "pharetra sapien turpis, id sagittis magna vulputate id. Nulla at arcu id "
    "quam molestie tristique. Proin sagittis tellus id porttitor efficitur. "
    "Vestibulum mattis lorem ligula, sed feugiat diam cursus vel. Ut "
    "scelerisque tempus nunc, id imperdiet eros aliquet a.\n"
    "Maecenas ligula "
    "massa, mollis nec massa commodo, consequat accumsan nulla. Aliquam "
    "aliquet, tellus at aliquam porta, justo ipsum tristique libero, non "
    "imperdiet neque erat vel massa. Praesent sollicitudin efficitur felis, in "
    "lobortis leo. Curabitur aliquet elementum metus eget consequat. Sed "
    "tempus sed ligula at iaculis. Aliquam nec libero sed massa congue sodales "
    "et a sem. Pellentesque molestie tellus suscipit, ullamcorper neque sit "
    "amet, pulvinar justo. Vestibulum vulputate dui a semper suscipit. Duis "
    "laoreet consequat ante, a vestibulum eros facilisis nec.";

// 1 paragraphs, 50 words, 334 + 1 bytes of Lorem Ipsum
static const char lorem_ipsum_50[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse "
    "quis neque vel ante dapibus dignissim at at dui. Fusce non commodo mi, "
    "vitae pharetra dui. Integer at sapien risus. Phasellus a cursus nisi, sit "
    "amet egestas dui. In venenatis pulvinar mauris. Pellentesque tincidunt "
    "justo ut convallis lacinia. Donec luctus euismod.";

/***[Static functions
   prototypes]*********************************************/

/***[Static
   functions]********************************************************/
void button_press(void *params) {
    widget_t *text_widget = (widget_t *)params;
    textbox_set_color(text_widget, random() % 7);
}

/***[Public functions]********************************************************/
int main(void) {
    logger_init();
    widget_t *layout = layout_new(stdscr, 2, 2);
    // widget_t *text_ul =
    //     textbox_new("This text is very large, so it will likely take "
    //                 "more than one row to be written. Also, since I am "
    //                 "here, I would like to say gambatte!, you can do it!");

    widget_t *text_ul = textbox_new(lorem_ipsum_50);
    widget_t *text_dl = textbox_new(lorem_ipsum_50);
    widget_t *text_dr = textbox_new("DR");

    WINDOW *a = newwin(1, 1, 1, 1);
    widget_t *sub_layout = layout_new(a, 2, 2);

    widget_t *text_uul = textbox_new("UL");
    widget_t *text_uur = textbox_new("UR");
    widget_t *text_ddl = textbox_new("DL");
    widget_t *text_ddr = textbox_new("DR");

    layout_add(sub_layout, text_uul, 0, 0);
    layout_add(sub_layout, text_uur, 0, 1);
    layout_add(sub_layout, text_ddl, 1, 0);
    layout_add(sub_layout, text_ddr, 1, 1);

    // widget_t *button =
    //     button_new(button_press, (void *)text_dr, "hi", '9',
    //     COLOR_CYAN_BLACK);

    layout_add(layout, text_ul, 0, 0);
    layout_add(layout, sub_layout, 0, 1);
    layout_add(layout, text_dl, 1, 0);
    layout_add(layout, text_dr, 1, 1);

    layout_show(layout);
    layout_del(layout);

    textbox_t *textbox = (textbox_t *)text_ul->data;
    printf("%d, %d\n", LINES, COLS);
    printf("COLORS: %d, COLOR_PAIRS: %d\n", COLORS, COLOR_PAIRS);
    printf("%d, %d\n", textbox->text_rows, textbox->text_words);
    return 0;
}
