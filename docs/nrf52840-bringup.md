# nRF52840 DK Bring-Up Notes

## Board Target

- Zephyr board target: `nrf52840dk/nrf52840`
- Board: Nordic nRF52840 Development Kit / PCA10056
- SoC: nRF52840 Cortex-M4F
- Memory profile: 256 KiB RAM, 1 MiB flash
- Native radios: BLE and IEEE 802.15.4
- Wi-Fi path: external backhaul module or gateway, provisioned over BLE

## Firmware Responsibilities

- Start independent sensor and telemetry threads.
- Use BLE as the provisioning and management plane.
- Require MCUboot-confirmed images before telemetry is accepted.
- Publish framed sensor telemetry only after endpoint and certificate material are provisioned.
- Record power evidence for active, radio, and sleep windows.

## Hardware Validation

1. Build and flash the Zephyr app with `west build -b nrf52840dk/nrf52840 firmware/zephyr`.
2. Confirm the serial log prints `Rheslar nRF52840 Zephyr sensor node started`.
3. Confirm LED0 toggles after each telemetry sample.
4. Bond over BLE and validate the management path before enabling field telemetry.
5. Measure average current across a 60-second sample interval and compare it with the host model budget.
