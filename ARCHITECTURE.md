# Zephyr RTOS IoT Sensor Node Architecture

## Goal

Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking.

## Runtime Shape

1. The nRF52840 DK boots a confirmed MCUboot image.
2. Node configuration is validated for identity, firmware version, sampling period, battery threshold, and average-current budget.
3. BLE provisioning supplies the secure endpoint, certificate fingerprint, and optional external Wi-Fi backhaul profile.
4. A sensor thread samples battery, environmental, and motion data.
5. A telemetry thread frames the latest sample for secure MQTT/HTTPS backhaul.
6. A power profiler records active, radio, and sleep windows against the node budget.
7. Validation logs are emitted for repeatable review in CI and during hardware bring-up.

## C++17 Design Shape

- `ZephyrSensorNode` owns the cycle-level orchestration.
- `ISensorReader`, `IProvisioningStore`, `ITelemetryTransport`, `IUpdateValidator`, and `IPowerProfiler` isolate hardware and network dependencies.
- `McubootUpdateValidator` enforces signature, confirmation, rollback, and slot-size rules.
- `Nrf52840PowerProfiler` models current draw for active work, radio transmission, and low-power sleep.
- `ScriptedSensorReader`, `StaticProvisioningStore`, and `CapturingTelemetryTransport` make CI deterministic without a Zephyr SDK.

## SOLID Notes

- Single Responsibility: board profile, provisioning, sensor reads, telemetry, update checks, and power evidence are separated.
- Open/Closed: real Zephyr adapters can replace host simulators without changing orchestration.
- Liskov Substitution: simulator and hardware implementations share focused interfaces.
- Interface Segregation: each runtime capability exposes only the methods the node needs.
- Dependency Inversion: the node depends on abstractions instead of concrete drivers or transports.

## Boundaries

- `include/sensor_node/`: host-testable domain model and interfaces.
- `src/`: C++17 orchestration, simulators, power model, and CLI demo.
- `firmware/zephyr/`: Zephyr RTOS application slice for `nrf52840dk/nrf52840`.
- `docs/`: validation plans, timing notes, hardware captures, and acceptance evidence.
- `tests/`: host-side cycle tests for safety gates and evidence.
- `.github/workflows/`: CI entry point for build and validation evidence.

## Validation Plan

- Build the host model with CMake.
- Run the executable and confirm the nRF52840 cycle passes with power evidence.
- Run CLI failure modes for low battery, offline telemetry, invalid certificate material, tight power budget, and unconfirmed images.
- Run CTest to validate board, provisioning, MCUboot, sensor, telemetry, and power gates.
- Build and flash `firmware/zephyr` from a Zephyr workspace for the nRF52840 DK.
- Add serial logs, BLE provisioning captures, and current measurements after hardware bring-up.
- Capture CI, terminal, and hardware evidence for the portfolio detail page.

## Expansion Notes

- Replace host interfaces with concrete Zephyr drivers for sensors, settings/NVS, TLS sockets, MCUmgr, and PM hooks.
- Add OpenThread support if the deployment uses the nRF52840 IEEE 802.15.4 radio.
- Keep Wi-Fi as an external backhaul path because the nRF52840 DK does not provide native Wi-Fi.

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
