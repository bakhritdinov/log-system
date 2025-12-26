# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2025-12-26
### Added
- Integrated `uthash.h` for O(1) file record lookups.
- Added `FetchContent` in CMake for seamless dependency management.
- Implemented `ZMQ_SNDHWM` to protect against Out-of-Memory (OOM) issues.
- Added explicit memory cleanup in `log_agent` on exit.

### Optimized
- **Architecture Shift:** Moved log parsing logic from Agent to Collector. The Agent now ships raw lines, significantly reducing its CPU footprint.
- **Linux Core:** Optimized `inotify` loop to process events in batches using a dedicated buffer.
- **Protocol:** Simplified ZeroMQ messaging to a 2-frame protocol `[Path][Line]`.

### Fixed
- Fixed a potential buffer overflow in string copying using safer `strncpy` patterns.
- Resolved an issue in Docker Compose where the agent would fail to resolve the collector's hostname.

## [1.0.0] - 2025-12-20
### Added
- Initial release of the Log Shipper.
- Basic support for Linux (inotify), macOS (kqueue), and Windows.
- PUB/SUB implementation using ZeroMQ.
- Basic recursive directory scanning.