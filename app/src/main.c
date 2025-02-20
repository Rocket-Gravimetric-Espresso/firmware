#include "../../../modules/nau7802_loadcell/drivers/sensor/nau7802_loadcell/nau7802_loadcell.h"
#include "lv_api_map.h"
#include "lvgl/screens/main_screen.h"
#include "syscalls/sensor.h"
#include "zephyr/drivers/sensor.h"
#include <lvgl.h>
#include <lvgl_input_device.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED4_NODE DT_NODELABEL(led4)
#define TOUCH_NODE DT_NODELABEL(touch_irq)
#define POWER_SW_NODE DT_NODELABEL(power_sw)

#define CALIBRATION_FACTOR 0.002877658408
#define ZERO_OFFFSET 0.0

/* Touch Interrupt */
static struct gpio_dt_spec touch_irq =
    GPIO_DT_SPEC_GET_OR(TOUCH_NODE, gpios, {0});
static struct gpio_callback touch_callback;

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED4_NODE, gpios);
static const struct gpio_dt_spec power_sw =
    GPIO_DT_SPEC_GET(POWER_SW_NODE, gpios);

static double process_sensor_value(const struct device *dev) {
    struct sensor_value force;
    int ret;

    ret = sensor_sample_fetch(dev);
    if (ret != 0) {
        LOG_ERR("ret: %d, Sensor sample update error", ret);
        return ret;
    }

    ret = sensor_channel_get(dev, SENSOR_CHAN_FORCE, &force);
    if (ret != 0) {
        LOG_ERR("ret: %d, Cannot read NAU7802 force channel.", ret);
        return ret;
    }

    return sensor_value_to_double(&force);
}

static void touch_isr_callback(const struct device *port,
                               struct gpio_callback *cb, uint32_t pins) {
    LOG_INF("Touch ISR.");
    lv_task_handler();
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
}

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

    // TODO: ADC calibration
    /*
      float32_t zero_offset = ZERO_OFFFSET;
      float32_t calib_factor = CALIBRATION_FACTOR;
      const struct sensor_value offset_attri;
      const struct sensor_value calib_attri;

      memcpy(&offset_attri.val1, &zero_offset, sizeof(zero_offset));
      memcpy(&calib_attri.val1, &calib_factor, sizeof(calib_factor));

      if (sensor_attr_set(adc_dev, SENSOR_CHAN_FORCE, SENSOR_ATTR_OFFSET,
                          &offset_attri) != 0) {
          LOG_ERR("Cannot configure the offset.");
          return 0;
      }

      if (sensor_attr_set(adc_dev, SENSOR_CHAN_FORCE, SENSOR_ATTR_CALIBRATION,
                          &calib_attri) != 0) {
          LOG_ERR("Cannot configure the calibration.");
          return 0;
      }
  */

    /* Initialize touch IRQ */
    /*
      if (gpio_is_ready_dt(&touch_irq)) {
          int err;

          err = gpio_pin_configure_dt(&touch_irq, GPIO_INPUT);
          if (err) {
              LOG_ERR("Failed to configure gpio for interrupt: %d", err);
              return 0;
          }

          gpio_init_callback(&touch_callback, touch_isr_callback,
                             BIT(touch_irq.pin));

          err = gpio_add_callback(touch_irq.port, &touch_callback);
          if (err) {
              LOG_ERR("Failed to add touch interrupt callback: %d", err);
              return 0;
          }
          err = gpio_pin_interrupt_configure_dt(&touch_irq, GPIO_INT_DISABLE);
          if (err) {
              LOG_ERR("Failed to enable touch interrupt callback: %d", err);
              return 0;
          }
      }
  */

    /* Initialize Load switch */
    if (gpio_is_ready_dt(&power_sw)) {
        int err;
        err = gpio_pin_configure_dt(&power_sw, GPIO_OUTPUT_ACTIVE);
    }

    /* Initialize LVGL */
    show_main_screen();
    lv_task_handler();
    k_msleep(100);
    display_blanking_off(display_dev);

    /* Set zero offset */
    double zero_offset = process_sensor_value(adc_dev);
    LOG_INF("Zero Offset: %d", zero_offset);

    while (1) {
        // LVGL test
        lv_task_handler();
        // ADC test
        double ret = process_sensor_value(adc_dev);
        // LOG_INF("%f, %f", ret, CALIBRATION_FACTOR * (ret - zero_offset)
        // - 4.0);
        // Load switch test
        gpio_pin_toggle_dt(&power_sw);
        // Sleep
        k_msleep(10);
    }
}
