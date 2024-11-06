#include "lvgl/screens/main_screen.h"
#include <lvgl.h>
#include <lvgl_input_device.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rocket, LOG_LEVEL_INF);

#ifdef CONFIG_GPIO
static struct gpio_dt_spec touch_interrupt_gpio =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(touch), gpios, {0});
static struct gpio_callback touch_callback;

static void button_isr_callback(const struct device *port,
                                struct gpio_callback *cb, uint32_t pins) {
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
}
#endif /* CONFIG_GPIO */

int main(void) {
    const struct device *display_dev;
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Device %s is not ready. Exiting..", display_dev->name);
        return 0;
    }

#ifdef CONFIG_GPIO
    if (gpio_is_ready_dt(&touch_interrupt_gpio)) {
        int err;

        err = gpio_pin_configure_dt(&touch_interrupt_gpio, GPIO_INPUT);
        if (err) {
            LOG_ERR("Failed to configure gpio for interrupt: %d", err);
            return 0;
        }

        gpio_init_callback(&touch_callback, button_isr_callback,
                           BIT(touch_interrupt_gpio.pin));

        err = gpio_add_callback(touch_interrupt_gpio.port, &touch_callback);
        if (err) {
            LOG_ERR("Failed to add touch interrupt callback: %d", err);
            return 0;
        }
        err = gpio_pin_interrupt_configure_dt(&touch_interrupt_gpio,
                                              GPIO_INT_DISABLE);
        if (err) {
            LOG_ERR("Failed to enable touch interrupt callback: %d", err);
            return 0;
        }
    }
#endif /* CONFIG_GPIO */

    show_main_screen();
    lv_task_handler();
    display_blanking_off(display_dev);
    int res;

    while (1) {
        lv_task_handler();
        k_sleep(K_MSEC(10));
        res = gpio_pin_get_raw(touch_interrupt_gpio.port,
                               touch_interrupt_gpio.pin);
        // LOG_INF("GPIO level: %d", res);
    }
}
