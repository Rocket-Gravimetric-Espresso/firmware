#include <lvgl.h>

lv_obj_t *create_button(lv_obj_t *parent, const char *text,
                        lv_event_cb_t event_cb) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 100, 50);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    return btn;
}
