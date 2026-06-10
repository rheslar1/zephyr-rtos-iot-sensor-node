#include <stdio.h>
#include <stddef.h>

typedef struct {
  const char *title;
  const char *summary;
  const char *evidence_target;
  const char *tags[8];
  size_t tag_count;
} project_profile_t;

static const project_profile_t profile = {
  "Zephyr RTOS IoT Sensor Node",
  "Edge sensor node with Zephyr threads, Wi-Fi/BLE provisioning, power-mode profiling, encrypted telemetry, and signed OTA firmware updates.",
  "Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking.",
  {
  "Zephyr RTOS",
  "BLE",
  "Wi-Fi",
  "MCUboot",
  "TLS",
  "Power profiling"
  },
  6u
};

int main(void) {
  printf("%s\n", profile.title);
  printf("Summary: %s\n", profile.summary);
  printf("Evidence target: %s\n", profile.evidence_target);
  printf("Stack:");

  for (size_t index = 0; index < profile.tag_count; ++index) {
    printf(" %s%s", profile.tags[index], index + 1u == profile.tag_count ? "" : ",");
  }

  printf("\n");
  return 0;
}
