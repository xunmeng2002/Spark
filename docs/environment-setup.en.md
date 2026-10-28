# Environment Setup Guide

> This document covers both **Windows (MSVC)** and **WSL (GCC)** platforms, detailing every step required to set up the build environment from scratch.

---

## Table of Contents

- [Environment Setup Guide](#environment-setup-guide)
  - [Table of Contents](#table-of-contents)
  - [1. Windows (MSVC)](#1-windows-msvc)
    - [1.1 System Requirements](#11-system-requirements)
    - [1.2 Visual Studio 2022](#12-visual-studio-2022)
    - [1.3 Git](#13-git)
    - [1.4 Proxy Configuration (Optional but Recommended)](#14-proxy-configuration-optional-but-recommended)
    - [1.5 doctest](#15-doctest)
    - [1.6 Verification](#16-verification)
  - [2. WSL (GCC)](#2-wsl-gcc)
    - [2.1 System Requirements](#21-system-requirements)
    - [2.2 WSL Setup](#22-wsl-setup)
    - [2.3 Build Toolchain](#23-build-toolchain)
    - [2.4 doctest](#24-doctest)
    - [2.5 Proxy Configuration (Optional)](#25-proxy-configuration-optional)
    - [2.6 Verification](#26-verification)
  - [3. Troubleshooting](#3-troubleshooting)
    - [3.1 CMake cannot find doctest](#31-cmake-cannot-find-doctest)
    - [3.2 cmake cannot find Ninja in WSL](#32-cmake-cannot-find-ninja-in-wsl)
    - [3.3 Git errors after turning proxy off](#33-git-errors-after-turning-proxy-off)

---

## 1. Windows (MSVC)

### 1.1 System Requirements

- **OS**: Windows 10 21H2+ / Windows 11
- **Disk**: At least 20 GB free space

### 1.2 Visual Studio 2022

Install **Visual Studio 2022** (Enterprise or Community). Under **Workloads**, check:

- **Desktop development with C++**
  - MSVC v143 build tools (x64/x86)
  - Windows 10/11 SDK
  - C++ CMake tools for Windows
  - C++ ATL for latest v143 build tools (optional)

Reference installation path:

```
D:/Program Files/Microsoft Visual Studio/2022/Enterprise/
```

### 1.3 Git

```bash
git --version  # Must be ≥ 2.30
```

Git can be installed via the Visual Studio installer or from [https://git-scm.com](https://git-scm.com).

### 1.4 Proxy Configuration (Optional but Recommended)

In mainland China, GitHub connectivity is often unreliable. Using a proxy tool (e.g., Clash Verge) is recommended.

**1.4.1 Configure Git Proxy**

```bash
# Set (adjust port to match your proxy tool)
git config --global http.proxy http://127.0.0.1:7897
git config --global https.proxy http://127.0.0.1:7897

# Unset
git config --global --unset http.proxy
git config --global --unset https.proxy
```

**1.4.2 Set Environment Variables (Persistent)**

```bash
setx HTTP_PROXY "http://127.0.0.1:7897"
setx HTTPS_PROXY "http://127.0.0.1:7897"
setx http_proxy "http://127.0.0.1:7897"
setx https_proxy "http://127.0.0.1:7897"
```

Restart your terminal or Visual Studio after setting.

### 1.5 doctest

Unit tests use **doctest 2.5.3** — header-only, no package manager. Following the sibling repositories' convention, the build reads it from the sibling directory `../Libs/doctest`, which must have the CMake-installed layout:

```text
Libs/
└── doctest/
    ├── include/doctest/doctest.h
    └── lib/cmake/doctest/doctestConfig.cmake
```

Install doctest into that location from its source tree (`doctest` below denotes the source root):

```bash
cmake -S doctest -B doctest/build -DCMAKE_INSTALL_PREFIX=../Libs/doctest
cmake --build doctest/build --target install
```

> **Note**: the root `CMakeLists.txt` locates it with `find_package(doctest CONFIG REQUIRED PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/doctest")`; update that line if the directory is moved or renamed.

### 1.6 Verification

Open **Visual Studio**, select the `x64-Debug` configuration, and run CMake configuration (generate cache):

- Configuration should resolve `../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake`
- `UnitTests` should build without errors or warnings
- Running `UnitTests` should end with `test cases: 503 | 503 passed` (Debug)

---

## 2. WSL (GCC)

### 2.1 System Requirements

- **OS**: Windows 10 21H2+ / Windows 11 (WSL2 enabled)
- **WSL version**: 2.0.0+
- **Linux distro**: Ubuntu 22.04 LTS (recommended)

### 2.2 WSL Setup

**2.2.1 Install WSL**

```bash
# Run PowerShell as Administrator
wsl --install -d Ubuntu

# Verify
wsl -l -v
#   NAME      STATE           VERSION
# * Ubuntu    Running         2
```

**2.2.2 Mirrored Networking Mode (Optional but Recommended)**

Makes WSL share the Windows network stack — localhost is unified and proxy settings are inherited automatically.

Create `%USERPROFILE%\.wslconfig` with:

```ini
[wsl2]
networkingMode=mirrored
dnsTunneling=true
autoProxy=true
```

Restart WSL:

```bash
wsl --shutdown
# Then restart wsl
```

### 2.3 Build Toolchain

Run these commands **inside WSL**:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build pkg-config -y
```

| Package | Purpose |
|---------|---------|
| `build-essential` | g++, gcc, make, and other compilation tools |
| `cmake` | CMake build system |
| `ninja-build` | Ninja build system (the project's generator) |
| `pkg-config` | Package configuration helper |

### 2.4 doctest

When building `/mnt/d/Gitee/Spark` inside WSL, `../Libs/doctest` resolves to the same directory used on Windows — no separate installation is needed. If the repository is cloned independently inside WSL, place the same doctest installation next to it.

**Verify:**

```bash
ls ../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake
```

### 2.5 Proxy Configuration (Optional)

If mirrored networking mode is not enabled, configure the proxy manually inside WSL:

```bash
# Git proxy
git config --global http.proxy http://127.0.0.1:7897
git config --global https.proxy http://127.0.0.1:7897

# Environment variables
echo 'export http_proxy=http://127.0.0.1:7897' >> ~/.bashrc
echo 'export https_proxy=http://127.0.0.1:7897' >> ~/.bashrc
source ~/.bashrc
```

### 2.6 Verification

Run CMake configuration inside WSL:

```bash
cd /mnt/d/Gitee/Spark
cmake -S . -B out/build/WSL-GCC-Debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX=../Libs/Spark/x64-linux \
  -G Ninja
```

Alternatively, use **Visual Studio**'s CMake integration — select the `WSL-GCC-Debug` configuration and generate the cache.

---

## 3. Troubleshooting

### 3.1 CMake cannot find doctest

- Confirm `../Libs/doctest/lib/cmake/doctest/doctestConfig.cmake` exists (doctest 2.5.3)
- If the directory sits elsewhere, move it back beside the repository or update the `find_package(doctest ...)` line in the root `CMakeLists.txt`
- In VS, select **Project → Delete Cache and Regenerate**, or delete the `out/` directory and reconfigure

### 3.2 cmake cannot find Ninja in WSL

```bash
sudo apt install ninja-build
```

### 3.3 Git errors after turning proxy off

```bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```
