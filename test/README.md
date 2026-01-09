
This directory is intended for PlatformIO Test Runner and project tests.
# System Calibration & Validation
This directory contains experimental scripts used to validate and calibrate the BINSAI sensor array before final deployment.

## 1. MQ-135 Gas Sensor Calibration
To ensure accurate PPM (Parts Per Million) readings, the sensor's resistance in clean air ($R_0$) must be determined.
* **Script:** [`mq135_calibration.cpp`](test/mq135_calibration.cpp)
* **Methodology:** The sensor is pre-heated for 24 hours to stabilize the heating element before taking the $R_0$ average.

## 2. GPS & Ultrasonic Validation
Initial testing to verify NMEA sentence acquisition and ultrasonic wave propagation accuracy.
* **Script:** [`sensor_check.cpp`](test/sensor_check.cpp)
* **Accuracy Threshold:** Distance ±1cm, GPS Horizontal Dilution of Precision (HDOP) < 2.0.

More information about PlatformIO Unit Testing:
- https://docs.platformio.org/en/latest/advanced/unit-testing/index.html
