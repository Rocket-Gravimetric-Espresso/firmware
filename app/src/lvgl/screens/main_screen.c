#include "main_screen.h"
#include "core/lv_disp.h"
#include "core/lv_event.h"
#include "core/lv_obj_pos.h"
#include "core/lv_obj_style.h"
#include "extra/widgets/spinbox/lv_spinbox.h"
#include "misc/lv_area.h"
#include "misc/lv_color.h"
#include "widgets/lv_img.h"
#include "zephyr/logging/log.h"
#include "zephyr/toolchain.h"
#include <lvgl.h>

LOG_MODULE_REGISTER(ui, LOG_LEVEL_INF);
LV_IMG_DECLARE(portafilter);

static lv_obj_t *screen_main;
static lv_obj_t *screen_dose_ratio;
static lv_obj_t *screen_extraction;
static lv_obj_t *background;
static lv_style_t color_red;
static lv_color_t color_white;
static lv_style_t font_large;

void lv_set_properties(void)
{
    lv_style_init(&color_red);
    lv_style_set_bg_color(&color_red, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&color_red,
                               lv_palette_lighten(LV_PALETTE_RED, 3));

    color_white = lv_color_white();

    lv_style_init(&font_large);
    lv_style_set_text_font(&font_large, &lv_font_montserrat_22);
}

void lv_set_background(void)
{
    background = lv_img_create(lv_scr_act());
    lv_img_set_src(background, &portafilter);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(background, 320, 240);
}

void show_main_screen(void)
{
    LOG_INF("Main.");
    screen_main = lv_obj_create(NULL);
    lv_scr_load(screen_main);
    lv_set_background();
    lv_set_properties();

    lv_obj_t *button_brew = lv_btn_create(lv_scr_act());
    lv_obj_align(button_brew, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(button_brew, 80, 80);
    lv_obj_set_style_radius(button_brew, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(button_brew, &color_red, 0);

    lv_obj_t *label = lv_label_create(button_brew);
    lv_label_set_text(label, "BREW");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(button_brew, main_screen_event_handler,
                        LV_EVENT_CLICKED, NULL);
}

void main_screen_event_handler(lv_event_t *e)
{
    ARG_UNUSED(e);
    show_settings_screen();
}

void lv_set_label(lv_obj_t *parent, char *label) {}

static lv_obj_t *spinbox_dose;

static void lv_spinbox_increment_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED ||
        code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_dose);
    }
}

static void lv_spinbox_decrement_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED ||
        code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_dose);
    }
}

void lv_example_spinbox_1(void)
{
    spinbox_dose = lv_spinbox_create(lv_scr_act());
    lv_spinbox_set_range(spinbox_dose, 10, 22);
    lv_spinbox_set_value(spinbox_dose, 14);
    lv_spinbox_set_digit_format(spinbox_dose, 2, 0);
    lv_spinbox_set_pos(spinbox_dose, 2);
    lv_spinbox_step_prev(spinbox_dose);
    lv_obj_set_width(spinbox_dose, 40);
    lv_obj_align(spinbox_dose, LV_ALIGN_CENTER, -60, -20);

    lv_obj_t *label_grams = lv_label_create(lv_scr_act());
    lv_label_set_text(label_grams, "grams");
    lv_obj_align_to(label_grams, spinbox_dose, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_text_color(label_grams, color_white, 0);

    lv_coord_t h = lv_obj_get_height(spinbox_dose);

    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox_dose, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, NULL);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox_dose, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, NULL);
}

void show_settings_screen(void)
{
    LOG_INF("Dose and Ratio.");
    screen_dose_ratio = lv_obj_create(NULL);
    lv_scr_load(screen_dose_ratio);
    lv_set_background();

    lv_obj_t *label_dose_ratio = lv_label_create(lv_scr_act());
    lv_label_set_text(label_dose_ratio, "Dose and Ratio");
    lv_obj_align(label_dose_ratio, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(label_dose_ratio, color_white, 0);
    lv_obj_add_style(label_dose_ratio, &font_large, LV_PART_MAIN);

    lv_example_spinbox_1();

    lv_obj_t *button_extract = lv_btn_create(lv_scr_act());
    lv_obj_align(button_extract, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_size(button_extract, 80, 80);
    lv_obj_set_style_radius(button_extract, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(button_extract, &color_red, 0);

    lv_obj_t *label = lv_label_create(button_extract);
    lv_label_set_text(label, "GO!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(button_extract, settings_screen_event_handler,
                        LV_EVENT_CLICKED, NULL);
}

void settings_screen_event_handler(lv_event_t *e)
{
    ARG_UNUSED(e);
    show_extraction_screen();
}

static void set_angle(void *obj, int32_t v) { lv_arc_set_value(obj, v); }

void lv_example_arc_2(void)
{
    /*Create an Arc*/
    lv_obj_t *arc = lv_arc_create(lv_scr_act());
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL,
                        LV_PART_KNOB); /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(
        arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
    lv_obj_center(arc);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_start(&a);
}

void show_extraction_screen(void)
{
    LOG_INF("Extraction.");
    screen_extraction = lv_obj_create(NULL);
    lv_scr_load(screen_extraction);
    lv_set_background();

    lv_obj_t *label_extraction = lv_label_create(screen_extraction);
    lv_label_set_text(label_extraction, "Extracting");
    lv_obj_align(label_extraction, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(label_extraction, color_white, 0);
    lv_obj_add_style(label_extraction, &font_large, LV_PART_MAIN);

    lv_example_arc_2();

    lv_obj_t *button_stop = lv_btn_create(lv_scr_act());
    lv_obj_align(button_stop, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_size(button_stop, 60, 60);
    lv_obj_set_style_radius(button_stop, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_style(button_stop, &color_red, 0);

    lv_obj_t *label_stop = lv_label_create(button_stop);
    lv_label_set_text(label_stop, "STOP!");
    lv_obj_align(label_stop, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(button_stop, extraction_screen_event_handler,
                        LV_EVENT_CLICKED, NULL);
}

void extraction_screen_event_handler(lv_event_t *e)
{
    ARG_UNUSED(e);
    show_main_screen();
}
