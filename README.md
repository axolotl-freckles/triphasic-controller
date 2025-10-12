# Trifasic inversor controller for ESP32

This project contains the basic modules for a three phase wave generator using
PWMs. The intended use is to be paired with a driver circuit like an inversor or
three phase AC motor driver.

The purpose of this project is to provide a simple interface to control the
parameters (frequency and amplitude) of the generated wave and to obtain
readings from sensors in real time via an API.

## How to use notes

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

If you want to test the latest changes, the `dev` branch has the up-to-date
latest changes.

### Compilation

The project was made using the Esspressif toolchain managed by the VScode
extension, so the requirements are written taking into account this IDE
exclusively.

In order to compile the project, it is necessary to go to the `idf.py menuconfig`
and turn on the
`Component config -> ESP Timer (High Resolution Timer) -> Suppor ISR dispatch method`
option.

## How to use example (gide from the ESP project template)

Follow detailed instructions provided specifically for this example.

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)

## Example folder contents

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```txt
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

- Program upload failure

	- Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
	- The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

For suppor regarding the controller code it is highly suggested that you raise
a github issue in this repo.

You can alternatively mail <aavaloscorrales@gmail.com> directly but be warned
that the response time will be quite random.

For ESP related questions please use the following feedback channels:

- For technical queries, go to the [esp32.com](https://esp32.com/) forum
- For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)
