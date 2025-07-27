[Русская версия](README.ru.md)

# ESP-01 UART to Server Bridge

This is a firmware for the ESP-01/ESP8266 module that acts as a bridge, reading data from the UART serial port and forwarding it to a remote server via an HTTP POST request.

It's designed for scenarios where you need to connect a device with a serial output (like an Arduino, a sensor, or any other microcontroller) to the internet without modifying its original firmware.

## Features

- Connects to a Wi-Fi network.
- Listens for incoming data on the UART RX pin.
- Forwards received data to a specified server endpoint.
- Configuration is kept separate from the code in a `secrets.h` file.

## Requirements

- PlatformIO IDE or Core.
- ESP-01 or ESP-01S module.
- A USB-to-TTL serial adapter for programming the ESP-01.

## Setup and Installation

1.  **Clone the repository:**
    ```bash
    git clone <your-repository-url>
    cd ESP-01-UARTtoSERVER
    ```

2.  **Configure your credentials:**
    -   Copy the example secrets file `src/secrets.h.example` to `src/secrets.h`.
        ```bash
        # On Windows
        copy src\secrets.h.example src\secrets.h
        # On Linux/macOS
        cp src/secrets.h.example src/secrets.h
        ```
    -   Open `src/secrets.h` and replace the placeholder values with your Wi-Fi SSID, password, and the server access key.

3.  **Build and Upload:**
    -   Open the project in PlatformIO.
    -   Build and upload the firmware to your ESP-01 module.

## Usage

1.  Connect the `TX` pin of your data source device to the `RX` pin of the ESP-01 module.
2.  Ensure both devices share a common ground (GND).
3.  Power on the devices.

The ESP-01 will automatically connect to the configured Wi-Fi network. Any data sent to its serial RX pin will be captured and sent to your server.

## License

This project is licensed under the MIT License.