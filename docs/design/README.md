# Zephyr RTOS IoT Sensor Node Design Package

## Purpose

Edge sensor node with Zephyr threads, Wi-Fi/BLE provisioning, power-mode profiling, encrypted telemetry, and signed OTA firmware updates.

This package defines the project as an implementation-ready embedded system. It covers system architecture, requirements, interface boundaries, runtime design, validation evidence, and phased delivery.

## Project Profile

| Field | Value |
| --- | --- |
| Repository | `rheslar1/zephyr-rtos-iot-sensor-node` |
| Primary stack | C++17, C++ Design Patterns, SOLID, Zephyr RTOS, BLE, Wi-Fi, MCUboot, TLS, Power profiling |
| Review proof point | Real concurrency, secure update flow, low-power design, and production-grade device lifecycle thinking. |

## Artifacts

- [System Design](system-design.md)
- [Requirements](requirements.md)
- [Interface Control](interface-control.md)
- [Runtime Design](runtime-design.md)
- [Validation Plan](validation-plan.md)
- [Implementation Roadmap](implementation-roadmap.md)
- [Draw.io UML](diagrams/system-design.drawio)
- [PNG UML](diagrams/system-design.png)
