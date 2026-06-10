# Zephyr RTOS IoT Sensor Node

Edge sensor node with Zephyr threads, Wi-Fi/BLE provisioning, power-mode profiling, encrypted telemetry, and signed OTA firmware updates.

## Portfolio Purpose

This repository implements a Zephyr-style IoT sensor node for the Nordic nRF52840 Development Kit. The root C++17 model is host-buildable for CI and mirrors the firmware responsibilities: BLE provisioning, MCUboot image validation, sensor sampling, secure telemetry framing, and low-power evidence. The `firmware/zephyr` folder contains the on-target Zephyr application slice for `nrf52840dk/nrf52840`.

## Stack

- C++17
- C++ Design Patterns
- SOLID
- Zephyr RTOS
- BLE
- Wi-Fi
- MCUboot
- TLS
- Power profiling

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/zephyr_rtos_iot_sensor_node
ctest --test-dir build --output-on-failure
```

Simulator failure modes:

```bash
./build/zephyr_rtos_iot_sensor_node --low-battery
./build/zephyr_rtos_iot_sensor_node --offline
./build/zephyr_rtos_iot_sensor_node --bad-cert
./build/zephyr_rtos_iot_sensor_node --tight-power
./build/zephyr_rtos_iot_sensor_node --unconfirmed-image
```

To build the Zephyr firmware from a Zephyr workspace:

```bash
west build -b nrf52840dk/nrf52840 firmware/zephyr
west flash
```

## Implementation Slices

- nRF52840 DK board profile using the Zephyr `nrf52840dk/nrf52840` target.
- BLE provisioning gate with TLS endpoint and certificate fingerprint validation.
- Node configuration gate for device identity, firmware version, sampling period, battery threshold, and power budget.
- MCUboot running-image validation with signature, confirmation, rollback counter, and slot-size checks.
- Sensor thread model with battery, temperature, humidity, pressure, and acceleration samples.
- Telemetry thread model that emits compact QoS1 frames for secure backhaul.
- nRF52840 power profiler estimating active, radio, and low-power sleep windows.
- On-target Zephyr app skeleton with sensor and telemetry threads, BLE enablement, and LED heartbeat.
- CTest coverage for accepted cycles, unsupported boards, unsigned images, Wi-Fi assumptions, low battery, telemetry failure, and power evidence.

## Evidence Target

Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking.

## Remote

Intended public repository: https://github.com/rheslar1/zephyr-rtos-iot-sensor-node

<!-- cpp17-solid-implementation:start -->
## C++17, Design Patterns, and SOLID Implementation

This repository includes a host-buildable C++17 implementation, not only documentation. The implementation applies:

- Strategy pattern for validation rules.
- Adapter interfaces for input samples and telemetry/reporting.
- Composite validation for combining safety and readiness checks.
- Facade orchestration through the project runtime class.
- SOLID boundaries between profile data, input acquisition, validation, telemetry encoding, and tests.
<!-- cpp17-solid-implementation:end -->

<!-- deep-architecture-links:start -->
## Deep Architecture and UML

- [Deep architecture](docs/deep-architecture.md)
- [Full UML Draw.io source](docs/diagrams/full-system-uml.drawio)
- [Full UML PNG export](docs/diagrams/full-system-uml.png)
<!-- deep-architecture-links:end -->
