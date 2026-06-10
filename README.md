# Zephyr RTOS IoT Sensor Node

Edge sensor node with Zephyr threads, Wi-Fi/BLE provisioning, power-mode profiling, encrypted telemetry, and signed OTA firmware updates.

## Portfolio Purpose

This repository is an Embedded Systems project scaffold for the Rheslar portfolio. It is designed to become a hardware-backed project with build output, validation logs, and reviewable implementation evidence.

## Stack

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
python -m unittest discover -s tests
```

## Implementation Slices

- Native starter executable that exposes the project identity, stack, and validation target.
- Architecture document with control boundaries, data flow, safety assumptions, and evidence plan.
- Unit smoke test that keeps source, docs, and CI files present as the repo grows.
- GitHub Actions workflow for configure, build, executable smoke run, and repository validation.

## Evidence Target

Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking.

## Remote

Intended public repository: https://github.com/rheslar1/zephyr-rtos-iot-sensor-node
