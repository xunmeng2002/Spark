# 环境准备指南

> 本文档面向 **Windows（MSVC）** 和 **WSL（GCC）** 两个平台，说明从零搭建项目编译环境所需的全部步骤。

---

## 目录

- [环境准备指南](#环境准备指南)
  - [目录](#目录)
  - [一、Windows（MSVC）](#一windowsmsvc)
    - [1.1 系统要求](#11-系统要求)
    - [1.2 Visual Studio 2022](#12-visual-studio-2022)
    - [1.3 Git](#13-git)
    - [1.4 代理配置（可选但推荐）](#14-代理配置可选但推荐)
    - [1.5 doctest](#15-doctest)
    - [1.6 验证](#16-验证)
  - [二、WSL（GCC）](#二wslgcc)
    - [2.1 系统要求](#21-系统要求)
    - [2.2 WSL 安装与配置](#22-wsl-安装与配置)
    - [2.3 构建工具链](#23-构建工具链)
    - [2.4 doctest](#24-doctest)
    - [2.5 代理配置（可选）](#25-代理配置可选)
    - [2.6 验证](#26-验证)
  - [三、常见问题](#三常见问题)
    - [3.1 CMake 报找不到 doctest](#31-cmake-报找不到-doctest)
    - [3.2 WSL 中 cmake 找不到 Ninja](#32-wsl-中-cmake-找不到-ninja)
    - [3.3 代理关闭后 Git 报错](#33-代理关闭后-git-报错)

---

## 一、Windows（MSVC）

### 1.1 系统要求

- **操作系统**：Windows 10 21H2+ / Windows 11
- **磁盘空间**：至少 20 GB 可用空间

### 1.2 Visual Studio 2022

安装 **Visual Studio 2022**（Enterprise 或 Community 均可），工作负载勾选：

- **使用 C++ 的桌面开发**
  - MSVC v143 生成工具（x64/x86）
  - Windows 10/11 SDK
  - C++ CMake 工具（适用于 Windows）
  - 适用于最新 v143 生成工具的 C++ ATL（可选）

安装路径（参考）：

```
D:/Program Files/Microsoft Visual Studio/2022/Enterprise/
```

### 1.3 Git

```bash
git --version  # 需 ≥ 2.30
```

Git 随 Visual Studio 安装时可选，也可从 [https://git-scm.com](https://git-scm.com) 单独安装。

### 1.4 代理配置（可选但推荐）

在中国大陆网络环境下，GitHub 访问不稳定，推荐使用代理工具（如 Clash Verge）。

**1.4.1 配置 Git 代理**

```bash
# 设置（根据你的代理端口修改）
git config --global http.proxy http://127.0.0.1:7897
git config --global https.proxy http://127.0.0.1:7897

# 取消
git config --global --unset http.proxy
git config --global --unset https.proxy
```

**1.4.2 配置环境变量（持久化）**

```bash
setx HTTP_PROXY "http://127.0.0.1:7897"
setx HTTPS_PROXY "http://127.0.0.1:7897"
setx http_proxy "http://127.0.0.1:7897"
setx https_proxy "http://127.0.0.1:7897"
```

设置后需重启终端或 Visual Studio。

### 1.5 doctest

单元测试框架为 **doctest 2.5.3**，纯头文件、无需包管理器。按兄弟仓的约定，构建从同级目录 `../Libs/doctest` 读取，该目录须为 CMake 安装后的形态：

```text
Libs/
└── doctest/
    ├── include/doctest/doctest.h
    └── lib/cmake/doctest/doctestConfig.cmake
```

由 doctest 源码安装到该位置（下称源码树根为 `doctest`）：

```bash
cmake -S doctest -B doctest/build -DCMAKE_INSTALL_PREFIX=../Libs/doctest
cmake --build doctest/build --target install
```

> **注意**：根 `CMakeLists.txt` 通过 `find_package(doctest CONFIG REQUIRED PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/doctest")` 定位；目录移动或改名后须同步修改该行。

### 1.6 验证

打开 **Visual Studio**，选择 `x64-Debug` 配置，执行 CMake 配置（生成 cache）：

- 配置阶段应定位到 `../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake`
- 构建 `UnitTests` 无报错、无警告
- 运行 `UnitTests`，末尾汇总应为 `test cases: 503 | 503 passed`（Debug 档）

---

## 二、WSL（GCC）

### 2.1 系统要求

- **操作系统**：Windows 10 21H2+ / Windows 11（已启用 WSL2）
- **WSL 版本**：2.0.0+
- **Linux 发行版**：Ubuntu 22.04 LTS（推荐）

### 2.2 WSL 安装与配置

**2.2.1 安装 WSL**

```bash
# 以管理员身份运行 PowerShell
wsl --install -d Ubuntu

# 确认版本
wsl -l -v
#   NAME      STATE           VERSION
# * Ubuntu    Running         2
```

**2.2.2 镜像网络模式（可选但推荐）**

使 WSL 共享 Windows 的网络栈，localhost 互通，自动继承代理设置。

在 `%USERPROFILE%\.wslconfig` 中写入：

```ini
[wsl2]
networkingMode=mirrored
dnsTunneling=true
autoProxy=true
```

重启 WSL：

```bash
wsl --shutdown
# 然后重新运行 wsl
```

### 2.3 构建工具链

在 WSL 终端中执行：

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build pkg-config -y
```

| 包名 | 作用 |
|------|------|
| `build-essential` | g++、gcc、make 等编译工具链 |
| `cmake` | CMake 构建系统 |
| `ninja-build` | Ninja 构建系统（项目使用的生成器） |
| `pkg-config` | 包配置工具 |

### 2.4 doctest

WSL 中构建 `/mnt/d/Gitee/Spark` 时，`../Libs/doctest` 与 Windows 侧指向同一份目录，不必重复安装。若仓库在 WSL 内独立克隆，把同样的 doctest 安装目录放回仓库同级即可。

**验证：**

```bash
ls ../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake
```

### 2.5 代理配置（可选）

如果未启用镜像网络模式，需在 WSL 中手动配置代理：

```bash
# Git 代理
git config --global http.proxy http://127.0.0.1:7897
git config --global https.proxy http://127.0.0.1:7897

# 环境变量
echo 'export http_proxy=http://127.0.0.1:7897' >> ~/.bashrc
echo 'export https_proxy=http://127.0.0.1:7897' >> ~/.bashrc
source ~/.bashrc
```

### 2.6 验证

在 WSL 终端直接运行 CMake 配置：

```bash
cd /mnt/d/Gitee/Spark
cmake -S . -B out/build/WSL-GCC-Debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX=../Libs/Spark/x64-linux \
  -G Ninja
```

或使用 **Visual Studio** 的 CMake 集成，选择 `WSL-GCC-Debug` 配置后生成 cache。

---

## 三、常见问题

### 3.1 CMake 报找不到 doctest

- 确认 `../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake` 存在（doctest 2.5.3）
- 目录位置不对时，移回仓库同级，或同步修改根 `CMakeLists.txt` 中的 `find_package(doctest ...)` 行
- 在 VS 中选择 **项目 → 清除缓存并重新生成**，或删除 `out/` 目录后重新配置

### 3.2 WSL 中 cmake 找不到 Ninja

```bash
sudo apt install ninja-build
```

### 3.3 代理关闭后 Git 报错

```bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```
