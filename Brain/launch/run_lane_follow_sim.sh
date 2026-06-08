#!/usr/bin/env bash
# Lane-follow pipeline test using camera_sim (no real camera/STM32 needed for vision/planning/control loop).
# Pipeline: camera_sim -> lane_node -> [serial_node -> state_node] -> planning_node -> control_node
#
# Usage: ./launch/run_lane_follow_sim.sh [video_path] [uart_device]
#   video_path:  default Brain/record/2p_haveturn.mp4
#   uart_device: default /dev/ttyACM0 — if serial_node fails to open it, EgoState.speed stays 0
#                (Stanley still runs thanks to the epsilon term, just without real speed feedback)

set -u

ROOT="$(dirname "$0")/.."
BIN="$ROOT/build/bin"
VIDEO="${1:-$ROOT/record/Full_map.mp4}"
DEVICE="${2:-/dev/ttyACM0}"

trap 'kill "${PIDS[@]:-}" 2>/dev/null; wait 2>/dev/null' INT TERM EXIT

PIDS=()

"$BIN/camera_sim_node" "$VIDEO" &
PIDS+=($!)
sleep 0.3

"$BIN/lane_node" --show &
PIDS+=($!)
sleep 0.3

"$BIN/serial_node" "$DEVICE" &
PIDS+=($!)
sleep 0.3

"$BIN/state_node" &
PIDS+=($!)
sleep 0.3

"$BIN/planning_node" &
PIDS+=($!)
sleep 0.3

"$BIN/control_node" &
PIDS+=($!)

wait
