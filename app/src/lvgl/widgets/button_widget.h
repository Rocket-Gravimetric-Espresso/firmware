#ifndef BUTTON_WIDGET_H
#define BUTTON_WIDGET_H

#include <lvgl.h>

lv_obj_t *create_button(lv_obj_t *parent, const char *text,
                        lv_event_cb_t event_cb);

#endif // BUTTON_WIDGET_H
