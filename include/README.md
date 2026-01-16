# BINSAI System Configuration Headers

This directory contains the header files (`.h`) that define the global constants, pin mappings, and threshold parameters for the BINSAI monitoring system.

## Strategic Role
To ensure code maintainability and research scalability, all hardcoded values are abstracted into header files within this directory. This allows for rapid system reconfiguration without altering the core logic in the `src/` directory.

## Key Files
- `config.h`: Centralized definitions for GPIO pins (ESP32), sensor calibration constants (R0), and network credentials.
- `definitions.h`: Enumerations and structures for system states and data telemetry.

*Note: Avoid placing executable logic in this directory; it is strictly reserved for declarations and macro definitions.*
