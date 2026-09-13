# Spark
[![License](https://img.shields.io/badge/License-BSD--4--Clause-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/Language-C++20+-orange.svg)]()
[![Build](https://img.shields.io/badge/Build-CMake3.20+-green.svg)]()

**Spark** 是一套面向**金融交易系统、风险管理系统**设计的跨平台 C++ 通用基础库，集成高性能日志、多模型网络通信、协议序列化、高性能数据结构与工具组件等常用能力，适配 Linux / Windows 双平台，可快速落地金融后端服务开发。

Created by [Fireseeker](https://fireseeker.cn/)

## 一、项目概述

本项目为个人开源基础组件库，聚焦金融场景下的底层通用能力封装，规避重复造轮子。库基于标准 C++20 开发，采用 CMake 跨平台构建，配套 Python 脚本实现代码自动生成，内置基于 Google Test 的完整单元测试套件，适合学习、二次开发及中小型金融业务系统集成。

## 二、核心功能模块

整体分为四大核心模块 + 工具脚本，覆盖后端开发高频场景：

### 1. 核心基础模块（Core）

提供系统底层通用能力，跨平台兼容：
- **Logger**：高性能异步日志系统，支持日志分级、文件滚动、多线程安全写入
- **MD5**：标准 MD5 加密算法实现
- **Platform**：跨平台目录、文件、系统接口封装
- **Thread**：线程基础封装，支持线程启停、同步、管理
- **Timer**：通用定时器组件
- **Utility**：通用工具函数集合（含 Double 精度比较、时间工具等）
- **Aspect**：AOP 面向切面编程，支持日志切面、性能监控切面
- **ConfigStructs**：通用配置结构体（时区、IP 地址、订阅合约等）

### 2. 网络通信模块（Network）

适配主流 IO 模型，支持协议通信与共享内存交互，专为金融低时延场景优化：
- **IO**：统一 IO 抽象接口，屏蔽平台差异；多路复用模型：`Select`（通用）、`Epoll`（Linux）、`IOCP`（Windows）
- **Tcp**：TCP 通信封装，完整支持 Select / Epoll / IOCP 三种模型的服务端与客户端
- **Protocol**：金融协议封装——Step 协议（文本标签型）、XTP 协议（二进制高性能）；包含包序列化（`Package::MakePackage`）、包解析（`PackageReader`）、缓冲管理、校验和计算
- **Shm**：共享内存通信，适用于进程间高速数据交互

### 3. 序列化模块（Serialization）

数据编解码、格式解析与字符编码转换：
- **JSON**：基于 JsonCpp 实现 JSON 解析与序列化
- **Base64**：Base64 编解码工具
- **CSV**：轻量 CSV 文件解析器（CsvParser + CsvRecord）
- **Encode**：字符编码互转（GBK / UTF-8 / Unicode）

### 4. 模板工具库（TemplateLib）

高性能数据结构与通用业务组件：
- **Buffer**：通用缓冲区
- **RingBuffer**：环形缓冲区（无锁 / 有锁）
- **ShmBuffer**：共享内存缓冲区
- **ObjectPool**：对象池，减少频繁内存分配开销
- **LockFreeQueue**：无锁队列
- **ThreadSafeList**：线程安全链表容器

### 5. 辅助工具

项目内置 Python 脚本，用于代码解析、模型生成、批量处理等自动化工作。

## 三、项目目录结构

```
Spark/
├── include/                    # 对外暴露头文件
│   ├── Spark/Core/             # Core 模块头文件
│   ├── Spark/Network/          # Network 模块头文件
│   ├── Spark/Serialization/    # Serialization 模块头文件
│   └── Spark/TemplateLib/      # TemplateLib 模块头文件
├── src/                        # 源码实现
│   ├── Core/                   # Core 模块实现
│   ├── Network/                # Network 模块实现
│   └── Serialization/          # Serialization 模块实现
├── test/                       # 测试程序
│   ├── unittest/               # GTest 单元测试（主要测试入口）
│   │   ├── Core/               # Core 模块单元测试（9 文件）
│   │   ├── Network/            # Network 模块单元测试（4 文件）
│   │   ├── Serialization/      # Serialization 模块单元测试（4 文件）
│   │   ├── TemplateLib/        # TemplateLib 模块单元测试（6 文件）
│   │   └── CMakeLists.txt      # 单元测试构建配置
│   ├── TestCommon/             # 测试公共库（Package 工厂、订阅器等）
│   ├── Packages/               # 测试用包模型定义与生成
│   ├── TestClient/             # 网络客户端测试（旧版）
│   ├── TestServer/             # 网络服务端测试（旧版）
│   ├── TestCore/               # 核心组件测试（旧版）
│   └── TestMD5/                # MD5 验证程序
├── model/                      # 数据模型定义文件（Head.xml / parselist.xml 等）
├── submodules/                 # 子模块依赖（CMakeCommon）
├── bin/                        # 构建产物：动态库 / 可执行文件（按配置分目录）
├── lib/                        # 构建产物：静态库 / 导入库（按配置分目录）
├── out/                        # CMake Presets 构建目录
├── .workflow/                  # CI 流水线配置（GCC 构建）
├── CMakeLists.txt              # CMake 主构建配置
├── CMakePresets.json           # CMake 预设配置（VS / 命令行）
├── *.py                        # Python 自动化脚本
├── UpdateSubmodule.bat/sh      # 子模块更新脚本
├── Install.sh                  # Linux 安装脚本（cmake --install）
├── .gitmodules                 # Git 子模块配置
├── .gitignore                  # Git 忽略规则
└── LICENSE                     # BSD-4-Clause 开源许可证
```

## 四、环境依赖

### 基础要求

- C++ 编译器：支持 **C++20 及以上**（GCC、Clang、MSVC）
- 构建工具：**CMake 3.20+**
- 脚本环境：**Python 3.6+**（仅用于代码生成类脚本，非运行依赖）
- 测试框架：**Google Test**（CMake 自动查找，需已安装或由 vcpkg / 系统包管理器提供）
- 平台：Linux、Windows

### 依赖子模块

项目依赖 `CMakeCommon` 子模块，克隆代码后需同步拉取子模块。

> 详细的环境搭建步骤（代理配置、vcpkg 安装、WSL 镜像网络等）请参见 [环境准备指南](docs/environment-setup.md)。

## 五、快速构建 & 编译

### 1. 克隆代码（含子模块）

```bash
git clone --recursive https://gitee.com/xunmeng200/Spark.git
cd Spark
```

### 2. 更新子模块（若未递归克隆）

```bash
# Linux / Mac
sh UpdateSubmodule.sh

# Windows
UpdateSubmodule.bat
```

### 3. CMake 编译

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

> **提示**：项目内置 `CMakePresets.json`，也可使用 Presets 构建（推荐）：
>
> ```bash
> cmake --preset x64-Release          # Windows（MSVC）
> cmake --build out/build/x64-Release
> ```
>
> Linux / WSL 可改用 `WSL-GCC-Debug` / `WSL-GCC-Release` 预设。

编译完成后，库文件（Core / Network / Serialization / TemplateLib）输出至 `lib/<Config>` 目录，可执行文件（UnitTests、Test* 等）输出至 `bin/<Config>` 目录（例如 Release 配置对应 `bin/Release`）。

### 4. 运行单元测试

```bash
cd build
ctest --output-on-failure
# 或直接运行编译产物
./bin/Release/UnitTests
```

## 六、基础使用示例

### 示例 1：高性能日志组件

```cpp
#include <Spark/Core/Logger/Logger.h>

using namespace spark::core;

int main(int argc, const char* argv[])
{
    // 初始化日志器（传入进程名），设置输出级别并启动日志线程
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    // 分级日志输出（printf 风格格式化）
    WriteLog(LogLevel::Info, "Application started successfully");
    WriteLog(LogLevel::Debug, "Debug message: system init done");
    WriteLog(LogLevel::Error, "Demo running, error code:[%d]", 1001);

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    return 0;
}
```

### 示例 2：JSON 序列化与解析

```cpp
#include <Spark/Serialization/json/json.h>
#include <iostream>
#include <memory>
#include <string>

int main()
{
    // 构造 JSON 对象
    Json::Value root;
    root["order_id"] = "20260615001";
    root["price"] = 123.45;
    root["volume"] = 1000;
    root["is_buy"] = true;

    // 序列化：JSON 对象 → 字符串
    Json::StreamWriterBuilder writerBuilder;
    std::string jsonStr = Json::writeString(writerBuilder, root);
    std::cout << "JSON String: " << jsonStr << std::endl;

    // 反序列化：字符串 → JSON 对象
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

### 示例 3：网络通信（Step 协议客户端）

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

// 包工厂：按包 ID 创建对应包对象（本示例省略实现，详见 test/Packages/PackageFactory.cpp）
class MyPackageFactory : public PackageFactoryBase
{
public:
    virtual Package* CreatePackage(UShortType packageID) override;
};

// Step 协议客户端：继承 Protocol 并实现 ProtocolSubscriber 回调
class MyStepClient : public Protocol, public ProtocolSubscriber
{
public:
    MyStepClient()
        : Protocol(ProtocolTypeType::Step, ServerTypeType::Client,
                   IOModelType::Select, 0, new MyPackageFactory())
    {
        m_ReqInsertOrder = new ReqInsertOrderPackage(); // 由模型自动生成（见 test/Packages）
        Subscribe(this);                        // 注册自身为消息订阅者
        RegisterFront("tcp://127.0.0.1:20001"); // 连接服务端地址
        // 共享内存地址格式：RegisterFront("shm://TestShm:4");  // "shm://" + 服务名 + ":" + 最大连接数
    }

    // 连接建立回调
    void OnProtocolConnect(SessionIDType sessionID, const char* ip, int port) override
    {
        WriteLog(LogLevel::Info, "OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);
        SendReqInsertOrder();
    }

    // 连接断开回调
    void OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port) override
    {
        WriteLog(LogLevel::Info, "OnDisConnect SessionID:[%lld]", sessionID);
    }

    // 消息到达回调
    void OnMessage(Package* package) override
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
        SendReqInsertOrder(); // 收到消息后回送一笔委托
    }

    // 构造并发送一笔买入开仓委托
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
    // 初始化日志
    Logger::GetInstance().Init(argv[0]);
    Logger::GetInstance().SetLogLevel(LogLevel::Info, LogLevel::Info);
    Logger::GetInstance().Start();

    // 创建 IO 线程并启动客户端
    IOThread* ioThread = new IOThread("StepClient");
    MyStepClient client;
    client.SetIOThread(ioThread);
    if (!client.Init())
        return -1;

    ioThread->Start();      // 启动事件循环（阻塞当前线程）
    ioThread->Join();

    Logger::GetInstance().Stop();
    Logger::GetInstance().Join();
    return 0;
}
```

> **说明**：示例中的 `ReqInsertOrderPackage` / `ReqInsertOrderField` 等包模型类由模型自动生成（完整实现参见 `test/Packages/`），回调与包工厂的完整用法参见 `test/TestClient/TestStepClient.cpp`。

## 七、单元测试

项目内置基于 **Google Test** 的完整单元测试套件，共 **23 个测试文件**，覆盖四个模块：

| 模块 | 测试文件 | 覆盖内容 |
| --- | --- | --- |
| **Core** | `AspectTest` | 日志切面、性能切面 |
| | `DirTest` | 目录创建、删除、遍历 |
| | `DoubleUtilityTest` | 浮点数精度比较 |
| | `LoggerTest` | 日志分级、文件滚动、线程安全 |
| | `MD5Test` | MD5 加密正确性 |
| | `ThreadBaseTest` | 线程启停、同步 |
| | `TimerTest` | 定时器触发、取消 |
| | `TimeUtilityTest` | 时间格式化、转换 |
| | `UtilityTest` | 通用工具函数 |
| **Network** | `StepUtilityTest` | Step 协议字段解析、Head/Tail 流式转换（47 用例） |
| | `ProtocolUtilityTest` | CRC32C 校验和与字节模式查找（9 + 7 用例） |
| | `PackageReaderTest` | 缓冲管理：Append/PopFront/Reset（12 用例） |
| | `PackageSerializationTest` | MakePackage ↔ ParsePackage 端到端往返、重同步与版本校验（15 用例） |
| **Serialization** | `Base64Test` | Base64 编解码 |
| | `CSVParserTest` | CSV 解析行、列、引号转义 |
| | `CSVRecordTest` | CSV 记录读写 |
| | `EncodeTest` | GBK/UTF-8/Unicode 互转 |
| **TemplateLib** | `BufferTest` | 缓冲区读写、扩容 |
| | `LockFreeQueueTest` | 无锁队列入队出队 |
| | `ObjectPoolTest` | 对象池分配、复用 |
| | `RingBufferTest` | 环形缓冲区读写覆盖 |
| | `ShmBufferTest` | 共享内存缓冲 |
| | `ThreadSafeListTest` | 线程安全增删遍历 |

### 运行全部测试

```bash
cd build
ctest
```

亦可直接运行 `bin/<Config>/UnitTests` 可执行文件查看详细输出（例如 Release 为 `bin/Release/UnitTests`）。

## 八、脚本说明

根目录下 Python 脚本用于自动化代码处理：

| 脚本 | 说明 |
| ---- | ---- |
| pump.py / pumpall.py | 模板代码生成引擎（由 pumplist.xml / parselist.xml 驱动） |
| pumptemp.py | 特定模板生成（如 Types.h / EnumString.h） |
| ParsePackageModel.py / ParseTableModel.py | 包模型 / 数据表模型解析 |
| ParseShortField.py / ParseShortItem.py | 短字段 / 短条目解析 |
| parseall.py | 批量解析入口 |
| ConvertToUtf8Bom.py | 文件编码统一转换为 UTF-8 BOM |
| clearall.py | 临时文件清理 |

## 九、许可证 & 声明

- **开源协议**：BSD-4-Clause，详见 [LICENSE](LICENSE) 文件
- **适用范围**：本项目仅供个人学习、研究使用
- **风险提示**：本库为个人开源项目，生产环境使用请自行充分测试并评估风险

## 十、补充说明

- **跨平台差异**：`Epoll` 仅支持 Linux，`IOCP` 仅支持 Windows，代码已做平台适配；`Select` 为通用模型，两平台均可使用
- **金融协议**：内置 Step 协议（文本标签型，适用于期货 CTP 协议族）和 XTP 协议（二进制高性能）封装，可直接对接对应行情 / 交易接口
- **扩展开发**：可基于现有模块扩展自定义协议、数据结构、业务组件
- **包含路径**：头文件使用 `#include <Spark/Module/HeaderName.h>` 风格
