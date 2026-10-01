PLC Modbus TCP UI

A Windows desktop application built with C++, Slint, and libmodbus for communicating with a PLC over Modbus TCP.

The application provides a graphical interface to monitor the PLC connection, display Modbus latency, monitor register status, and control specific register bits.

Features

Modbus TCP communication with a PLC

Automatic PLC reconnection

PLC online/offline status

Modbus response latency display

Register 2 Bit 2 monitoring

Register 0 Bit 0 control

Register 1 Bit 1 control

Automatic bit reset after 1 second

Slint-based graphical user interface

CMake build system

One-click build and run using build_run.bat

Project Structure
PLC_Modbus_UI/
├── .gitignore
├── CMakeLists.txt
├── build_run.bat
├── main.cpp
├── main.slint
└── README.md


The build/ directory is generated automatically and is not included in Git.

Requirements

The project is currently configured for Windows.

Required software:

Windows 10/11

Visual Studio 2022 with C++ development tools

CMake

Slint C++ 1.18.1

libmodbus 3.1.6

Git

PLC Configuration

The current PLC connection settings are defined in main.cpp:

const char *PLC_IP = "192.168.0.1";
const int PLC_PORT = 502;
const int SLAVE_ID = 1;


Change these values according to the PLC configuration.

For example:

const char *PLC_IP = "192.168.0.100";
const int PLC_PORT = 502;
const int SLAVE_ID = 1;

Build and Run

The easiest way to build and run the application is:

.\build_run.bat


The script:

Cleans the previous CMake build directory.

Configures the project using CMake.

Builds the Release version.

Runs the application.

The executable is generated under:

build\Release\PLC_Modbus.exe

Manual Build

If you want to build manually:

mkdir build
cd build
cmake ..
cmake --build . --config Release


Run the application:

.\Release\PLC_Modbus.exe

Modbus Operation
Register 0 Bit 0

When the first button is pressed:

Register 0 is read.

Bit 0 is set.

The value is written back to the PLC.

The UI displays the bit as ON.

After 1 second, Register 0 is read again.

Bit 0 is cleared.

The updated value is written back to the PLC.

Register 1 Bit 1

When the second button is pressed:

Register 1 is read.

Bit 1 is set.

The value is written back to the PLC.

The UI displays the bit as ON.

After 1 second, Register 1 is read again.

Bit 1 is cleared.

The updated value is written back to the PLC.

Register 2 Bit 2

Register 2 is continuously monitored.

The application periodically reads Register 2 and displays the state of Bit 2:

ON
OFF


The Modbus response time is also displayed in milliseconds.

PLC Connection

The application automatically attempts to reconnect if the PLC connection is lost.

The UI displays:

PLC ONLINE


when communication succeeds and:

PLC OFFLINE


when communication fails.

Git

Clone the project:

git clone https://github.com/vaishnavi0211-hiremath/PLC_Modbus_UI.git


Enter the project directory:

cd PLC_Modbus_UI


Build and run:

.\build_run.bat

Author

vaishnavi0211-hiremath

GitHub:

https://github.com/vaishnavi0211-hiremath

License

This project is provided for educational and development purposes.
