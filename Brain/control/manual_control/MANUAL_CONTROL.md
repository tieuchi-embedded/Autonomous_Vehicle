# Manual Control — Keyboard-driven Vehicle Operation

## Overview

Manual control allows operator to drive the vehicle via SSH keyboard input without requiring an autonomous mode. Useful for:
- Test bench data collection
- Video recording with known trajectories
- Hardware testing and calibration
- Emergency manual takeover

## Architecture

```
keyboard (SSH terminal)
    ↓
manual_node (raw TTY, non-blocking input)
    ↓
CONTROL_CMD message (POSIX MQ)
    ↓
serial_node (UART downlink)
    ↓
#RPM:value;;\r\n and #STR:value;;\r\n frames
    ↓
STM32F4 (motor and servo control)
```

## Build

Ensure CMake includes the manual_control subdirectory (should already be in `Brain/CMakeLists.txt`):

```bash
cd ~/Desktop/Autonomous_Vehicle/Brain
cmake -S . -B build
cmake --build build
```

The binary `manual_node` will be at `build/bin/manual_node`.

## Run

### Prerequisites

- STM32 connected via USB (appears as `/dev/ttyACM0` or similar)
- Running over SSH with a TTY allocated (not pipe/heredoc)

### Launch Manual Control

```bash
./launch/run_manual.sh [uart_device]
```

Default device: `/dev/ttyACM0`. Override with:
```bash
./launch/run_manual.sh /dev/ttyUSB0
```

**What happens:**
1. `serial_node` starts in background, opens UART, waits for STM32 data
2. `manual_node` starts in foreground, takes over terminal, displays control info
3. Keyboard input is processed without requiring Enter

### Stop

Press `q` to quit. Manual_node sends safety stop (`rpm=0, steer=0`) before exit.

On unexpected crash: `Ctrl+C` sends SIGINT to both processes, cleanup via trap.

## Operation

### Keys

| Key | Action | Range |
|-----|--------|-------|
| `w` | Increase RPM | +step each press |
| `s` | Decrease RPM | -step each press |
| `a` | Steer left | -STEER_STEP each press |
| `d` | Steer right | +STEER_STEP each press |
| `space` | Full stop (rpm=0, steer=0) | — |
| `x` | Center steering only (steer=0) | — |
| `q` | Quit and shutdown | — |

### Display

Each state change prints to terminal (same line, carriage return):
```
rpm=+50.0  steer=+5.0 deg
```

Units:
- **rpm**: Motor RPM (±100 typical range, hardware-limited at STM32)
- **steer_deg**: Steering angle in degrees (±25 typical range, hardware-constrained)

### Tuning Step Sizes

Without rebuild, adjust step sizes via environment variables:

```bash
export MANUAL_RPM_STEP=10.0      # default 20.0
export MANUAL_STEER_STEP=2.5     # default 5.0
export MANUAL_STEER_MAX=30.0     # default 25.0
./launch/run_manual.sh
```

**Recommended values:**
- **Low speed test**: `RPM_STEP=5, STEER_STEP=2.5`
- **Normal driving**: `RPM_STEP=20, STEER_STEP=5`
- **Aggressive**: `RPM_STEP=50, STEER_STEP=10` (requires good reflexes)

## Technical Details

### Non-blocking Keyboard Input

- Raw TTY mode: `ICANON` and `ECHO` disabled via `termios`
- Polling interval: 50ms via `select()` with zero-copy read
- No blocking on I/O — responsive even under load

### UART Protocol

Downlink (Brain → STM32):
```
#RPM:value;;\r\n     // Motor RPM command
#STR:value;;\r\n     // Steering angle command
```

Example:
```
#RPM:30.0;;\r\n
#STR:10.5;;\r\n
```

STM32 interprets:
- Positive RPM: forward motion
- Negative RPM: reverse motion
- Zero RPM: coast/brake (hardware-dependent)
- Steering: degrees, typically ±25° max

### Message Flow

1. Operator presses key → C value in memory
2. Check if changed → mark `dirty=true`
3. Publish `ControlCmd{rpm, steer_deg}` to MQ
4. `serial_node` subscribes, formats UART frames, sends to STM32
5. STM32 controls motor and servo

## Troubleshooting

### "cannot set raw tty (need real TTY, not pipe)"

Cause: Running over pipe/heredoc instead of interactive SSH session.

Fix:
```bash
# ✓ Correct
ssh user@pi
./launch/run_manual.sh

# ✗ Wrong
echo "./launch/run_manual.sh" | ssh user@pi
ssh user@pi < script.sh
```

Ensure TTY is allocated:
```bash
ssh -t user@pi ./launch/run_manual.sh
```

### "ipc_publish_open failed" or "ipc_subscribe_open failed"

Cause: Another `manual_node` or `serial_node` already running, or stale IPC resources.

Fix:
```bash
# Kill existing processes
pkill manual_node
pkill serial_node

# Optional: clean IPC (only if necessary)
rm -f /dev/mqueue/CONTROL_CMD /dev/shm/SERIAL_*

# Retry
./launch/run_manual.sh
```

### Vehicle does not respond to commands

Cause: STM32 not receiving frames, or UART device wrong.

Fix:
1. Check device:
   ```bash
   ls -la /dev/ttyACM* /dev/ttyUSB*
   ```
2. Test UART with `dmesg` to see if data arrives:
   ```bash
   dmesg | tail -20
   ```
3. Verify serial_node is running:
   ```bash
   ps aux | grep serial_node
   ```

### Keys respond slowly or lag

Cause: Polling timeout too high (default 50ms), or system load.

Reduce in code if needed (edit `manual_node.cpp` line 53):
```cpp
struct timeval tv = {0, 50 * 1000};  // 50ms polling
```

Lower value = lower latency, higher CPU usage.

## Integration with Other Modes

Manual control is **independent** and can coexist with:
- **Perception pipeline** (camera_sim_node, lane_node running) — manual_node just publishes CONTROL_CMD
- **Autonomous mode** (future planning_node) — switch modes by only running one publisher to CONTROL_CMD

To avoid conflicts:
- **Mode 1 (autonomous)**: run perception + planning_node (no manual_node)
- **Mode 2 (manual)**: run manual_node + serial_node (perception optional for monitoring)

## Example Sessions

### Record Test Drive

```bash
# Terminal 1: Start manual control
./launch/run_manual.sh

# Terminal 2 (on test PC): Record video
ffmpeg -f x11grab -i :0 -c:v libx264 test_drive.mp4

# Drive: w/a/d/s to navigate, q to stop
# Recorded data goes to STM32 firmware log (if enabled)
```

### Bench Test with Known Inputs

```bash
export MANUAL_RPM_STEP=10
export MANUAL_STEER_STEP=1
./launch/run_manual.sh /dev/ttyUSB0

# Slow, precise control for sensor calibration
```

## See Also

- [Brain architecture plan](./../../../.claude/plans/brain-architecture-phased.md)
- [Serial bridge protocol](../serial_bridge/include/serial/protocol.hpp)
- [Control message format](../../messages/control_cmd.h)
