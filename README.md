# Real-Time Log Shipper (C + ZeroMQ)

A high-performance, minimalist log shipping solution written in C. Designed for ultra-low overhead, it monitors directory structures and streams raw log data to a central collector.

## 🚀 Key Optimizations

* **O(1) File Tracking:** Uses `uthash` for instant lookup of file offsets, ensuring performance doesn't degrade with thousands of log files.
* **Zero-Parsing Agent:** Following the "thin agent" philosophy, the shipper performs no regex or string parsing, preserving CPU cycles on production nodes.
* **Native Event Loops:** Avoids expensive polling by using kernel-level APIs (`inotify`, `kqueue`, `ReadDirectoryChangesW`).
* **Memory-Safe Streaming:** Implements ZeroMQ High Water Marks (HWM) to prevent memory exhaustion during network congestion.

## 🏗 Architecture

1.  **Log Agent:** A "sidecar" utility that tracks file descriptors and offsets. It uses a multi-frame ZeroMQ message protocol: `[Topic: Path] [Payload: Raw Line Data]`.
2.  **Log Collector:** A central sink that aggregates streams from multiple agents. It handles log coloring and can be extended for persistent storage.

## 🛠 Prerequisites

* **libzmq:** ZeroMQ core library.
* **uthash:** (Handled automatically via CMake FetchContent).
* **CMake:** 3.15 or higher.

## 💻 Usage

### Locally
1. **Start Collector:**
   ```bash
   ./log_collector tcp://*:5555
   
2. **Start Agent:**
   ```bash
    ./log_agent /var/log/my_app tcp://127.0.0.1:5555