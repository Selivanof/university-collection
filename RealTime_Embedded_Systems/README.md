<div align="center">
<h3 align="center">Real-Time Embedded Systems</h3>

  <a href=""> ![Static Badge](https://img.shields.io/badge/Language-C%2B%2B-6866fb)</a>
  <a href=""> ![Static Badge](https://img.shields.io/badge/Semester-8-green)</a>
</div>

This repository contains the project that I completed for the Real-Time Embedded Systems 2023-2024 Course, as part of my undergraduate studies at Aristotle University of Thessaloniki.

|   Course Information     |                           |
|--------------------------|---------------------------|
| Semester                 | 8                         |
| Attendance Year          | 2023-2024                 |
| Project Type             | Mandatory                 |
| Team Project             | No                        |
| Language Used            | C++                       |

## Description

A WebSocket client that monitors and records symbol updates using Finnhub's WebSocket API.

## Cross-Compiling

This project involves cross-compiling static libraries for `aarch64` architecture, specifically ZLib and OpenSSL. The instructions below outline how to setup the toolchain and build these libraries. While this is the method I used, you can use any other prefered way to cross-compile them.

1. **Install the dependencies and toolchain**

- Download and install CMake
- Download the cross-compilation toolchain for `aarch64` from [ARM's website](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and install it in a directory (For the rest of the instructions, `/opt/aarch64` is assumed to be the installation directory)

2. **Export Toolchain Variables**

Set up the environment variables for the cross-compilation toolchain:

```sh
export SYSROOT=/opt/aarch64
export CROSS_PREFIX=$SYSROOT/bin/aarch64-none-linux-gnu-
```
3. **Cross-Compiling zlib (Static Linking)**

- Clone the latest version of zlib and configure it for cross-building with our toolchain
```sh
git clone https://github.com/madler/zlib.git
cd zlib
./configure --prefix=$SYSROOT/usr --static --arch=aarch64
```
- Build and install zlib to our `aarch64` sysroot
```sh
make
make install
```

4. **Cross-Compiling OpenSSL (Static Linking)**

- Clone the latest version of OpenSSL and configure it for cross-building with our toolchain
```sh
git clone https://github.com/openssl/openssl.git
cd openssl
./Configure linux-aarch64 no-shared \
  --openssldir=$SYSROOT/usr/ssl \
  --cross-compile-prefix=$CROSS_PREFIX \
  --prefix=$SYSROOT/usr \
  --with-zlib-include=$SYSROOT/usr/include \
  --with-zlib-lib=$SYSROOT/usr/lib
```
- Build and install OpenSSL to our `aarch64` sysroot
```sh
make
make install
```

5. **Cross-Compiling libwebsockets (Static Linking)**
- Clone the latest version of libwebsockets
```sh
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
```
- Create the toolchain file using this command
```sh
cat << EOF > toolchain-aarch64.cmake
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER "\$ENV{CROSS_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "\$ENV{CROSS_PREFIX}g++")

set(CMAKE_INSTALL_PREFIX "\$ENV{SYSROOT}/usr")
set(CMAKE_PREFIX_PATH "\$ENV{SYSROOT}/usr")

SET(CMAKE_FIND_ROOT_PATH "\$ENV{SYSROOT}")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
```

- Run CMake
```sh
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-aarch64.cmake \
         -DLWS_WITH_SSL=ON \
         -DLWS_WITH_STATIC=ON \
         -DLWS_WITHOUT_TESTAPPS=ON \
         -DBUILD_SHARED_LIBS=OFF
```

- Build and install
```sh
make
make install
```
6. **Cross-Compiling this project (Static Linking)**

Any libraries that do no require compilation should be automatically fetched by the provided `CMakeLists.txt`, while any precompiled libraries should be found automatically by CMake's `find_package`. Finally, to build this project:

```sh
cd /path/to/this/project
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

You should now have a binary of this project compiled for `aarch64` with all the necessary libraries stically linked!

## Setup

To start using this application, you must first follow these steps:

1. **Obtain an API Token**:  
   Visit [Finnhub](https://finnhub.io/) and sign up or log in to obtain your API token.

2. **Set the Environment Variable**:  
   Once you have your API token, set the environment variable `FINNHUB_API_TOKEN` with your token as the value.

   - For **Bash**, run the following command:
     ```bash
     export FINNHUB_API_TOKEN=your_api_token_here
     ```

   - For **Zsh**, run:
     ```zsh
     export FINNHUB_API_TOKEN=your_api_token_here
     ```

   - For **Fish shell**:
     ```fish
     set -x FINNHUB_API_TOKEN your_api_token_here
     ```

   - For **C Shell (csh/tcsh)**:
     ```csh
     setenv FINNHUB_API_TOKEN your_api_token_here
     ```

Once the environment variable is set, the application will be ready to connect to the Finnhub API.

## Configuration

When you run the program for the first time, a default `websocket_config.json` file will be created in the same directory as the executable. This configuration file can be modified to adjust various aspects of the application. Below is a breakdown of all the available fields:

| **Field**                      | **Type**             | **Description**                                                                                               | **Default**               |
|--------------------------------|----------------------|---------------------------------------------------------------------------------------------------------------|---------------------------|
| `pingIdleSec`                | `integer`            | The maximum time (in seconds) to wait on an idle WebSocket before sending a Ping message. | `30` seconds   |
| `pongTimeoutSec`                | `integer`            | The maximum time (in seconds) to wait for a Pong response from the WebSocket server before considering the connection lost. | `10` seconds   |
| `initialReconnectDelayMs`      | `integer`            | The initial delay (in milliseconds) before attempting to reconnect to the WebSocket server after a connection loss. | `10000` ms (10 seconds)   |
| `maxReconnectDelayMs`          | `integer`            | The maximum delay (in milliseconds) between reconnection attempts, used to implement an exponential backoff strategy. | `60000` ms (60 seconds)   |
| `maxReconnectionAttempts`        | `integer`            | The maximum number of reconnection attempts the application will make before giving up.                      | `1000` attempts           |
| `movingAverageTimeWindowMinutes`| `integer`            | The time window (in minutes) used to calculate the moving average for statistics.                            | `15` minutes              |
| `recordingWorkersCount`        | `integer`            | The number of worker threads allocated for recording data.                                                   | `2` workers               |
| `statisticsWorkersCount`       | `integer`            | The number of worker threads allocated for processing statistics.                                            | `1` worker                |
| `exportingWorkersCount`        | `integer`            | The number of worker threads allocated for exporting data.                                                   | `1` worker                |
| `recordingAffinities`          | `array of integers`  | The processor cores (identified by their core IDs) assigned to recording workers.                             | `[]` (no explicit affinity)|
| `statisticsAffinities`         | `array of integers`  | The processor cores (identified by their core IDs) assigned to statistics workers.                           | `[]` (no explicit affinity)|
| `exportingAffinities`          | `array of integers`  | The processor cores (identified by their core IDs) assigned to exporting statistics.                            | `[]` (no explicit affinity)|
| `symbols`                      | `array of strings`  | The list of stock symbols that the application will monitor. | `["NVDA", "TSLA", "AMZN", "MU", "MSFT", "AAPL"]` |

-  Setting `maxReconnectionAttempts` to 0 will disable the reconnection mechanism.
-  The client may automatically adjust or exclude any invalid values (excluding `symbols`). Check the reported configuration on startup.