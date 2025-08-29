#include "lv_api_map.h"
#include "lvgl/lv_ui.h"
#include <lvgl.h>
#include <lvgl_input_device.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "brew_thread.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define TOUCH_NODE DT_NODELABEL(touch_irq)

/* Touch Interrupt */
static struct gpio_dt_spec touch_irq =
	GPIO_DT_SPEC_GET_OR(TOUCH_NODE, gpios, {0});
static struct gpio_callback touch_callback;

static void touch_isr_callback(const struct device *port,
			       struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("Touch ISR.");
	lv_task_handler();
	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
}

int main(void)
{
	const struct device *display_dev;
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s is not ready. Exiting..", display_dev->name);
		return 0;
	}

	/* Initialize touch IRQ */
	if (gpio_is_ready_dt(&touch_irq)) {
		int err;

		err = gpio_pin_configure_dt(&touch_irq, GPIO_INPUT);
		if (err) {
			LOG_ERR("Failed to configure gpio for interrupt: %d",
				err);
			return 0;
		}

		gpio_init_callback(&touch_callback, touch_isr_callback,
				   BIT(touch_irq.pin));

		err = gpio_add_callback(touch_irq.port, &touch_callback);
		if (err) {
			LOG_ERR("Failed to add touch interrupt callback: %d",
				err);
			return 0;
		}
		err = gpio_pin_interrupt_configure_dt(&touch_irq,
						      GPIO_INT_DISABLE);
		if (err) {
			LOG_ERR("Failed to enable touch interrupt callback: %d",
				err);
			return 0;
		}
	}

	/* Initialize LVGL */
	lv_ui_init();
	lv_task_handler();
	k_msleep(100);
	display_blanking_off(display_dev);

	brew_thread_start();

	while (1) {
		lv_task_handler();
		k_msleep(10);
	}
}
