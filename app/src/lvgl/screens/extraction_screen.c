#include "extraction_screen.h"
#include <lvgl.h>

lv_obj_t *extraction_screen;

void show_extraction_screen(void) {
    extraction_screen = lv_obj_create(NULL);
    lv_scr_load(extraction_screen);

    lv_obj_t *label = lv_label_create(extraction_screen);
    lv_label_set_text(label, "Extracting..");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void extraction_screen_event_handler(lv_event_t *e) {}
