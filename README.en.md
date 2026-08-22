# Spark
[![License](https://img.shields.io/badge/License-BSD--4--Clause-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/Language-C++20+-orange.svg)]()
[![Build](https://img.shields.io/badge/Build-CMake3.20+-green.svg)]()

**Spark** is a cross-platform C++ general-purpose foundational library designed for **financial trading systems and risk management systems**. It integrates high-performance logging, multi-model network communication, protocol serialization, high-performance data structures, and utility components. It supports Linux / Windows platforms and can accelerate financial backend service development.

Created by [Fireseeker](https://fireseeker.cn/)

## 1. Project Overview

This is a personal open-source foundational component library focused on encapsulating low-level common capabilities for financial scenarios. The library is developed in standard C++20, uses CMake for cross-platform building, includes Python scripts for automatic code generation, and comes with a comprehensive Google Test-based unit test suite. It is suitable for learning, secondary development, and integration into small-to-medium financial business systems.

## 2. Core Features

The library is divided into four core modules plus utility scripts:

### 2.1 Core Module

Provides low-level system capabilities with cross-platform compatibility:
- **Logger**: High-performance asynchronous logging system supporting log levels, file rolling, and thread-safe writing
- **MD5**: Standard MD5 encryption algorithm implementation
- **Platform**: Cross-platform directory, file, and system interface wrappers
- **Thread**: Basic thread encapsulation supporting start/stop, synchronization, and management
- **Timer**: General-purpose timer component
- **Utility**: Collection of common utility functions (Double precision comparison, time utilities, etc.)
- **Aspect**: AOP (Aspect-Oriented Programming) support for logging aspects and performance monitoring
- **ConfigStructs**: Common configuration structs (time zone, IP address, subscribed instruments, etc.)

### 2.2 Network Module

Adapts to mainstream I/O models, supports protocol communication and shared memory interaction, optimized for low-latency financial scenarios:
- **IO**: Unified I/O abstraction interface masking platform differences; multiplexing models: `Select` (general), `Epoll` (Linux), `IOCP` (Windows)
- **Tcp**: TCP communication wrappers with full client/server support for Select / Epoll / IOCP models
- **Protocol**: Financial protocol support — Step protocol (text-based tag-value) and XTP protocol (binary high-performance); includes package serialization (`Package::MakePackage`), package parsing (`PackageReader`), buffer management, and checksum calculation
- **Shm**: Shared memory communication for high-speed inter-process data exchange

### 2.3 Serialization Module

Data encoding/decoding, format parsing, and character encoding conversion:
- **JSON**: JSON parsing and serialization based on JsonCpp
- **Base64**: Base64 encoding/decoding utility
- **CSV**: Lightweight CSV file parser (CsvParser + CsvRecord)
- **Encode**: Character encoding conversion (GBK / UTF-8 / Unicode)

### 2.4 Template Library (TemplateLib)

High-performance data structures and reusable business components:
- **Buffer**: Generic buffer
- **RingBuffer**: Ring buffer (lock-free and locked variants)
- **ShmBuffer**: Shared memory buffer
- **ObjectPool**: Object pool to reduce frequent memory allocation overhead
- **LockFreeQueue**: Lock-free queue
- **ThreadSafeList**: Thread-safe linked list container

### 2.5 Utility Scripts

Built-in Python scripts for automation tasks such as code parsing, model generation, and batch processing.

## 3. Project Directory Structure

```
Spark/
├── include/                    # Public headers
│   ├── Spark/Core/             # Core module headers
│   ├── Spark/Network/          # Network module headers
│   ├── Spark/Serialization/    # Serialization module headers
│   └── Spark/TemplateLib/      # TemplateLib module headers
├── src/                        # Source code
│   ├── Core/                   # Core module implementations
│   ├── Network/                # Network module implementations
│   └── Serialization/          # Serialization module implementations
├── test/                       # Test programs
│   ├── unittest/               # GTest unit tests (primary test entry)
│   │   ├── Core/               # Core module tests (9 files)
│   │   ├── Network/            # Network module tests (4 files)
│   │   ├── Serialization/      # Serialization module tests (4 files)
│   │   ├── TemplateLib/        # TemplateLib module tests (6 files)
│   │   └── CMakeLists.txt      # Unit test build configuration
│   ├── TestCommon/             # Shared test library (Package factory, subscribers, etc.)
│   ├── Packages/               # Test package model definitions & generation
│   ├── TestClient/             # Network client tests (legacy)
│   ├── TestServer/             # Network server tests (legacy)
│   ├── TestCore/               # Core component tests (legacy)
│   └── TestMD5/                # MD5 verification program
├── model/                      # Data model definitions (Head.xml / XtpHead.xml, etc.)
├── submodules/                 # Submodule dependencies (CMakeCommon)
├── bin/                        # Build outputs: dynamic libraries / executables (per config)
├── lib/                        # Build outputs: static libraries / import libraries (per config)
├── out/                        # CMake Presets build directory
├── .workflow/                  # CI pipeline configuration (GCC build)
├── CMakeLists.txt              # CMake main build configuration
├── CMakePresets.json           # CMake presets (VS / CLI)
├── *.py                        # Python automation scripts
├── UpdateSubmodule.bat/sh      # Submodule update scripts
├── Install.sh                  # Linux install script (cmake --install)
├── .gitmodules                 # Git submodule configuration
├── .gitignore                  # Git ignore rules
└── LICENSE                     # BSD-4-Clause license
```

## 4. Environment Dependencies

### Prerequisites

- C++ compiler supporting **C++20 or later** (GCC, Clang, MSVC)
- Build tool: **CMake 3.20+**
- Script runtime: **Python 3.6+** (only for code generation scripts, not a runtime dependency)
- Test framework: **Google Test** (auto-detected by CMake; must be installed via vcpkg or system package manager)
- Platform: Linux, Windows

### Submodule Dependencies

The project depends on the `CMakeCommon` submodule. After cloning, you need to synchronize and pull the submodule.

> For detailed environment setup steps (proxy configuration, vcpkg installation, WSL mirrored networking, etc.), see the [Environment Setup Guide](docs/environment-setup.en.md).

## 5. Quick Build & Compilation

### 5.1 Clone Repository (with Submodules)

```bash
git clone --recursive https://gitee.com/xunmeng200/Spark.git
cd Spark
```

### 5.2 Update Submodules (if not cloned recursively)

```bash
# Linux / Mac
sh UpdateSubmodule.sh

# Windows
UpdateSubmodule.bat
```

### 5.3 CMake Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

> **Tip**: The project ships `CMakePresets.json`. You can also build via presets (recommended):
>
> ```bash
> cmake --preset x64-Release          # Windows (MSVC)
> cmake --build out/build/x64-Release
> ```
>
> On Linux / WSL, use the `WSL-GCC-Debug` / `WSL-GCC-Release` presets instead.

After compilation, library files (Core / Network / Serialization / TemplateLib) are output to `lib/<Config>`, and executables (UnitTests, Test*, etc.) are output to `bin/<Config>` (e.g., Release configuration produces `bin/Release`).

### 5.4 Run Unit Tests

```bash
cd build
ctest --output-on-failure
# or run the built executable directly
./bin/Release/UnitTests
```

## 6. Basic Usage Examples

### 6.1 High-Performance Logging

```cpp
#include <Spark/Core/Logger/Logger.h>

using namespace spark::core;

int main(int argc, const char* argv[])
{
    // Initialize the logger with the process name, set levels, and start the logging thread
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    // Level-based logging (printf-style formatting)
    WriteLog(LogLevel::Info, "Application started successfully");
    WriteLog(LogLevel::Debug, "Debug message: system init done");
    WriteLog(LogLevel::Error, "Demo running, error code:[%d]", 1001);

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    return 0;
}
```

### 6.2 JSON Serialization & Parsing

```cpp
#include <Spark/Serialization/json/json.h>
#include <iostream>
#include <memory>
#include <string>

int main()
{
    // Build a JSON object
    Json::Value root;
    root["order_id"] = "20260615001";
    root["price"] = 123.45;
    root["volume"] = 1000;
    root["is_buy"] = true;

    // Serialize: JSON object -> string
    Json::StreamWriterBuilder writerBuilder;
    std::string jsonStr = Json::writeString(writerBuilder, root);
    std::cout << "JSON String: " << jsonStr << std::endl;

    // Deserialize: string -> JSON object
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    Json::Value parsed;
    std::string errs;
    bool ok = reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.size(), &parsed, &errs);
    if (ok)
    {
        std::cout << "Parsed price: " << parsed["price"].asDouble() << std::endl;
    }

    return 0;
}
```

### 6.3 Network Communication (Step Protocol Client)

```cpp
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/Network/Protocol/ProtocolSubscriber.h>
#include <Spark/Network/Protocol/PackageFactoryBase.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <cstring>

using namespace spark;
using namespace spark::core;
using namespace spark::network;

// Package factory: creates the corresponding package object by package ID
// (implementation omitted here; see test/Packages/PackageFactory.cpp)
class MyPackageFactory : public PackageFactoryBase
{
public:
    virtual Package* CreatePackage(UShortType packageID) override;
};

// Step protocol client: inherit Protocol and implement ProtocolSubscriber callbacks
class MyStepClient : public Protocol, public ProtocolSubscriber
{
public:
    MyStepClient()
        : Protocol(ProtocolTypeType::Step, ServerTypeType::Client,
                   IOModelType::Select, 0, new MyPackageFactory())
    {
        m_ReqInsertOrder = new ReqInsertOrderPackage(); // auto-generated from the model (see test/Packages)
        Subscribe(this);                        // Register self as message subscriber
        RegisterFront("tcp://127.0.0.1:20001"); // Connect to server address
        // Shared memory format: RegisterFront("shm://TestShm:4");  // "shm://" + serviceName + ":" + maxConnections
    }

    // Connection established callback
    void OnProtocolConnect(SessionIDType sessionID, const char* ip, int port) override
    {
        WriteLog(LogLevel::Info, "OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);
        SendReqInsertOrder();
    }

    // Connection disconnected callback
    void OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port) override
    {
        WriteLog(LogLevel::Info, "OnDisConnect SessionID:[%lld]", sessionID);
    }

    // Incoming message callback
    void OnMessage(Package* package) override
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
        SendReqInsertOrder(); // Send an order after receiving a message
    }

    // Build and send a buy-to-open order
    void SendReqInsertOrder()
    {
        m_ReqInsertOrder->Prepare(m_SessionID, false, ++m_MessageSeqNum);
        m_ReqInsertOrder->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
        memset(m_ReqInsertOrder->ReqInsertOrder, 0, sizeof(ReqInsertOrderField));
        Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->AccountID, "Xunmeng001");
        Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->ExchangeID, "SHSE");
        Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->InstrumentID, "600036");
        m_ReqInsertOrder->ReqInsertOrder->Direction = DirectionType::Buy;
        m_ReqInsertOrder->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
        m_ReqInsertOrder->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
        m_ReqInsertOrder->ReqInsertOrder->Price = 100.5;
        m_ReqInsertOrder->ReqInsertOrder->Volume = 1000;
        Send(m_ReqInsertOrder);
        m_ReqInsertOrder->Deallocate();
    }

private:
    SessionIDType m_SessionID = 0LL;
    int m_MessageSeqNum = 0;
    ReqInsertOrderPackage* m_ReqInsertOrder;
};

int main(int argc, const char* argv[])
{
    // Initialize logger
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    // Create IO thread and start the client
    IOThread* ioThread = new IOThread("StepClient");
    MyStepClient client;
    client.SetIOThread(ioThread);
    if (!client.Init())
        return -1;

    ioThread->Start();      // Start the event loop (blocks current thread)
    ioThread->Join();

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    return 0;
}
```

> **Note**: Package model classes such as `ReqInsertOrderPackage` / `ReqInsertOrderField` are auto-generated from the model (see `test/Packages/`). For the complete usage of callbacks and the package factory, see `test/TestClient/TestStepClient.cpp`.

## 7. Unit Tests

The project includes a comprehensive **Google Test**-based unit test suite with **23 test files** across four modules:

| Module | Test Files | Coverage |
| --- | --- | --- |
| **Core** | `AspectTest` | Logging & performance aspects |
| | `DirTest` | Directory creation, deletion, traversal |
| | `DoubleUtilityTest` | Floating-point precision comparison |
| | `LoggerTest` | Log levels, file rolling, thread safety |
| | `MD5Test` | MD5 hash correctness |
| | `ThreadBaseTest` | Thread start/stop, synchronization |
| | `TimerTest` | Timer firing and cancellation |
| | `TimeUtilityTest` | Time formatting and conversion |
| | `UtilityTest` | General utility functions |
| **Network** | `StepUtilityTest` | Step protocol field parsing, Head/Tail stream conversion (36 cases) |
| | `ProtocolUtilityTest` | CHECKSUM calculation (8 cases) |
| | `PackageReaderTest` | Buffer management: Append/PopFront/Shift/Reset (14 cases) |
| | `PackageSerializationTest` | End-to-end MakePackage ↔ ParsePackage round-trip (6 cases) |
| **Serialization** | `Base64Test` | Base64 encoding/decoding |
| | `CSVParserTest` | CSV row/column parsing, quote escaping |
| | `CSVRecordTest` | CSV record reading/writing |
| | `EncodeTest` | GBK/UTF-8/Unicode conversion |
| **TemplateLib** | `BufferTest` | Buffer read/write and expansion |
| | `LockFreeQueueTest` | Lock-free queue push/pop |
| | `ObjectPoolTest` | Object pool allocation and reuse |
| | `RingBufferTest` | Ring buffer read/write wrap-around |
| | `ShmBufferTest` | Shared memory buffer |
| | `ThreadSafeListTest` | Thread-safe insert/delete/traverse |

### Running All Tests

```bash
cd build
ctest
```

Or run `bin/<Config>/UnitTests` directly for detailed console output (e.g., `bin/Release/UnitTests` for Release builds).

## 8. Script Reference

Python scripts in the root directory are used for automated code processing:

| Script | Description |
|--------|-------------|
| pump.py / pumpall.py | Template-based code generation engine (driven by pumplist.xml / parselist.xml) |
| pumptemp.py | Generation for specific templates (e.g., Types.h / EnumString.h) |
| ParsePackageModel.py / ParseTableModel.py | Package model / table model parsing |
| ParseShortField.py / ParseShortItem.py | Short field / short item parsing |
| parseall.py | Batch parsing entry point |
| ConvertToUtf8Bom.py | Normalize file encoding to UTF-8 BOM |
| clearall.py | Temporary file cleanup |

## 9. License & Disclaimer

- **License**: BSD-4-Clause. See the [LICENSE](LICENSE) file for details.
- **Scope**: This project is intended solely for personal learning and research purposes.
- **Disclaimer**: This is a personal open-source project. For production use, please conduct thorough testing and evaluate risks.

## 10. Additional Notes

- **Cross-platform differences**: `Epoll` is only supported on Linux, `IOCP` only on Windows. The code has been adapted accordingly. `Select` is the general model and works on both platforms.
- **Financial protocols**: Built-in wrappers for Step (text-based tag-value, commonly used in futures CTP protocol family) and XTP (binary high-performance) protocols, ready to connect to corresponding market/trading interfaces.
- **Extensibility**: Custom protocols, data structures, and business components can be added based on the existing modules.
- **Include path style**: Headers use `#include <Spark/Module/HeaderName.h>` convention.
