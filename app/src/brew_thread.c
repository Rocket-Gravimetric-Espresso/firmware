#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "brew_thread.h"
#include "../../../modules/nau7802_loadcell/drivers/sensor/nau7802_loadcell/nau7802_loadcell.h"
#include "syscalls/sensor.h"
#include "zephyr/drivers/sensor.h"

LOG_MODULE_REGISTER(brew, LOG_LEVEL_INF);

#define STACK_SIZE 2048
#define PRIORITY   5

#define WEIGHT_LOWER	    80
#define WEIGHT_UPPER	    250
#define WEIGHT_TARGET_DELTA 38
#define CALIBRATION_FACTOR  0.002877658408
#define ZERO_OFFFSET	    0.0

#define LED4_NODE     DT_NODELABEL(led4)
#define POWER_SW_NODE DT_NODELABEL(power_sw)

static const struct gpio_dt_spec power_sw =
	GPIO_DT_SPEC_GET(POWER_SW_NODE, gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED4_NODE, gpios);
static const struct device *const adc_dev =
	DEVICE_DT_GET(DT_NODELABEL(nau7802));

int zero_offset;

static struct k_thread brew_thread_data;
K_THREAD_STACK_DEFINE(brew_stack_area, STACK_SIZE);

static double process_sensor_value(const struct device *dev)
{
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

static void start_brew(void)
{
	gpio_pin_set_dt(&power_sw, 1);
	gpio_pin_set_dt(&led, 1);
}

static void stop_brew(void)
{
	gpio_pin_set_dt(&power_sw, 0);
	gpio_pin_set_dt(&led, 0);
}

static void brew_thread_fn(void *p1, void *p2, void *p3)
{
	double offset = 0.0;
	double ret = process_sensor_value(adc_dev);
	LOG_INF("%f, %f", ret, CALIBRATION_FACTOR * (ret - zero_offset) - 4.0);
	LOG_INF("Waiting for cup weight %d–%d g...", WEIGHT_LOWER,
		WEIGHT_UPPER);

	while (true) {
		double weight = CALIBRATION_FACTOR *
				(process_sensor_value(adc_dev) - zero_offset);
		LOG_INF("Weight: %f g", weight);
		if (weight >= WEIGHT_LOWER && weight <= WEIGHT_UPPER) {
			LOG_INF("Offset weight: %f g", weight);
			k_sleep(K_MSEC(1000)); // settle
			double check_weight =
				CALIBRATION_FACTOR *
				(process_sensor_value(adc_dev) - zero_offset);
			if (check_weight > weight - 5 &&
			    check_weight < weight + 5) {
				offset = weight;
				break;
			}
		}
		k_sleep(K_MSEC(200));
	}

	start_brew();
	LOG_INF("Brewing started...");

	while (true) {
		double weight = CALIBRATION_FACTOR *
				(process_sensor_value(adc_dev) - zero_offset);
		if (weight >= offset + WEIGHT_TARGET_DELTA) {
			LOG_INF("Target reached: %f g (offset: %f g)", weight,
				offset);
			break;
		}
		k_sleep(K_MSEC(100));
	}

	stop_brew();
	LOG_INF("Brewing finished.");
}

void brew_thread_start(void)
{
	/* Initialize ADC sensor */
	if (!device_is_ready(adc_dev)) {
		LOG_ERR("Device %s is not ready. Exiting..", adc_dev->name);
		// return 0;
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

	  if (sensor_attr_set(adc_dev, SENSOR_CHAN_FORCE,
	  SENSOR_ATTR_CALIBRATION, &calib_attri) != 0) { LOG_ERR("Cannot
	  configure the calibration."); return 0;
	  }
      */

	/* Initialize Load switch */
	if (gpio_is_ready_dt(&power_sw)) {
		int err;
		err = gpio_pin_configure_dt(&power_sw, GPIO_OUTPUT_ACTIVE);
	}

	/* Initialize LED */
	if (gpio_is_ready_dt(&led)) {
		int err;
		err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	/* Set zero offset */
	zero_offset = process_sensor_value(adc_dev);
	LOG_INF("Zero Offset: %d", zero_offset);

	k_tid_t tid = k_thread_create(&brew_thread_data, brew_stack_area,
				      STACK_SIZE, brew_thread_fn, NULL, NULL,
				      NULL, PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(tid, "brew_thread");
}
