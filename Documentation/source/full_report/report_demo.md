# Báo cáo Kỹ thuật — Hệ thống Brain BFMC

## HARDWARE

### Embedded Computer
| Board | Vai trò |
|-------|---------|
| Raspberry Pi 5 | Main Brain — chạy toàn bộ software stack |
| Jetson Nano / Orin | Dự phòng, chưa dùng trong build hiện tại |

### MCU
| Board | Vai trò |
|-------|---------|
| STM32F401 Nucleo | Điều khiển cơ cấu chấp hành (speed/steering servo) + đọc cảm biến |
| STM32F411 Black Pill | Dự phòng |
| STM32G0B1 Nucleo | Dự phòng |

MCU giao tiếp với Brain qua **UART** (`/dev/ttyACM0`, baud 115200). Không dùng ROS hay middleware — Brain và MCU trao đổi qua text protocol tự định nghĩa.

### Cảm biến
| Cảm biến | Model | Giao tiếp | Dữ liệu |
|----------|-------|-----------|---------|
| IMU | BNO055 | I2C → STM32 | yaw, pitch, roll (degrees) |
| Encoder | AS5600 | I2C → STM32 | motor RPM |

### Cơ cấu chấp hành
- **Speed Servo**: điều khiển tốc độ động cơ (PWM từ STM32)
- **Steering Servo**: lái bánh trước (PWM từ STM32)
- Gear ratio: **7:1**, wheel radius: **6.5 cm**

---

## SOFTWARE

### ARCHITECTURE

#### Flow tổng quan

```
STM32 ──UART──▶ serial_node ──▶ SERIAL (MQ)
                                     │
                                     ▼
                              state_node ──▶ EGO_STATE (MQ)
                                                  │
libcamera ──▶ camera_node ──▶ CAMERA_FRAME (SHM) ─┤
                                     │             │
                                     ▼             ▼
                              lane_node ──▶ LANE_STATE (MQ)
                              object_detection_node ──▶ DETECTIONS (MQ)
                                                        │
                                     [Phase 2] planning_node ──▶ BEHAVIOR_CMD (MQ)
                                                                        │
                                              [Phase 3] control_node ──▶ CONTROL_CMD (MQ)
                                                                              │
                                                               serial_node ──UART──▶ STM32
```

#### Kiến trúc layer

| Layer | Nội dung |
|-------|---------|
| **Hardware / IO** | STM32 (MCU), camera (libcamera), UART |
| **libipc** | IPC kernel: SHM latest-wins + POSIX MQ, C11 |
| **messages** | POD C structs — source of truth mọi payload |
| **Nodes** | Multi-binary executables — mỗi node 1 process độc lập |
| **Config** | YAML per-node, load lúc startup |

**Multi-binary** có nghĩa mỗi node (`serial_node`, `lane_node`, …) là một executable riêng. Nodes không link chung process, không share memory trực tiếp — toàn bộ giao tiếp qua `libipc`.

#### libipc — IPC tự viết

`libipc` cung cấp 2 transport:

| Transport | Dùng cho | Đặc điểm |
|-----------|---------|-----------|
| **SHM** (Shared Memory) | `CAMERA_FRAME` | Latest-wins ring, SPMC, subscriber tự chờ publisher init |
| **POSIX MQ** | Mọi message nhỏ còn lại | Queue depth clamp theo `/proc/sys/fs/mqueue/msg_max` |

API C (`bus.h`):
```c
int  ipc_publish_open(TopicId id, IpcPubHandle *h);
int  ipc_publish(IpcPubHandle *h, const void *data, size_t len);
void ipc_publish_close(IpcPubHandle *h);   // tự unlink SHM/MQ

int  ipc_subscribe_open(TopicId id, IpcSubHandle *h);
int  ipc_poll(IpcSubHandle *h, void *buf, size_t len, int timeout_ms);
void ipc_subscribe_close(IpcSubHandle *h);
```

C++ binding (`bus.hpp`) bọc RAII:
```cpp
ipc::Publisher<LaneState>   pub(LANE_STATE);
ipc::Subscriber<SerialData> sub(SERIAL);
SerialData msg;
sub.poll(msg, 100); // 100ms timeout
```

Python binding (`ipc.py` + `ipc_schema.py`): ctypes wrapper, map struct từ `messages/*.h`.

#### Topic table

| TopicId | Producer | Transport | Payload struct |
|---------|---------|-----------|---------------|
| `CAMERA_FRAME = 1` | camera_node / camera_sim_node | SHM | `CameraFrame` |
| `SERIAL = 2` | serial_node | POSIX MQ | `SerialData` |
| `EGO_STATE = 3` | state_node | POSIX MQ | `EgoState` |
| `LANE_STATE = 4` | lane_node | POSIX MQ | `LaneState` |
| `CONTROL_CMD = 5` | control_node | POSIX MQ | `ControlCmd` |

#### Message structs (`messages/*.h`)

```c
// message.h — header chung mọi message
typedef struct {
    uint32_t topic;
    uint32_t seq;
    uint64_t ts_ns;
} MessageHeader;

// serial_data.h — raw UART từ STM32
typedef struct {
    MessageHeader h;
    float yaw, pitch, roll;       // degrees (BNO055)
    float wx, wy, wz;             // rad/s (tạm = 0)
    float ax, ay, az;             // m/s² (tạm = 0)
    float rpm;                    // motor RPM
} SerialData;

// ego_state.h — trạng thái xe
typedef struct {
    MessageHeader h;
    uint32_t time_ms;   // ms kể từ khi state_node start
    float angle;        // yaw pass-through (degrees)
    float speed;        // cm/s = (rpm/7) * 2π * 6.5 / 60
} EgoState;

// camera_frame.h — frame từ camera
typedef struct {
    MessageHeader h;
    uint32_t w, h, stride;
    uint32_t fmt;            // pixel format code
    uint32_t data_offset;    // offset vào SHM block
} CameraFrame;

// lane_state.h — kết quả lane detection
typedef struct {
    MessageHeader h;
    float heading_err_rad;   // góc tiếp tuyến lane vs trục xe, atan(poly'(bottom))
    float lateral_offset_m;  // khoảng cách ngang tâm xe vs tâm lane (m), +right
} LaneState;

// control_cmd.h
typedef struct {
    MessageHeader h;
    float speed;    // m/s target
    float steer;    // [-1, +1]
    float brake;    // [0, 1]
} ControlCmd;
```

#### UART Protocol (STM32 ↔ Brain)

Uplink (STM32 → Brain):
```
@IMU:yaw,pitch,roll;;\r\n     — góc Euler từ BNO055 (degrees)
@RPM:rpm;;\r\n                 — motor RPM
```

Downlink (Brain → STM32, Phase 3):
```
#SPED::<float>\r\n   — speed setpoint
#STER::<float>\r\n   — steering angle
#BRAK::<float>\r\n   — brake
```

`protocol.cpp` implement cả encode lẫn decode từ Phase 1; Phase 1 chỉ gọi decode.

---

### PERCEPTION

#### serial_node

- Đọc UART `/dev/ttyACM0` (115200 baud) qua `termios`
- Parse line theo prefix `@IMU` / `@RPM`
- Publish `SerialData` lên topic `SERIAL` (POSIX MQ)

#### state_node

- Sub `SERIAL` (100 Hz)
- `angle = yaw` (pass-through từ IMU)
- `speed (cm/s) = (rpm / 7) × 2π × 6.5 / 60`
- Publish `EgoState` lên topic `EGO_STATE`

#### camera_node / camera_sim_node

- **camera_node** (production): libcamera API trên Pi 5, capture YUV → publish SHM
- **camera_sim_node** (dev): đọc file MP4, resize 320×240, loop video, publish SHM
- Publish `CameraFrame` lên topic `CAMERA_FRAME` (SHM latest-wins)

#### lane_node

Pipeline:
```
CAMERA_FRAME (SHM)
  → IPM warp (perspective → bird's-eye)
  → HLS binary (L channel only, L_MIN = 200)
  → Sliding window (left + right lane)
  → Polyfit bậc 2 (numpy-style trên C++)
  → EMA tracker (alpha smoothing)
  → LaneState pub (POSIX MQ)
```

IPM constants (tune theo video `2p_haveturn.mp4`, 320×240):

| Điểm | X | Y |
|------|---|---|
| Top-left src | 0.15 | 0.30 |
| Top-right src | 0.85 | 0.30 |
| Bot-left src | 0.00 | 0.80 |
| Bot-right src | 1.00 | 0.80 |

Ưu tiên lane: dùng **right lane** làm tham chiếu, fallback sang left khi right mất, tự recover về right.

`PIXEL_TO_M = 0.005` — chưa calibrate thực tế, cần đo lại trên xe thật.

Debug flag `--show`: hiển thị 3 đường trên warped image:
- **Xanh lá** (cam nếu fallback): poly lane tham chiếu
- **Trắng**: thẳng đứng giữa ảnh (trục xe)
- **Vàng**: song song lane tham chiếu, dính trục xe tại bottom
- **Đỏ đáy**: segment offset lane vs trục xe

#### object_detection_node (Phase 1 — todo)

- Python + ONNXRuntime, model YOLOv8n 13 class
- Sub `CAMERA_FRAME` → ONNX inference @ 320×320 → NMS → distance estimate → HSV color classify (chỉ `traffic_light`) → pub `DETECTIONS`
- Target: ≥ 15 Hz trên Pi 5

| id | class | id | class |
|----|-------|----|-------|
| 0 | car | 7 | highway_entry |
| 1 | pedestrian | 8 | highway_exit |
| 2 | traffic_light | 9 | roundabout_sign |
| 3 | stop_sign | 10 | no_entry |
| 4 | priority_sign | 11 | one_way |
| 5 | parking_sign | 12 | pedestrian_sign |
| 6 | crosswalk_sign | | |

---

### LOCALIZATION

#### localization_node (Phase 1 — todo)

- Dead reckoning thuần: `x += v·cos(yaw)·dt`, `y += v·sin(yaw)·dt`
- Sub `EGO_STATE` @ 20 Hz
- Không có GPS hardware → bỏ GPS fusion
- Publish `Pose2D {x, y, heading}` lên topic `POSE2D`

---

### PLANNING (Phase 2 — sketch)

- `behavior_manager`: FSM các trạng thái lái (follow lane, stop at sign, intersection handling…)
- `local_planner`: tính trajectory ngắn hạn
- `mission_manager`: quản lý mission goal
- Sub: `LANE_STATE`, `DETECTIONS`, `EGO_STATE`, `POSE2D`
- Pub: `BEHAVIOR_CMD`

---

### CONTROL (Phase 3 — sketch)

- Thuật toán: Pure Pursuit hoặc Stanley (chưa chốt) + speed PID
- Sub `BEHAVIOR_CMD`, `EGO_STATE` → tính `steer`, `speed`, `brake`
- Pub `CONTROL_CMD` → `serial_node` downlink → STM32

---

### BUILD & LAUNCH

```bash
# Build
cmake -S Brain -B Brain/build && cmake --build Brain/build

# Chạy toàn bộ Perception (Phase 1)
bash Brain/launch/run_perception.sh
```

Thứ tự khởi động: `serial_node` → `camera_node/sim` → `lane_node` → `object_detection_node` → `state_node` → `localization_node`.
**Lưu ý**: publisher phải start trước subscriber tương ứng (SHM sub chờ pub tạo block).

---

### TRẠNG THÁI VERIFY

| Hạng mục | Trạng thái |
|----------|-----------|
| POSIX MQ pub/sub (C và C++) | ✅ Verify máy dev |
| SHM pub/sub (C và C++) | ✅ Verify máy dev |
| serial_node ↔ STM32 thật | ✅ `/dev/ttyACM0` |
| camera_sim + lane_node --show | ✅ Verify máy dev |
| Build clean toàn bộ target | ✅ Máy dev |
| Python bindings | ⏳ Chưa verify trên xe |
| object_detection_node | ⏳ Chưa implement |
| localization_node | ⏳ Chưa implement |
| camera_node (libcamera thật) | ⏳ Chờ có camera |
| Tune lane PIXEL_TO_M, IPM | ⏳ Cần xe thật |

---

## SIMULATION

*(Chưa có — tất cả test trên xe thật)*

---

## PROJECT

- Cuộc thi: **BFMC (Bosch Future Mobility Challenge)** — xe tự hành tỷ lệ 1:10
- Mục tiêu dài hạn: viết research paper, nộp IEEE
- Runtime: Raspberry Pi 5 / Ubuntu 24.04
- Build: CMake 3.22+, C11 / C++17 / Python 3.12
- IPC: tự viết (`libipc`) — **không dùng ROS**
- Test: trực tiếp trên xe thật
