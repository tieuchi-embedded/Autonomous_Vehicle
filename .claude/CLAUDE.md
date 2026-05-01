# Global
## Personal Preferrence
Ngôn ngữ: trả lời tiếng Việt, code/comments tiếng Anh
Tone: technical, concise — không verbose
Luôn trả lời ngắn gọn nhất có thể, không mất thời gian và token
Code style: minimal changes, đừng refactor những gì không liên quan
Khi không chắc: hỏi trước, đừng assume
Errors: luôn tìm root cause, không band-aid fixes

# Project

## Overview
Dự án phát triển xe tự hành tỷ lệ 1:10 tham gia cuộc thi BFMC của Bosch, bao gồm các phần từ camera, cảm biến, giao tiếp các module percieption, planning, control, và điều khiển cơ cấu chấp hành, mục tiêu xa hơn là viết nghiên cứu khoa học và đăng paper IEEE

---

## System Architecture
Không đọc phần Embedded platform
**Plan (source of truth):** [.claude/plans/brain-architecture-phased.md](.claude/plans/brain-architecture-phased.md) — đọc file này mỗi khi bắt đầu session mới trước khi implement.

### Kiến trúc chốt
- **Multi-binary**: mỗi node là 1 process độc lập, giao tiếp qua `libipc` (SHM + POSIX MQ)
- **Phased**: Phase 1 Perception → Phase 2 Planning → Phase 3 Control → Phase 4 Web
- **Build**: native trên Pi 5, `cmake -S . -B build && cmake --build build`
- **Test**: trực tiếp trên xe thật, không có unit test

### Cấu trúc thư mục `brain/`
```
brain/
├── libipc/          # C — SHM (latest-wins) + POSIX MQ + bindings cpp/python
├── messages/        # C headers — POD structs (source of truth mọi payload)
├── perception/      # camera/ lane/ object_detection/ state/ localization/
├── io/              # serial_bridge/ (UART↔STM32), ws_server/ (Phase 4)
├── config/          # camera.yaml, serial.yaml, lane.yaml, od.yaml, state.yaml, localization.yaml
├── launch/          # run_perception.sh
├── models/          # ONNX weights
└── data/calibration/
```

### Tiến độ
- **Phase 1 — Perception**: Foundation **hoàn tất**, đã verify trên máy dev
- **Phase 2/3/4**: còn sketch, chưa plan chi tiết

#### Foundation — trạng thái implement
- [x] Cấu trúc thư mục `Brain/` (chú ý: viết hoa B)
- [x] `messages/` — POD structs: `message.h`, `serial_data.h`, `ego_state.h`, `camera_frame.h`, `lane_state.h`, `control_cmd.h`
- [x] `libipc/include/ipc/topic.h` — TopicId enum + TransportKind + DropPolicy + TopicDescriptor
- [x] `libipc/src/topic.c` — descriptor table + `topic_get()`
- [x] `libipc/src/mq_transport.c` + `mq_transport.h` — POSIX MQ wrap, clamp depth theo `/proc/sys/fs/mqueue/msg_max`
- [x] `libipc/src/shm.c` + `shm.h` — SHM latest-wins, sub tự chờ pub init header
- [x] `libipc/src/bus.c` + `libipc/include/ipc/bus.h` — unified API, `extern "C"` guard
- [x] `libipc/bindings/cpp/include/ipc/bus.hpp` — `Publisher<T>` / `Subscriber<T>` RAII
- [x] `libipc/CMakeLists.txt` — build `liblibipc.a` (static) + `liblibipc.so` (shared cho Python), expose `bindings/cpp/include`
- [x] `libipc/bindings/python/ipc_schema.py` — ctypes.Structure mapping messages/*.h + TopicId constants
- [x] `libipc/bindings/python/ipc.py` — Publisher/Subscriber class, auto-fill MessageHeader, context manager
- [x] `io/serial_bridge/` — `protocol.cpp` parse `@IMU`/`@RPM`, `serial_reader.cpp` UART termios, `serial_node` pub `SERIAL`
- [x] `perception/state/` — `StateEstimator` sub `SERIAL` → compute `angle`+`speed` → pub `EGO_STATE`
- [x] `perception/camera_sim/` — `camera_sim_node` đọc MP4 → resize 320x240 → pub `CAMERA_FRAME` (SHM), loop video
- [x] `perception/lane/` — IPM + HLS binary + sliding window + polyfit bậc 2 → pub `LANE_STATE`

#### Verify đã làm
- MQ: `pub_demo` ↔ `sub_demo` (C và C++) ✓
- SHM: `shm_pub_demo` ↔ `shm_sub_demo` (C và C++) ✓
- `serial_node` + `sub_demo` — truyền `SerialData` từ STM32 thật qua `/dev/ttyACM0` ✓
- `camera_sim_node` + `lane_node --show` — pipeline video→lane detect chạy trên máy dev ✓
- Build clean toàn bộ target trên máy dev ✓
- Python bindings chưa verify trên xe — xem `demo/Python_demo/python_demo.md`

#### Quyết định đã chốt
- Bỏ `ImuState`, `WheelOdom` — thay bằng `SerialData` (raw UART data)
- `SerialData` = `{yaw, pitch, roll, wx, wy, wz, ax, ay, az, rpm}` — pub bởi `serial_node`, wx/wy/wz/ax/ay/az tạm = 0
- `EgoState` = `{time_ms, angle, speed}` — pub bởi `state_node`
  - `time_ms`: ms kể từ khi `state_node` start
  - `angle`: yaw degrees pass-through từ IMU
  - `speed`: cm/s = `(rpm / 7) * 2π * 6.5 / 60`
- `LaneState` = `{heading_err_rad, lateral_offset_m}` — pub bởi `lane_node`
  - `heading_err_rad`: góc tiếp tuyến lane tham chiếu vs trục xe (atan của derivative poly tại bottom)
  - `lateral_offset_m`: khoảng cách ngang tâm xe vs tâm lane (m), +right
- TopicId hiện tại: `CAMERA_FRAME=1, SERIAL=2, EGO_STATE=3, LANE_STATE=4, CONTROL_CMD=5`
- UART protocol STM32: `@IMU:yaw,pitch,roll;;\r\n` và `@RPM:rpm;;\r\n` (degrees, motor RPM)
- Hardware: gear ratio 7:1, wheel radius 6.5cm, `/dev/ttyACM0`, baud 115200
- Bỏ GPS (không có hardware) — `localization_node` chỉ dead reckoning
- V2X node: defer, chưa cần
- TopicId enum bỏ prefix `TOPIC_`, DropPolicy dùng `NEW`/`OLD`/`NEVER`
- POSIX MQ/SHM names: `/<topic>` (bỏ prefix `bfmc_`)
- MQ depth clamp xuống `msg_max` hệ thống (mặc định 10 trên Ubuntu desktop)
- **SHM cleanup**: `ipc_publish_close` tự unlink MQ/SHM — không cần `rm -f /dev/mqueue/* /dev/shm/*` thủ công
- **SHM sub**: sub chờ pub tạo SHM trước (không tự create/ftruncate) — **phải chạy pub trước sub**

#### lane_node — chi tiết implement
- **Pipeline**: `CAMERA_FRAME (SHM)` → IPM warp → HLS binary (L channel only, L_MIN=200) → sliding window → polyfit bậc 2 → EMA tracker → `LANE_STATE (MQ)`
- **Ưu tiên lane**: right lane cho heading, fallback left khi right mất, tự recover về right
- **`LaneSource` enum**: `RIGHT`, `LEFT`, `NONE` — expose trong `DetectResult`
- **Debug `--show`**: 3 đường trên warped image:
  - Xanh lá (cam nếu fallback): LINE 1 bám poly lane tham chiếu
  - Trắng: LINE 2 thẳng đứng giữa ảnh (thân xe)
  - Vàng: LINE 3 song song LINE 1, dính LINE 2 tại bottom
  - Đỏ đáy: offset segment LINE 1 ↔ LINE 2
- **IPM constants** (tune theo video `2p_haveturn.mp4`, 320x240):
  - `SRC_TL_X=0.15, SRC_TR_X=0.85, SRC_TOP_Y=0.30`
  - `SRC_BL_X=0.00, SRC_BR_X=1.00, SRC_BOT_Y=0.80`
- **Còn cần tune**: `PIXEL_TO_M=0.005` chưa calibrate thực tế

#### libipc — bug fix đã làm
- `shm.c shm_open_sub`: bỏ `ftruncate` — sub chỉ open (không create), tránh shrink SHM của pub gây Bus error
- `bus.c ipc_publish_close`: thêm `shm_unlink_topic` / `mq_transport_unlink` để tự cleanup khi pub đóng

### TODO còn lại (Phase 1)
- [ ] **Verify Python bindings**: cross-test `pub_demo.py` ↔ C `sub_demo` — xem `demo/Python_demo/python_demo.md`
- [ ] **`ipc_schema.py`** cập nhật cho khớp `SerialData`/`EgoState`/`LaneState` mới (chưa sync)
- [ ] **Tune lane_node**: `PIXEL_TO_M`, IPM points trên xe thật (góc camera thật khác video)
- [ ] `camera_node`: libcamera capture thật + publish SHM (thay `camera_sim_node` khi có camera)
- [ ] `object_detection_node`: ONNX YOLOv8n + postprocess + node
- [ ] `localization_node`: dead reckoning, sub `EGO_STATE`
- [ ] `launch/run_perception.sh` — integration tất cả node

---

## Tech stack
- Languages: C (C11), C++ (C++17), Python 3.12, Bash
- Build: CMake 3.22+ (máy dev), build native trên Raspberry Pi 5
- Target: Raspberry Pi 5 (Ubuntu 24.04) + STM32F4 (ARM Cortex-M4)
- IPC: tự viết — SHM latest-wins + POSIX MQ (không dùng ROS)
- Testing: xe thật, không unit test
---

## Hardware Context
- Board: [STM32F4 / Raspberry Pi / custom board]
- Peripherals: UART, SPI, I2C, GPIO, ADC
- RTOS: FreeRTOS / bare-metal / Linux
---

## Hard Rules (KHÔNG được vi phạm)
- KHÔNG dùng malloc/free trong ISR context
- KHÔNG blocking calls (sleep, mutex_lock vô hạn) trong real-time tasks
- KHÔNG hardcode địa chỉ memory — dùng defines hoặc linker script
- KHÔNG bỏ qua return values của system calls
- KHÔNG commit code chưa qua lint + test
---

## Things Claude Should Know

- Primary runtime: **Raspberry Pi 5 / Ubuntu 24.04**, build native trên Pi
- STM32F4: Cortex-M4, giao tiếp qua UART với Brain — không code trực tiếp trừ khi được yêu cầu
- IPC: tự viết C, **không gợi ý ROS hay middleware bên ngoài**
- Python bindings (`ipc.py`, `ipc_schema.py`) đã implement, chưa verify trên xe
- Python import pattern: `sys.path.insert(0, "Brain/libipc/bindings/python")` rồi `import ipc, ipc_schema` (không dùng package import)
- Solo project: **không over-engineer**, không abstraction thừa
- Không có unit test — test trên xe thật
- `messages/` là source of truth cho mọi struct — **không định nghĩa struct ở nơi khác**
- `libipc` API + topic enum đã chốt — xem `libipc/include/ipc/topic.h` và plan file trước khi implement node mới
