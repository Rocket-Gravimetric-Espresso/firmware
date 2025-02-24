#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include <lvgl.h>

void lv_set_properties(void);
void show_main_screen(void);
void main_screen_event_handler(lv_event_t *e);
void show_settings_screen(void);
void settings_screen_event_handler(lv_event_t *e);
void show_extraction_screen(void);
void extraction_screen_event_handler(lv_event_t *e);
void lv_set_label(lv_obj_t *parent, char *label);
void lv_set_flex_layout(void);

#endif // MAIN_SCREEN_H
