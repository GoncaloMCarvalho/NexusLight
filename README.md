# NexusLight 🌈

**NexusLight** is a robust ESP32-based firmware designed to control RGB LED strips via a custom Android companion app. This project focuses on connecting the physical and digital worlds through seamless hardware-software integration, providing a highly responsive and synchronized user experience.

## Features

- **FreeRTOS Dual-Core Architecture:** 
  - **Core 0:** Dedicated exclusively to Bluetooth Classic communication, handling incoming serial data and state machine logic without blocking the hardware.
  - **Core 1:** Manages real-time PWM signal generation and executing non-blocking LED animations.
- **Fail-Safe State Management:** The firmware ensures the hardware boots in a safe `OFF` state and perfectly synchronizes with the mobile app upon connection, recovering the last known RGB values.
- **Dynamic Lighting Modes:**
  - `Manual (RGB)`: Precise, zero-latency color mixing via app sliders.
  - `Smooth Cycle`: A continuous sine-wave color transition algorithm.
  - `Party`: High-energy randomized strobing pattern.

## Hardware Setup

To replicate this project, you will need an ESP32, an RGB LED Strip, and three N-Channel MOSFETs to handle the current load.

| Channel | ESP32 Pin | Component |
| :--- | :--- | :--- |
| **Red** | `GPIO 25` | IRL3705N MOSFET Gate (via 220Ω resistor) |
| **Green** | `GPIO 26` | IRL3705N MOSFET Gate (via 220Ω resistor) |
| **Blue** | `GPIO 27` | IRL3705N MOSFET Gate (via 220Ω resistor) |

> **Hardware Note:** The **IRL3705N** is a logic-level N-channel MOSFET, critical for this project since it can be fully saturated by the ESP32's 3.3V logic outputs, avoiding overheating. A **220Ω resistor** must be placed in series between each ESP32 GPIO pin and the corresponding MOSFET gate to limit inrush current and protect the microcontroller during high-frequency PWM switching. Ensure the MOSFET source pins are connected to a common ground with the ESP32, and the LED strip receives its appropriate external power supply (e.g., 12V).

## Bluetooth Communication Protocol

The ESP32 acts as a Bluetooth Classic Server (named `NexusLight`). It listens for the following newline-terminated (`\n`) string commands sent from the client application:

| Command | Action |
| :--- | :--- |
| `TOGGLE` | Toggles the LED strip between the `OFF` state and the `Manual` (last RGB) state. |
| `M-1` | Forces a hard `OFF` state (typically triggered automatically upon app connection for safety). |
| `FESTA` | Activates the Party animation mode. |
| `CICLO` | Activates the Smooth Cycle animation mode. |
| `R<0-255>` | Updates the Red channel PWM duty cycle (e.g., `R255`). |
| `G<0-255>` | Updates the Green channel PWM duty cycle (e.g., `G128`). |
| `B<0-255>` | Updates the Blue channel PWM duty cycle (e.g., `B0`). |

## Software Dependencies & Build

This project is built using the Arduino framework for the ESP32. 
To build and flash the project:
1. Ensure you have the [ESP32 core for Arduino](https://github.com/espressif/arduino-esp32) installed in your IDE (VS Code with PlatformIO or Arduino IDE).
2. Include the built-in `BluetoothSerial.h` library.
3. Compile and upload to your board at a baud rate of `115200`.

## Mobile Application

The user interface was built using **MIT App Inventor**. The application's internal state machine mirrors the C++ logic to ensure complete synchronization and prevent out-of-sync states between the client and the hardware.

### How to Import the Project:
1. Navigate to the `mobile_app/` directory in this repository and download the `NexusLight.aia` file.
2. Open [MIT App Inventor](http://appinventor.mit.edu/).
3. Click on **Projects** -> **Import project (.aia) from my computer** and select the downloaded file.

*(Optional)* If you just want to install and run the app directly on your Android device without editing the blocks, you can download and install the pre-compiled `NexusLight.apk` located in the same folder.