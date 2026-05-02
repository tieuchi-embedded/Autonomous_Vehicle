#!/usr/bin/env bash
# Manual driving mode: keyboard SSH -> CONTROL_CMD -> serial_node -> STM32.
# Usage: ./launch/run_manual.sh [uart_device]

set -u

DEVICE="${1:-/dev/ttyACM0}"
BIN="$(dirname "$0")/../build/bin"

trap 'kill "${PIDS[@]:-}" 2>/dev/null; wait 2>/dev/null' INT TERM EXIT

PIDS=()

"$BIN/serial_node" "$DEVICE" &
PIDS+=($!)
sleep 0.3

# manual_node runs in foreground (needs TTY)
"$BIN/manual_node"
