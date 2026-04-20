# BFMC Brain — Multi-binary, phased implementation

> **Trạng thái:** WIP — Phase 1 (Perception) gần xong, Phases 2/3/4 còn sketch.
> **Session kế tiếp:** đọc file này trước khi tiếp tục planning hoặc implement.

---

## Context

Brain chạy trên **Raspberry Pi 5**. Kiến trúc **multi-binary**: mỗi node là 1 executable độc lập, giao tiếp qua `libipc` (shared memory + POSIX MQ). Triển khai **chia pha** — từng pha xong mới sang pha sau.

- **Phase 1 — Perception** ◀ đang làm
- **Phase 2 — Planning**
- **Phase 3 — Control + serial downlink**
- **Phase 4 — Web / telemetry**

| Tầng | Ngôn ngữ |
|------|----------|
| libipc (shm + mq) | C |
| camera, lane, state, localization, serial, planning, control | C++ |
| object_detection, web | Python |

Message schema là **POD C struct** trong `messages/*.h` — C++ dùng trực tiếp, Python map qua `ctypes.Structure`.

---

## 1. Toàn hệ (tham chiếu)

```
STM32 ──UART──▶ serial_node ──┬──▶ IMU_STATE ───┐
                              ├──▶ WHEEL_ODOM ──┤
                              └──▶ GPS_FIX ─────┼──▶ state_node ──▶ EGO_STATE ──┐
                                                │                                │
libcamera ──▶ camera_node ──shm──▶ CAMERA_FRAME ┤                                ├──▶ localization_node ──▶ POSE2D
                                                ├──▶ lane_node ──────▶ LANE_STATE│
                                                └──▶ object_detection ──▶ DETECTIONS

[Phase 2] planning_node sub (LANE_STATE, DETECTIONS, EGO_STATE, POSE2D) ──▶ BEHAVIOR_CMD
[Phase 3] control_node  sub (BEHAVIOR_CMD, EGO_STATE) ──▶ CONTROL_CMD ──▶ serial_node ──UART──▶ STM32
[Phase 4] web           sub tất cả → dashboard
```

---

## 2. PHASE 1 — Perception (hiện tại)

### 2.1 Scope Phase 1

| Node | Ngôn ngữ | Input | Output (topic) |
|------|----------|-------|----------------|
| **libipc** | C | — | shared lib |
| **messages** | C header | — | headers |
| **serial_node** (uplink) | C++ | UART từ STM32 | `IMU_STATE`, `WHEEL_ODOM`, `GPS_FIX` |
| **camera_node** | C++ | libcamera (Pi 5) | `CAMERA_FRAME` (shm) |
| **lane_node** | C++ | `CAMERA_FRAME` | `LANE_STATE` |
| **object_detection_node** | Python | `CAMERA_FRAME` | `DETECTIONS` |
| **state_node** | C++ | `IMU_STATE`, `WHEEL_ODOM` | `EGO_STATE` |
| **localization_node** | C++ | `EGO_STATE`, `GPS_FIX` (opt.) | `POSE2D` |

## Bảng topic

| Topic id           | Producer           | Ngôn ngữ | Transport | Payload        |
|--------------------|--------------------|----------|-----------|----------------|
| `CAMERA_FRAME`     | camera_node        | C++      | shm ring  | CameraFrame    |
| `LANE_STATE`       | lane_node          | C++      | POSIX MQ  | LaneState      |
| `DETECTIONS`       | object_detection   | Python   | POSIX MQ  | Detection[]    |
| `WORLD_STATE`      | planning_node (in) | internal | agg       | WorldState     |
| `BEHAVIOR_CMD`     | planning_node      | C++      | POSIX MQ  | BehaviorCmd    |
| `CONTROL_CMD`      | control_node       | C++      | POSIX MQ  | ControlCmd     |
| `ODOM_STATE`       | serial_node        | C++      | POSIX MQ  | OdomState      |

### 2.2 Cấu trúc thư mục (Phase 1)

```
brain/
├── CMakeLists.txt                 # root — add_subdirectory từng module
├── build.sh                       # cmake -S . -B build && cmake --build build
├── requirements.txt
│
├── libipc/                        # === C === core IPC
│   ├── CMakeLists.txt
│   ├── include/ipc/               # public — nodes chỉ #include từ đây
│   │   ├── bus.h                  # API: ipc_publish/subscribe/poll
│   │   └── topic.h                # TopicId enum
│   ├── src/                       # private — internal .c + .h
│   │   ├── bus.c
│   │   ├── shm.c                  # latest-wins buffer (SPMC, dùng cho CAMERA_FRAME)
│   │   ├── shm.h
│   │   ├── mq_transport.c         # POSIX MQ cho message nhỏ
│   │   ├── mq_transport.h
│   │   └── topic.c                # descriptor table + lookup
│   └── bindings/
│       ├── cpp/include/ipc/bus.hpp  # header-only, RAII + Publisher<T>/Subscriber<T>
│       └── python/
│           ├── ipc.py               # ctypes wrapper libipc.so
│           └── ipc_schema.py        # ctypes mirror của messages/*.h
│
├── messages/                      # === SOURCE OF TRUTH mọi payload ===
│   ├── CMakeLists.txt
│   ├── message.h                  # MessageHeader {topic, seq, ts_ns}
│   ├── camera_frame.h             # {w, h, stride, fmt, data_offset}
│   ├── lane_state.h               # {offset_m, heading_err_rad, curvature, conf}
│   ├── detection.h                # {class_id, tl_color, bbox[4], distance_m, conf}
│   ├── detections.h               # {count, items[MAX_DETECTIONS=32]}
│   ├── imu_state.h                # {roll, pitch, yaw, wx, wy, wz, ax, ay, az}
│   ├── wheel_odom.h               # {speed, ticks, dist}
│   ├── gps_fix.h                  # {lat, lon, hdop, sat_count, fix_type}
│   ├── ego_state.h                # {v, yaw, pitch, roll, yaw_rate}
│   └── pose2d.h                   # {x, y, heading, cov_xx, cov_yy, cov_hh}
│
├── perception/
│   ├── camera/                    # C++ — libcamera Pi 5
│   │   ├── CMakeLists.txt
│   │   ├── include/camera/capture.hpp
│   │   ├── src/capture.cpp        # libcamera API, YUV → BGR/RGB
│   │   └── app/camera_node.cpp
│   │
│   ├── lane/                      # C++ — OpenCV
│   │   ├── CMakeLists.txt
│   │   ├── include/lane/
│   │   │   ├── lane_detector.hpp
│   │   │   ├── preprocessing.hpp  # ROI + IPM + threshold
│   │   │   └── lane_tracker.hpp   # EMA/Kalman
│   │   ├── src/*.cpp
│   │   └── app/lane_node.cpp
│   │
│   ├── object_detection/          # Python — ONNXRuntime/YOLO
│   │   ├── detector.py
│   │   ├── postprocess.py         # NMS, distance ước lượng, classify_tl_color (HSV)
│   │   ├── model_loader.py
│   │   └── object_detection_node.py
│   │
│   ├── state/                     # C++ — fuse IMU + encoder
│   │   ├── CMakeLists.txt
│   │   ├── include/state/
│   │   │   ├── state_estimator.hpp
│   │   │   └── complementary_filter.hpp
│   │   ├── src/
│   │   │   ├── state_estimator.cpp
│   │   │   └── complementary_filter.cpp
│   │   └── app/state_node.cpp
│   │
│   └── localization/              # C++ — dead reckoning + GPS fusion
│       ├── CMakeLists.txt
│       ├── include/localization/
│       │   ├── dead_reckoning.hpp
│       │   └── gps_fusion.hpp
│       ├── src/*.cpp
│       └── app/localization_node.cpp
│
├── io/                            # === tầng cầu nối phần cứng / mạng ngoài ===
│   ├── serial_bridge/             # C++ — Phase 1 uplink, Phase 3 bổ sung downlink
│   │   ├── CMakeLists.txt
│   │   ├── include/serial/
│   │   │   ├── protocol.hpp       # parse @IMUR/@IMUW/@SPED/@ENCD/@GPSX...
│   │   │   ├── serial_reader.hpp
│   │   │   └── serial_writer.hpp  # để sẵn cho Phase 3
│   │   ├── src/
│   │   │   ├── protocol.cpp
│   │   │   └── serial_reader.cpp
│   │   └── app/serial_node.cpp
│   │
│   └── ws_server/                 # ☆ SKETCH — Phase 4: WebSocket cho dashboard
│       └── (chi tiết chốt sau)
│
├── config/
│   ├── config.yaml                # master: log_level, include các file con
│   ├── camera.yaml                # device, resolution, fps, format, calibration_file
│   ├── serial.yaml                # device /dev/ttyAMA0, baud 115200, timeout_ms
│   ├── lane.yaml                  # ROI, IPM points, threshold, poly degree, tracker alpha, conf_thr
│   ├── od.yaml                    # model_path, input_size, conf_thr, iou_thr, HSV params, focal_px
│   ├── state.yaml                 # alpha_yaw/pitch/roll, publish_rate_hz
│   └── localization.yaml          # accept_hdop, min_sats, origin_lat/lon, publish_rate_hz
│
├── launch/
│   └── run_perception.sh          # spawn 6 node + handle SIGINT
│
├── models/                        # ONNX weights
└── data/
    └── calibration/               # camera intrinsics, IMU bias
```

### 2.3 Serial protocol

Uplink (STM32 → Brain): prefix `@`, delimiter `::`, terminator `\r\n`
```
@IMUR::<roll>,<pitch>,<yaw>\r\n
@IMUW::<wx>,<wy>,<wz>\r\n
@IMUA::<ax>,<ay>,<az>\r\n
@SPED::<v>\r\n
@ENCD::<ticks>\r\n
@GPSX::<lat>,<lon>,<hdop>,<sat>\r\n
@BATT::<v>\r\n
@ERR::<code>\r\n
```

Downlink (Brain → STM32): prefix `#` — Phase 3
```
#SPED::<float>\r\n
#STER::<float>\r\n
#BRAK::<float>\r\n
```

`protocol.cpp` viết cả encode lẫn decode ngay từ Phase 1, Phase 1 chỉ gọi decode.

### 2.4 Message structs

```c
// message.h
typedef struct { uint32_t topic; uint32_t seq; uint64_t ts_ns; } MessageHeader;

// imu_state.h
typedef struct { MessageHeader h; float roll, pitch, yaw, wx, wy, wz, ax, ay, az; } ImuState;

// wheel_odom.h
typedef struct { MessageHeader h; float speed; int32_t ticks; float dist; } WheelOdom;

// gps_fix.h
typedef struct { MessageHeader h; double lat, lon; float hdop; uint8_t sat, fix_type; } GpsFix;

// ego_state.h
typedef struct { MessageHeader h; float v, yaw, pitch, roll, yaw_rate; } EgoState;

// pose2d.h
typedef struct { MessageHeader h; float x, y, heading; float cov_xx, cov_yy, cov_hh; } Pose2D;

// detection.h
typedef struct {
    uint8_t  class_id;   // 0..12
    uint8_t  tl_color;   // 0=none, 1=red, 2=yellow, 3=green
    uint16_t _pad;
    float    bbox[4];    // cx, cy, w, h normalized [0,1]
    float    distance_m;
    float    conf;
} Detection;

// detections.h
#define MAX_DETECTIONS 32
typedef struct { MessageHeader h; uint32_t count; Detection items[MAX_DETECTIONS]; } Detections;
```

### 2.5 Object detection node

13 class (1 model YOLO):

| id | class | id | class |
|----|-------|----|-------|
| 0 | car | 7 | highway_entry |
| 1 | pedestrian | 8 | highway_exit |
| 2 | traffic_light | 9 | roundabout_sign |
| 3 | stop_sign | 10 | no_entry |
| 4 | priority_sign | 11 | one_way |
| 5 | parking_sign | 12 | pedestrian_sign |
| 6 | crosswalk_sign | | |

Pipeline: sub `CAMERA_FRAME` → ONNX YOLOv8n → NMS → `estimate_distance` → `classify_tl_color` (HSV, chỉ khi class_id==2) → pub `DETECTIONS`.

Target: ≥ 15 Hz @ 320×320 trên Pi 5.

### 2.6 State node

- Sub `IMU_STATE` (50–100 Hz), `WHEEL_ODOM` (20–50 Hz)
- Complementary filter: `yaw ← α·(yaw + wz·dt) + (1-α)·yaw_imu`
- Pub `EGO_STATE` @ 50 Hz

### 2.7 Localization node

- Dead reckoning: `x += v·cos(yaw)·dt`, `y += v·sin(yaw)·dt`
- GPS latch khi `hdop < 2` và `sat >= 6`
- Pub `POSE2D` @ 20 Hz

### 2.8 Thứ tự triển khai Phase 1

1. **Foundation**: `messages/*.h` + `libipc/` core + bindings
2. **serial_node**: `protocol.cpp` + `serial_reader.cpp` + node
3. **camera_node**: libcamera capture + publish shm
4. **lane_node**: detector + node
5. **object_detection_node**: ONNX + postprocess + node
6. **state_node**: complementary filter + node
7. **localization_node**: dead reckoning + GPS fusion + node
8. **Integration**: `launch/run_perception.sh` chạy tất cả, verify trên xe

### 2.9 Launch

```bash
#!/usr/bin/env bash
set -euo pipefail
BIN=build/bin
rm -f /dev/shm/bfmc_* ; rm -f /dev/mqueue/* 2>/dev/null || true
trap 'kill ${PIDS[@]} 2>/dev/null; wait' INT TERM

"$BIN/serial_node"       --config config/config.yaml & PIDS+=($!)
sleep 0.3
"$BIN/camera_node"       --config config/config.yaml & PIDS+=($!)
sleep 0.3
"$BIN/lane_node"         --config config/config.yaml & PIDS+=($!)
python3 perception/object_detection/object_detection_node.py --config config/config.yaml & PIDS+=($!)
"$BIN/state_node"        --config config/config.yaml & PIDS+=($!)
"$BIN/localization_node" --config config/config.yaml & PIDS+=($!)
wait
```

---

## 3. PHASE 2 — Planning (sketch)

- `planning/`: `behavior_manager` (FSM), `local_planner`, `mission_manager`
- Sub: `LANE_STATE`, `DETECTIONS`, `EGO_STATE`, `POSE2D`
- Pub: `BEHAVIOR_CMD`

## 4. PHASE 3 — Control + serial downlink (sketch)

- `control/`: Pure Pursuit / Stanley + speed PID + watchdog
- Sub: `BEHAVIOR_CMD`, `EGO_STATE` → Pub: `CONTROL_CMD`
- `serial_node` mở downlink: `#SPED::`, `#STER::`, `#BRAK::`

## 5. PHASE 4 — Web (sketch)

- `io/ws_server/` (Python/FastAPI): sub mọi topic qua libipc → broadcast WebSocket
- `web/`: frontend static kết nối `ws://pi5:8765`

---

## 6. Giả định đã chốt

- Compute: Raspberry Pi 5, build native trên Pi
- IPC: tự viết C — SHM (latest-wins) cho CAMERA_FRAME, POSIX MQ cho message nhỏ
- Kiến trúc: multi-binary, launcher shell script
- IMU + encoder từ STM32 qua UART, protocol `@<CMD>::<val>\r\n`
- GPS: optional, localization chạy được bằng dead-reckoning thuần
- OD: Python + ONNXRuntime, YOLOv8n, 13 class, màu đèn = HSV post-process
- Lane: C++ + OpenCV, classical pipeline (IPM + threshold + polynomial)
- Tests: test trực tiếp trên xe thật, không có unit test

---

## 7. TODO — chưa plan xong

- [ ] `libipc` API signature: `ipc_publish`, `ipc_subscribe`, `ipc_poll` — chưa viết prototype
- [ ] Topic enum list trong `topic.h` — chưa gán id
- [ ] Topic descriptor table trong `topic.c` — `{transport, size, n_slots/depth, drop_policy}` per topic
- [ ] Drop policy: `CAMERA_FRAME` = latest-wins, `IMU_STATE` = drop_oldest, `CONTROL_CMD` = never-drop
- [ ] Multi-topic wait: expose fd cho epoll (state_node sub IMU + WHEEL_ODOM đồng thời)
- [ ] `config.yaml` schema chi tiết — key/value cụ thể
- [ ] Phase 2 Planning: FSM states, transitions, local planner algorithm
- [ ] Phase 3 Control: Pure Pursuit vs Stanley, PID, downlink range/units
- [ ] Phase 4 Web: WS port, JSON schema, throttling
- [ ] V2X/Environmental Server: có cần `io/v2x_node/` không
- [ ] Static map / HD map: format, lookup API, vị trí trong codebase
