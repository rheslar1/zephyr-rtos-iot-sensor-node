# Validation Plan

## Current Host Checks

- CMake configure completes.
- C++17 host model builds.
- Executable prints the nRF52840 DK target, BLE/external Wi-Fi backhaul posture, and one accepted sensor cycle.
- CLI failure modes exercise low battery, offline telemetry, invalid certificate material, tight power budget, and unconfirmed image handling.
- CTest verifies unsupported board rejection, node configuration validation, MCUboot image rejection, BLE provisioning, no-native-Wi-Fi handling, low battery rejection, telemetry transport failure, JSON-safe telemetry, power-budget rejection, and power evidence.
- GitHub Actions runs configure, build, executable smoke run, and CTest.

## Hardware Evidence To Add

- nRF52840 DK setup photo with USB/J-Link and any sensor wiring visible.
- Serial output from `west flash` and the first Zephyr boot.
- BLE provisioning capture showing device identity and endpoint material.
- Current profile across a 60-second sensor interval.
- MCUboot image confirmation and rollback-counter evidence.
- CI screenshot after the public repository is pushed.

## Project-Specific Evidence Target

Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking.

## Zephyr Hardware Check

```bash
west build -b nrf52840dk/nrf52840 firmware/zephyr
west flash
```

Expected serial markers:

```text
Bluetooth ready for BLE provisioning
Rheslar nRF52840 Zephyr sensor node started
sensor_node uptime=...
```

## Host Failure Modes

```bash
./build/zephyr_rtos_iot_sensor_node --low-battery
./build/zephyr_rtos_iot_sensor_node --offline
./build/zephyr_rtos_iot_sensor_node --bad-cert
./build/zephyr_rtos_iot_sensor_node --tight-power
./build/zephyr_rtos_iot_sensor_node --unconfirmed-image
```
