[![C/C++ CI](https://github.com/bakhritdinov/log-system/actions/workflows/ci.yml/badge.svg)](https://github.com/bakhritdinov/log-system/actions/workflows/ci.yml)

# Real-Time Log Shipper (C + ZeroMQ)

A high-performance, cross-platform log shipping solution written in C. It monitors a directory structure for changes, filters log entries, and streams them in real-time to a central collector server using ZeroMQ.

## 🚀 Key Optimizations

* **Configuration Context:** Uses a decoupled configuration pattern, avoiding global state and enabling easier integration/testing.
* **O(1) File Tracking:** Uses `uthash` for instant lookup of file offsets, ensuring performance doesn't degrade with thousands of log files.
* **Smart Cold Start:** Includes a `--fresh` mode to ignore existing terabytes of legacy data and only stream new incoming logs.
* **Native Event Loops:** Avoids expensive polling by using kernel-level APIs (`inotify`, `ReadDirectoryChangesW`).
* **Memory-Safe Streaming:** Implements ZeroMQ High Water Marks (HWM) to prevent memory exhaustion.

## 🏗 Architecture

1.  **Log Agent:** A "sidecar" utility that tracks file offsets using an internal hash table.
2.  **Log Collector:** A central sink that aggregates streams, handles ANSI coloring, and provides real-time visualization.

## 🛠 Prerequisites

* **libzmq:** ZeroMQ core library.
* **CMake:** 3.15 or higher.
* **Compiler:** GCC/Clang (C11 support) or MSVC.

## 💻 Usage

### Locally
1. **Start Collector:**
   ```bash
   ./log_collector tcp://*:5555
   
2. **Start Agent:**
   ```bash

    ./log_agent /var/log/my_app tcp://127.0.0.1:5555
