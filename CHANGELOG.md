# Changelog

## [1.2.0] - 2025-12-29
### Added
- **Configuration Context Pattern:** Replaced global variables with an `AgentConfig` structure for better thread-safety and testability.
- **Fresh Start Mode:** Added `--fresh` CLI flag to skip existing file content (tail mode), preventing massive data spikes on startup with large log files.
- **Cross-Platform Tail Logic:** Implemented "jump-to-end" logic for both Windows and Linux file handlers.

### Fixed
- Improved `cleanup_records` to prevent memory leaks during repeated test execution in Unity.
- Standardized pointer alignment and code style across all C files.

## [1.1.0] - 2025-12-26
### Added
- Integrated `uthash.h` for O(1) file record lookups.
- Added `FetchContent` in CMake for seamless dependency management.
- Implemented `ZMQ_SNDHWM` to protect against Out-of-Memory (OOM) issues.
- Added explicit memory cleanup in `log_agent` on exit.

### Optimized
- **Architecture Shift:** Moved log parsing logic from Agent to Collector. The Agent now ships raw lines, significantly reducing its CPU footprint.
- **Linux Core:** Optimized `inotify` loop to process events in batches.
- **Protocol:** Simplified ZeroMQ messaging to a 2-frame protocol `[Path][Line]`.

## [1.0.0] - 2025-12-20
### Added
- Initial release with basic support for Linux, macOS, and Windows.
- PUB/SUB implementation using ZeroMQ.