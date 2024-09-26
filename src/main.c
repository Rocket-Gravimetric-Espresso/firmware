#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rocket, LOG_LEVEL_INF);

int main(void) {
    LOG_INF("Firmware running.");

    while (1) {
    }
    return 0;
}
