#include "main_screen.h"
#include "settings_screen.h"
#include <lvgl.h>

lv_obj_t *main_screen;

void show_main_screen(void) {
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);

    lv_obj_t *btn = lv_btn_create(main_screen);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 120, 50);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Brew");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(btn, main_screen_event_handler, LV_EVENT_CLICKED, NULL);
}

void main_screen_event_handler(lv_event_t *e) { show_settings_screen(); }
