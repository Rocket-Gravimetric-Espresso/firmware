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
static struct gpio_dt_spec button_gpio =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static struct gpio_callback button_callback;

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
    if (gpio_is_ready_dt(&button_gpio)) {
        int err;

        err = gpio_pin_configure_dt(&button_gpio, GPIO_INPUT);
        if (err) {
            LOG_ERR("failed to configure button gpio: %d", err);
            return 0;
        }

        gpio_init_callback(&button_callback, button_isr_callback,
                           BIT(button_gpio.pin));

        err = gpio_add_callback(button_gpio.port, &button_callback);
        if (err) {
            LOG_ERR("failed to add button callback: %d", err);
            return 0;
        }

        err = gpio_pin_interrupt_configure_dt(&button_gpio,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (err) {
            LOG_ERR("failed to enable button callback: %d", err);
            return 0;
        }
    }
#endif /* CONFIG_GPIO */

    lv_task_handler();
    display_blanking_off(display_dev);

    show_main_screen();

    while (1) {
        lv_task_handler();
        k_sleep(K_MSEC(10));
    }
}
