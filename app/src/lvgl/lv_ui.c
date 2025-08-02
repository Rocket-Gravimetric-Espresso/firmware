#include "font/lv_font.h"
#include "lv_ui.h"
#include "core/lv_disp.h"
#include "core/lv_event.h"
#include "core/lv_obj.h"
#include "core/lv_obj_pos.h"
#include "core/lv_obj_style.h"
#include "misc/lv_area.h"
#include "misc/lv_color.h"
#include "misc/lv_style.h"
#include "widgets/lv_img.h"
#include "widgets/lv_label.h"
#include "zephyr/logging/log.h"
#include "zephyr/toolchain.h"
#include "zephyr/random/random.h"
#include <lvgl.h>

LOG_MODULE_REGISTER(ui, LOG_LEVEL_INF);
LV_IMG_DECLARE(portafilter);

static lv_obj_t *screen;
static lv_obj_t *background;

static lv_obj_t *view_home;
static lv_obj_t *view_adjust;
static lv_obj_t *view_extract;

static lv_style_t color_red;
static lv_color_t color_white;
static lv_style_t font_large;
static lv_style_t font_medium;

static void lv_ui_view_create(lv_obj_t **view);
static void lv_ui_home_init(void);
static void lv_ui_adjustment_init(void);
static void lv_ui_extraction_init(void);
static void lv_event_home_brew_cb(lv_event_t *e);
static void lv_event_adjustment_go_cb(lv_event_t *e);
static void lv_event_extraction_stop_cb(lv_event_t *e);

static void lv_ui_increment_dose_event_cb(lv_event_t *e);
static void lv_ui_decrement_dose_event_cb(lv_event_t *e);
static void lv_ui_increment_ratio_event_cb(lv_event_t *e);
static void lv_ui_decrement_ratio_event_cb(lv_event_t *e);

const char *bad_jokes[] = {
	"How did the hipster burn his tongue?\nHe drank his coffee before it "
	"was cool.",
	"What’s a barista’s favorite exercise?\nThe French press!",
	"Why do coffee beans never get a break?\nBecause they're always "
	"getting roasted!",
	"Espresso may not solve all your problems.\nBut it's worth a shot!",
	"I told my coffee it needs to chill.\nNow it's iced coffee.",
	"How do coffee beans say goodbye?\nSee you latte!"};

enum ground_dose {
	TEN = 10,
	ELEVEN,
	TWELVE,
	THIRTEEN,
	FOURTEEN,
	FIFTEEN,
	SIXTEEN,
	SEVENTEEN,
	EIGHTEEN,
	NINETEEN,
	TWENTY
};

enum brew_ratio {
	ONE_TO_ONE = 1,
	ONE_TO_TWO,
	ONE_TO_THREE
};

enum ground_dose chosen_dose = FOURTEEN;
enum brew_ratio chosen_ratio = ONE_TO_TWO;

/*
 * Initialize UI screen with styles shared across views
 */
void lv_ui_init(void)
{
	lv_style_init(&color_red);
	lv_style_set_bg_color(&color_red, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_bg_grad_color(&color_red,
				   lv_palette_lighten(LV_PALETTE_RED, 3));

	color_white = lv_color_white();

	lv_style_init(&font_medium);
	lv_style_set_text_font(&font_medium, &lv_font_montserrat_18);
	lv_style_init(&font_large);
	lv_style_set_text_font(&font_large, &lv_font_montserrat_22);

	screen = lv_obj_create(NULL);

	background = lv_img_create(screen);
	lv_img_set_src(background, &portafilter);
	lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_size(background, 320, 240);

	lv_ui_home_init();
	lv_ui_adjustment_init();
	lv_ui_extraction_init();

	lv_obj_clear_flag(view_home, LV_OBJ_FLAG_HIDDEN);
	lv_scr_load(screen);
}

/*
 * Create a transparent container for a view
 */
static void lv_ui_view_create(lv_obj_t **view)
{
	*view = lv_obj_create(screen);
	lv_obj_set_size(*view, 320, 240);
	lv_obj_set_style_border_width(*view, 0, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(*view, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_add_flag(*view, LV_OBJ_FLAG_HIDDEN);
}

/*
 * Init the UI home view
 */
static void lv_ui_home_init(void)
{
	lv_ui_view_create(&view_home);

	lv_obj_t *label_menu = lv_label_create(view_home);
	lv_label_set_text(label_menu, "Hello!");
	lv_obj_align(label_menu, LV_ALIGN_TOP_MID, 0, 10);
	lv_obj_set_style_text_color(label_menu, color_white, 0);
	lv_obj_add_style(label_menu, &font_large, LV_PART_MAIN);

	lv_obj_t *button = lv_btn_create(view_home);
	lv_obj_align(button, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_size(button, 80, 80);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_style(button, &color_red, 0);

	lv_obj_t *label_button = lv_label_create(button);
	lv_label_set_text(label_button, "BREW");
	lv_obj_align(label_button, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t *joke = lv_label_create(view_home);
	lv_label_set_text(joke, bad_jokes[sys_rand8_get() % 7]);
	lv_obj_set_style_text_color(joke, lv_color_white(), 0);
	lv_obj_align(joke, LV_ALIGN_BOTTOM_MID, 0, 0);

	lv_obj_add_event_cb(button, lv_event_home_brew_cb, LV_EVENT_CLICKED,
			    NULL);
}

static void lv_event_home_brew_cb(lv_event_t *e)
{
	ARG_UNUSED(e);
	lv_obj_add_flag(view_home, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(view_adjust, LV_OBJ_FLAG_HIDDEN);
}

/*
 * Init UI adjustment view
 */
static void lv_ui_adjustment_init(void)
{
	lv_ui_view_create(&view_adjust);

	lv_obj_t *label_menu = lv_label_create(view_adjust);
	lv_label_set_text(label_menu, "Dose and Ratio");
	lv_obj_align(label_menu, LV_ALIGN_TOP_MID, 0, 10);
	lv_obj_set_style_text_color(label_menu, color_white, 0);
	lv_obj_add_style(label_menu, &font_large, LV_PART_MAIN);

	/* Adjust ground dose */
	lv_obj_t *label_gram = lv_label_create(view_adjust);
	lv_obj_add_style(label_gram, &font_medium, LV_PART_MAIN);
	lv_obj_set_style_text_color(label_gram, color_white, 0);
	lv_label_set_text(label_gram, " g");
	lv_obj_align(label_gram, LV_ALIGN_CENTER, -55, -10);
	lv_obj_t *label_dose = lv_label_create(view_adjust);
	lv_obj_add_style(label_gram, &font_medium, LV_PART_MAIN);
	lv_obj_set_style_text_color(label_dose, color_white, 0);
	lv_label_set_text(label_dose, "14");
	lv_obj_align_to(label_dose, label_gram, LV_ALIGN_OUT_LEFT_MID, 0, 0);
	lv_obj_t *button = lv_btn_create(view_adjust);
	lv_obj_set_size(button, 40, 40);
	lv_obj_align(button, LV_ALIGN_CENTER, -20, -10);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_img_src(button, LV_SYMBOL_PLUS, 0);
	lv_obj_add_event_cb(button, lv_ui_increment_dose_event_cb, LV_EVENT_ALL,
			    label_dose);
	button = lv_btn_create(view_adjust);
	lv_obj_set_size(button, 40, 40);
	lv_obj_align(button, LV_ALIGN_CENTER, -110, -10);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_img_src(button, LV_SYMBOL_MINUS, 0);
	lv_obj_add_event_cb(button, lv_ui_decrement_dose_event_cb, LV_EVENT_ALL,
			    label_dose);

	/* Adjust ratio */
	lv_obj_t *label_ratio = lv_label_create(view_adjust);
	lv_label_set_text(label_ratio, "1:2");
	lv_obj_align(label_ratio, LV_ALIGN_CENTER, -65, 60);
	lv_obj_set_style_text_color(label_ratio, color_white, 0);
	lv_obj_add_style(label_ratio, &font_medium, LV_PART_MAIN);
	button = lv_btn_create(view_adjust);
	lv_obj_set_size(button, 40, 40);
	lv_obj_align(button, LV_ALIGN_CENTER, -20, 60);
	lv_obj_set_style_bg_img_src(button, LV_SYMBOL_PLUS, 0);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_event_cb(button, lv_ui_increment_ratio_event_cb,
			    LV_EVENT_ALL, label_ratio);
	button = lv_btn_create(view_adjust);
	lv_obj_set_size(button, 40, 40);
	lv_obj_align(button, LV_ALIGN_CENTER, -110, 60);
	lv_obj_set_style_bg_img_src(button, LV_SYMBOL_MINUS, 0);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_event_cb(button, lv_ui_decrement_ratio_event_cb,
			    LV_EVENT_ALL, label_ratio);

	button = lv_btn_create(view_adjust);
	lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
	lv_obj_set_size(button, 80, 80);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_style(button, &color_red, 0);
	lv_obj_t *label = lv_label_create(button);
	lv_label_set_text(label, "GO!");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_event_cb(button, lv_event_adjustment_go_cb, LV_EVENT_CLICKED,
			    NULL);
}

static void lv_event_adjustment_go_cb(lv_event_t *e)
{
	ARG_UNUSED(e);
	lv_obj_add_flag(view_adjust, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(view_extract, LV_OBJ_FLAG_HIDDEN);
}

static void lv_ui_increment_dose_event_cb(lv_event_t *e)
{
	lv_obj_t *label = lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED ||
	    code == LV_EVENT_LONG_PRESSED_REPEAT) {
		switch (chosen_dose) {
		case 10:
			chosen_dose = ELEVEN;
			lv_label_set_text(label, "11");
			break;
		case 11:
			chosen_dose = TWELVE;
			lv_label_set_text(label, "12");
			break;
		case 12:
			chosen_dose = THIRTEEN;
			lv_label_set_text(label, "13");
			break;
		case 13:
			chosen_dose = FOURTEEN;
			lv_label_set_text(label, "14");
			break;
		case 14:
			chosen_dose = FIFTEEN;
			lv_label_set_text(label, "15");
			break;
		case 15:
			chosen_dose = SIXTEEN;
			lv_label_set_text(label, "16");
			break;
		case 16:
			chosen_dose = SEVENTEEN;
			lv_label_set_text(label, "17");
			break;
		case 17:
			chosen_dose = EIGHTEEN;
			lv_label_set_text(label, "18");
			break;
		case 18:
			chosen_dose = NINETEEN;
			lv_label_set_text(label, "19");
			break;
		case 19:
			chosen_dose = TWENTY;
			lv_label_set_text(label, "20");
			break;
		case 20:
			break;
		}
	}
}

static void lv_ui_decrement_dose_event_cb(lv_event_t *e)
{
	lv_obj_t *label = lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED ||
	    code == LV_EVENT_LONG_PRESSED_REPEAT) {
		switch (chosen_dose) {
		case 10:
			break;
		case 11:
			chosen_dose = TEN;
			lv_label_set_text(label, "10");
			break;
		case 12:
			chosen_dose = ELEVEN;
			lv_label_set_text(label, "11");
			break;
		case 13:
			chosen_dose = TWELVE;
			lv_label_set_text(label, "12");
			break;
		case 14:
			chosen_dose = THIRTEEN;
			lv_label_set_text(label, "13");
			break;
		case 15:
			chosen_dose = FOURTEEN;
			lv_label_set_text(label, "14");
			break;
		case 16:
			chosen_dose = FIFTEEN;
			lv_label_set_text(label, "15");
			break;
		case 17:
			chosen_dose = SIXTEEN;
			lv_label_set_text(label, "16");
			break;
		case 18:
			chosen_dose = SEVENTEEN;
			lv_label_set_text(label, "17");
			break;
		case 19:
			chosen_dose = EIGHTEEN;
			lv_label_set_text(label, "18");
			break;
		case 20:
			chosen_dose = NINETEEN;
			lv_label_set_text(label, "19");
			break;
		}
	}
}
static void lv_ui_increment_ratio_event_cb(lv_event_t *e)
{
	lv_obj_t *label = lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED ||
	    code == LV_EVENT_LONG_PRESSED_REPEAT) {
		switch (chosen_ratio) {
		case 1:
			chosen_ratio = ONE_TO_TWO;
			lv_label_set_text(label, "1:2");
			break;
		case 2:
			chosen_ratio = ONE_TO_THREE;
			lv_label_set_text(label, "1:3");
			break;
		case 3:
			break;
		}
	}
}

static void lv_ui_decrement_ratio_event_cb(lv_event_t *e)
{
	lv_obj_t *label = lv_event_get_user_data(e);
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED ||
	    code == LV_EVENT_LONG_PRESSED_REPEAT) {
		switch (chosen_ratio) {
		case 1:
			break;
		case 2:
			chosen_ratio = ONE_TO_ONE;
			lv_label_set_text(label, "1:1");
			break;
		case 3:
			chosen_ratio = ONE_TO_TWO;
			lv_label_set_text(label, "1:2");
			break;
		}
	}
}

static void lv_ui_arc_set_angle(void *obj, int32_t v)
{
	lv_arc_set_value(obj, v);
}

static lv_obj_t *lv_ui_arc_create(lv_obj_t *parent)
{
	lv_obj_t *arc = lv_arc_create(parent);
	lv_obj_set_size(arc, 100, 100);
	// lv_obj_align(arc, LV_ALIGN_CENTER, -120, 0);
	lv_arc_set_rotation(arc, 270);
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_center(arc);

	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, arc);
	lv_anim_set_exec_cb(&a, lv_ui_arc_set_angle);
	lv_anim_set_time(&a, 1000);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_repeat_delay(&a, 500);
	lv_anim_set_values(&a, 0, 100);
	lv_anim_start(&a);
	return arc;
}

/*
 * Init UI extraction view
 */
static void lv_ui_extraction_init(void)
{
	lv_ui_view_create(&view_extract);

	lv_obj_t *label_menu = lv_label_create(view_extract);
	lv_label_set_text(label_menu, "Extracting");
	lv_obj_align(label_menu, LV_ALIGN_TOP_MID, 0, 10);
	lv_obj_set_style_text_color(label_menu, color_white, 0);
	lv_obj_add_style(label_menu, &font_large, LV_PART_MAIN);

	lv_obj_t *arc = lv_ui_arc_create(view_extract);

	lv_obj_t *label_weight = lv_label_create(arc);
	lv_label_set_text(label_weight, "0");
	lv_obj_set_style_text_color(label_weight, color_white, 0);
	lv_obj_align(label_weight, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_style(label_weight, &font_medium, LV_PART_MAIN);

	lv_obj_t *label_time = lv_label_create(view_extract);
	lv_label_set_text(label_time, "00:00");
	lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 70);
	lv_obj_set_style_text_color(label_time, color_white, 0);
	lv_obj_add_style(label_time, &font_medium, LV_PART_MAIN);

	lv_obj_t *button = lv_btn_create(view_extract);
	lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
	lv_obj_set_size(button, 50, 50);
	lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
	lv_obj_add_style(button, &color_red, 0);

	lv_obj_set_style_bg_img_src(button, LV_SYMBOL_STOP, 0);

	lv_obj_add_event_cb(button, lv_event_extraction_stop_cb,
			    LV_EVENT_CLICKED, NULL);
}

static void lv_event_extraction_stop_cb(lv_event_t *e)
{
	ARG_UNUSED(e);
	lv_obj_add_flag(view_extract, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(view_home, LV_OBJ_FLAG_HIDDEN);
}
