# Real-Time Log Shipper (C + ZeroMQ)

A high-performance, cross-platform log shipping solution written in C. It monitors a directory structure for changes, filters log entries, and streams them in real-time to a central collector server using ZeroMQ.

## Features

* **Real-Time Streaming:** Low latency log delivery using ZeroMQ (PUB/SUB).
* **Cross-Platform Monitoring:** Uses native OS APIs to minimize CPU usage:
    * **Linux:** `inotify`
    * **macOS:** `kqueue`
    * **Windows:** `ReadDirectoryChangesW` / `WaitForSingleObject`
* **Recursive Scanning:** Automatically detects logs in nested directories.
* **Resilience:** Handles log rotation and server reconnections automatically.
* **Dockerized:** Ready for deployment with Docker Compose.

## Architecture

1.  **Client (Log Agent):** Watches the file system. When a change is detected, it reads new lines from `.log` files and pushes them via TCP.
2.  **Server (Log Collector):** Subscribes to the stream and outputs formatted logs to `stdout`.

## Prerequisites

* **C Compiler:** GCC, Clang, or MSVC.
* **Build System:** CMake (3.10+).
* **Library:** libzmq (ZeroMQ).
* **Docker:** (Optional) for containerized build/run.

## Building and Running

### Using Docker (Recommended)

```bash
# Build and start services
docker-compose up --build