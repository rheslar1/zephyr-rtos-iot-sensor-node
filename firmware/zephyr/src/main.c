#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)

#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "nRF52840 DK LED0 alias is required"
#endif

struct sensor_sample {
  uint32_t uptime_ms;
  int16_t temperature_c_x10;
  uint16_t humidity_pct_x10;
  uint16_t battery_mv;
  int16_t acceleration_mg;
};

K_MSGQ_DEFINE(sample_queue, sizeof(struct sensor_sample), 8, 4);

static const struct gpio_dt_spec status_led =
    GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static void sensor_thread(void) {
  uint32_t sequence = 0U;

  while (true) {
    const struct sensor_sample sample = {
        .uptime_ms = (uint32_t)k_uptime_get_32(),
        .temperature_c_x10 = (int16_t)(246 + (int16_t)(sequence % 4U)),
        .humidity_pct_x10 = (uint16_t)(445 + (sequence % 10U)),
        .battery_mv = (uint16_t)(3670U - (sequence % 20U)),
        .acceleration_mg = (int16_t)(36 + (int16_t)(sequence % 8U)),
    };

    (void)k_msgq_put(&sample_queue, &sample, K_NO_WAIT);
    sequence++;
    k_sleep(K_SECONDS(60));
  }
}

static void telemetry_thread(void) {
  struct sensor_sample sample;

  while (true) {
    if (k_msgq_get(&sample_queue, &sample, K_FOREVER) == 0) {
      gpio_pin_toggle_dt(&status_led);
      printk("sensor_node uptime=%u temp_c_x10=%d humidity_pct_x10=%u "
             "battery_mv=%u accel_mg=%d\n",
             sample.uptime_ms,
             sample.temperature_c_x10,
             sample.humidity_pct_x10,
             sample.battery_mv,
             sample.acceleration_mg);
    }
  }
}

K_THREAD_DEFINE(sensor_tid, 1024, sensor_thread, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(telemetry_tid, 1536, telemetry_thread, NULL, NULL, NULL, 6, 0, 0);

int main(void) {
  int ret;

  if (!gpio_is_ready_dt(&status_led)) {
    printk("status LED is not ready\n");
    return 0;
  }

  ret = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
  if (ret != 0) {
    printk("failed to configure status LED: %d\n", ret);
    return 0;
  }

  ret = bt_enable(NULL);
  if (ret != 0) {
    printk("Bluetooth init failed: %d\n", ret);
  } else {
    printk("Bluetooth ready for BLE provisioning\n");
  }

  printk("Rheslar nRF52840 Zephyr sensor node started\n");
  return 0;
}
