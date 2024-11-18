#include "../../../modules/nau7802_loadcell/drivers/sensor/nau7802_loadcell/nau7802_loadcell.h"
#include "lvgl/screens/main_screen.h"
#include "syscalls/sensor.h"
#include "zephyr/drivers/sensor.h"
#include <lvgl.h>
#include <lvgl_input_device.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rocket, LOG_LEVEL_INF);

#define LED4_NODE DT_ALIAS(led4)

#ifdef CONFIG_GPIO
static struct gpio_dt_spec touch_interrupt_gpio =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(touch), gpios, {0});
static struct gpio_callback touch_callback;

// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

static int process_sensor_value(const struct device *dev) {
    struct sensor_value force;
    int ret;

    ret = sensor_sample_fetch(dev);
    if (ret != 0) {
        LOG_ERR("ret: %d, Sensor sample update error", ret);
        return ret;
    }

    ret = sensor_channel_get(dev, SENSOR_CHAN_FORCE, &force);
    if (ret != 0) {
        LOG_ERR("ret: %d, Cannot read NAU7802 force channelr", ret);
        return ret;
    }

    // LOG_INF("Force:%f", sensor_value_to_double(&force));
    LOG_INF("Force: %d, %d", force.val1, force.val2);
    return 0;
}

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

    const struct device *const adc_dev = DEVICE_DT_GET(DT_NODELABEL(nau7802));

    if (!device_is_ready(adc_dev)) {
        LOG_ERR("Device %s is not ready. Exiting..", adc_dev->name);
        return 0;
    }

    float32_t offset = 2.505000511475965652e+01;
    float32_t calibFactor = 6.527824679590503775e-06;
    const struct sensor_value offset_attri;
    const struct sensor_value calib_attri;

    memcpy(&offset_attri.val1, &offset, sizeof(offset));
    memcpy(&calib_attri.val1, &calibFactor, sizeof(calibFactor));

    if (sensor_attr_set(adc_dev, SENSOR_CHAN_FORCE, SENSOR_ATTR_OFFSET,
                        &offset_attri) != 0) {
        LOG_ERR("Cannot configure the offset");
        return 0;
    }

    if (sensor_attr_set(adc_dev, SENSOR_CHAN_FORCE, SENSOR_ATTR_CALIBRATION,
                        &calib_attri) != 0) {
        LOG_ERR("Cannot configure the offset");
        return 0;
    }

    // int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

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
        k_sleep(K_MSEC(2000));
        // ret = gpio_pin_toggle_dt(&led);
        // res = gpio_pin_get_raw(touch_interrupt_gpio.port,
        //                       touch_interrupt_gpio.pin);
        process_sensor_value(adc_dev);
    }
}
