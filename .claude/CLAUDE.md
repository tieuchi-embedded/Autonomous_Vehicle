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
- **Phase 1 — Perception**: đang implement Foundation
- **Phase 2/3/4**: còn sketch, chưa plan chi tiết

#### Foundation — trạng thái implement
- [x] Cấu trúc thư mục `brain/` — toàn bộ file/folder rỗng đã tạo
- [x] `messages/` — POD structs hoàn tất: `message.h`, `imu_state.h`, `wheel_odom.h`, `ego_state.h`, `camera_frame.h`, `lane_state.h`, `control_cmd.h` (stub P3)
- [x] `libipc/include/ipc/topic.h` — TopicId enum + TransportKind + DropPolicy + TopicDescriptor
- [x] `libipc/src/topic.c` — descriptor table + `topic_get()`
- [ ] `libipc/src/mq_transport.c` + `mq_transport.h`
- [ ] `libipc/src/shm.c` + `shm.h`
- [ ] `libipc/src/bus.c` + `libipc/include/ipc/bus.h`
- [ ] `libipc/bindings/cpp/include/ipc/bus.hpp`
- [ ] `libipc/bindings/python/ipc.py` + `ipc_schema.py`
- [ ] `libipc/CMakeLists.txt` + `brain/CMakeLists.txt`

#### Quyết định đã chốt (so với plan gốc)
- Bỏ: `GpsFix`, `Pose2D`, `Detection`, `Detections`, `BehaviorCmd` khỏi messages
- `EgoState` chỉ còn `yaw`, `pitch`, `roll` (bỏ `v`, `yaw_rate`)
- `LaneState` chỉ còn `heading_err_rad`
- TopicId enum bỏ prefix `TOPIC_`, DropPolicy dùng `NEW`/`OLD`/`NEVER`, TransportKind bỏ prefix `TRANSPORT_`
- POSIX MQ/SHM names: `/<topic>` (bỏ prefix `bfmc_`)

### TODO còn lại
- [ ] `config.yaml` schema key/value chi tiết
- [ ] Quyết định V2X node có cần không

---

## Tech stack
- Languages: C (C11), C++ (C++17), Python 3.12, Bash
- Build: CMake 3.26, build native trên Raspberry Pi 5
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
- Python node chưa implement — không scaffold trừ khi được yêu cầu rõ
- Solo project: **không over-engineer**, không abstraction thừa
- Không có unit test — test trên xe thật
- `messages/` là source of truth cho mọi struct — **không định nghĩa struct ở nơi khác**
- `libipc` API + topic enum đã chốt — xem `libipc/include/ipc/topic.h` và plan file trước khi implement node mới
