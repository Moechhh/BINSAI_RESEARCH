# BINSAI Private Libraries

This directory is reserved for project-specific local libraries developed for the BINSAI monitoring system. 

## Library Management Policy
To maintain research reproducibility and environment consistency:
1. **External Dependencies:** All third-party libraries (e.g., Blynk, TinyGPSPlus, MQUnifiedsensor) are strictly managed via `platformio.ini`.
2. **Local Modules:** This folder is used for modularizing BINSAI's custom algorithms if they exceed the scope of the main source files.

Current status: All logic is currently maintained within `src/` for centralized execution flow.
