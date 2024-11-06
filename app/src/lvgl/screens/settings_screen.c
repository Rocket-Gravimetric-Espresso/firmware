#include "settings_screen.h"
#include "extraction_screen.h"
#include <lvgl.h>

lv_obj_t *settings_screen;

void show_settings_screen(void) {
    settings_screen = lv_obj_create(NULL);
    lv_scr_load(settings_screen);

    lv_obj_t *btn = lv_btn_create(settings_screen);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 120, 50);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Start extraction");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(btn, settings_screen_event_handler, LV_EVENT_CLICKED,
                        NULL);
}

void settings_screen_event_handler(lv_event_t *e) { show_extraction_screen(); }
