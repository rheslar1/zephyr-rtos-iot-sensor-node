# Zephyr nRF52840 DK Firmware

This folder contains the on-target Zephyr application slice for the Nordic nRF52840 Development Kit.

Build from a Zephyr workspace:

```bash
west build -b nrf52840dk/nrf52840 firmware/zephyr
west flash
```

The host C++ implementation in the repository root is the CI-safe model of the same responsibilities: BLE provisioning gate, MCUboot metadata validation, sensor sampling, telemetry framing, and low-power evidence.
