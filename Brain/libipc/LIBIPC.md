# libipc — Hướng dẫn sử dụng

IPC nội bộ cho Brain. Hai transport: **SHM** (shared memory, latest-wins) cho data lớn/liên tục, **MQ** (POSIX message queue) cho message nhỏ.

---

## Topics đã đăng ký

| TopicId | Transport | DropPolicy | Payload |
|---------|-----------|------------|---------|
| `CAMERA_FRAME` (1) | SHM | NEW | `CameraFrame` + pixel bytes |
| `SERIAL` (2) | MQ | OLD | `SerialData` |
| `EGO_STATE` (3) | MQ | OLD | `EgoState` |
| `LANE_STATE` (4) | MQ | NEW | `LaneState` |
| `CONTROL_CMD` (5) | MQ | NEW | `ControlCmd` |
| `DETECTIONS` (6) | MQ | NEW | `Detections` |
| `BEHAVIOR_CMD` (7) | MQ | NEVER | `BehaviorCmd` |
| `POSE2D` (8) | MQ | OLD | `Pose2D` |

**DropPolicy:**
- `NEW` — queue đầy: drop message mới nhất vừa gửi
- `OLD` — queue đầy: drain cũ nhất rồi gửi lại
- `NEVER` — queue đầy: block tối đa 5s, trả lỗi nếu vẫn đầy

---

## C API (`ipc/bus.h`)

```c
#include "ipc/bus.h"

// Publisher
ipc_publisher_t*  ipc_publish_open(TopicId id, size_t payload_size);
int               ipc_publish(ipc_publisher_t* pub, const void* msg, size_t size);
void              ipc_publish_close(ipc_publisher_t* pub);  // tự unlink SHM/MQ

// Subscriber
ipc_subscriber_t* ipc_subscribe_open(TopicId id);
// return: 0=có data, 1=timeout/no-new-data, -1=lỗi
int               ipc_poll(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms);
void              ipc_subscribe_close(ipc_subscriber_t* sub);
```

### Ví dụ C — MQ pub/sub

```c
// Publisher
SerialData data = {0};
ipc_publisher_t* pub = ipc_publish_open(SERIAL, sizeof(SerialData));
data.yaw = 45.0f;
ipc_publish(pub, &data, sizeof(data));
ipc_publish_close(pub);

// Subscriber
SerialData buf;
ipc_subscriber_t* sub = ipc_subscribe_open(SERIAL);
int rc = ipc_poll(sub, &buf, sizeof(buf), 1000);  // timeout 1000ms
if (rc == 0) { /* buf có data */ }
ipc_subscribe_close(sub);
```

### Ví dụ C — SHM pub (CameraFrame)

```c
const size_t pixel_bytes = 320 * 180 * 3;
const size_t payload     = sizeof(CameraFrame) + pixel_bytes;

ipc_publisher_t* pub = ipc_publish_open(CAMERA_FRAME, payload);

uint8_t buf[payload];
CameraFrame* hdr = (CameraFrame*)buf;
uint8_t* px      = buf + sizeof(CameraFrame);

hdr->h.topic   = CAMERA_FRAME;
hdr->width     = 320;
hdr->height    = 180;
hdr->stride    = 320 * 3;
hdr->fmt       = 0;  // BGR
hdr->data_size = (uint32_t)pixel_bytes;
memcpy(px, frame_data, pixel_bytes);

ipc_publish(pub, buf, payload);
```

### Ví dụ C — SHM sub (CameraFrame)

```c
// Phải chạy pub trước, sub sẽ tự spin-wait cho đến khi pub tạo SHM
ipc_subscriber_t* sub = ipc_subscribe_open(CAMERA_FRAME);

uint8_t buf[sizeof(CameraFrame) + 320 * 180 * 3];
int rc = ipc_poll(sub, buf, sizeof(buf), 500);
if (rc == 0) {
    CameraFrame* hdr = (CameraFrame*)buf;
    uint8_t* px      = buf + sizeof(CameraFrame);
    // hdr->width, hdr->height, px...
}
```

---

## C++ API (`ipc/bus.hpp`)

Header-only, RAII. Include thêm `ipc/bus.hpp` (tự include `ipc/bus.h`).

```cpp
#include "ipc/bus.hpp"

// Publisher<T>
ipc::Publisher<T> pub(TopicId);           // payload_size = sizeof(T) mặc định
ipc::Publisher<T> pub(TopicId, size);     // custom size cho CameraFrame
bool pub.valid();
int  pub.send(const T& msg);

// Subscriber<T>
ipc::Subscriber<T> sub(TopicId);
bool sub.valid();
std::optional<T> sub.poll(int timeout_ms = 100);  // nullopt = timeout
```

### Ví dụ C++ — MQ

```cpp
#include "ipc/bus.hpp"
#include "messages/ego_state.h"

// Publisher
ipc::Publisher<EgoState> pub(EGO_STATE);
if (!pub.valid()) { /* lỗi */ }

EgoState msg{};
msg.h.topic = EGO_STATE;
msg.time_ms = 1000;
msg.angle   = 45.0f;
msg.speed   = 30.0f;
pub.send(msg);

// Subscriber
ipc::Subscriber<EgoState> sub(EGO_STATE);
auto msg = sub.poll(500);  // timeout 500ms
if (msg) {
    printf("angle=%.2f speed=%.2f\n", msg->angle, msg->speed);
}
```

### Ví dụ C++ — SHM (CameraFrame)

```cpp
// Sub (pub dùng C API như camera_sim_node)
ipc::Subscriber<CameraFrame> sub(CAMERA_FRAME);
auto hdr = sub.poll(500);
// hdr chỉ là struct header, không chứa pixel data
// Để đọc pixel, dùng C API ipc_poll với buffer đủ lớn (xem lane_node.cpp)
```

> **Lưu ý SHM sub pixel data**: `Subscriber<CameraFrame>` chỉ đọc header struct.
> Để đọc cả pixel, dùng C API (`ipc_poll`) với raw buffer như trong `lane_node.cpp`:
> ```cpp
> ipc_subscriber_t* cam_sub = ipc_subscribe_open(CAMERA_FRAME);
> std::vector<uint8_t> buf(sizeof(CameraFrame) + W * H * 3);
> ipc_poll(cam_sub, buf.data(), buf.size(), 500);
> const CameraFrame* hdr = reinterpret_cast<const CameraFrame*>(buf.data());
> const uint8_t*     px  = buf.data() + sizeof(CameraFrame);
> ```

---

## Python API (`ipc.py` + `ipc_schema.py`)

```python
import sys
sys.path.insert(0, "Brain/libipc/bindings/python")

import ipc_schema as schema
from ipc import Publisher, Subscriber
```

**Chưa verify trên xe** — xem `test/Python_test/python_test.md`.

> **Lưu ý**: `ipc_schema.py` hiện chưa sync với struct thực tế (`SerialData`, `EgoState` mới).
> Trước khi dùng, kiểm tra lại các field.

### TopicId trong Python

```python
schema.CAMERA_FRAME  # 1
schema.IMU_STATE     # 2  (cũ — thực tế là SERIAL)
schema.EGO_STATE     # 4  (cũ — thực tế là 3)
schema.LANE_STATE    # 5  (cũ — thực tế là 4)
```

> **TODO**: `ipc_schema.py` cần update lại TopicId và struct cho khớp với `topic.h`.

### Ví dụ Python

```python
# Publisher
pub = Publisher(schema.EGO_STATE, payload_size=ct.sizeof(schema.EgoState))
msg = schema.EgoState()
msg.yaw = 45.0
pub.publish(msg)  # tự fill MessageHeader (topic, seq, ts_ns)
pub.close()

# Subscriber (context manager)
with Subscriber(schema.LANE_STATE, schema.LaneState) as sub:
    msg = sub.poll(timeout_ms=1000)
    if msg:
        print(f"heading={msg.heading_err_rad:.4f}")
```

---

## Message structs (`messages/*.h`)

Tất cả message đều bắt đầu bằng `MessageHeader`:

```c
typedef struct {
    uint32_t topic;   // TopicId
    uint32_t seq;     // monotonic counter
    uint64_t ts_ns;   // CLOCK_MONOTONIC nanoseconds
} MessageHeader;
```

| Struct | Fields |
|--------|--------|
| `SerialData` | yaw, pitch, roll, wx, wy, wz, ax, ay, az, rpm |
| `EgoState` | time_ms, angle (deg), speed (cm/s) |
| `LaneState` | heading_err_rad, lateral_offset_m (+right) |
| `CameraFrame` | width, height, stride, fmt (0=BGR), data_size |
| `ControlCmd` | rpm (-500..+500), steer_deg (-30..+30) |
| `BehaviorCmd` | target_speed (m/s), target_heading (deg), mode |

---

## Build & Link

```cmake
# Trong CMakeLists.txt của node
target_link_libraries(my_node PRIVATE libipc)
# libipc tự expose: messages/, libipc/include/, libipc/bindings/cpp/include/
```

Build:

```bash
cmake -S Brain -B Brain/build
cmake --build Brain/build
```

Binary output: `Brain/build/bin/`

---

## Quy tắc quan trọng

1. **Pub trước sub** — SHM sub sẽ spin-wait cho đến khi pub tạo SHM. Nếu chạy sub trước pub, sub block vô hạn.
2. **`ipc_publish_close` tự cleanup** — không cần xóa `/dev/mqueue/*` hay `/dev/shm/*` thủ công.
3. **`messages/` là source of truth** — không định nghĩa lại struct ở nơi khác.
4. **CameraFrame payload = header + pixel bytes** — `payload_size = sizeof(CameraFrame) + stride * height`.
5. **SHM sub đọc pixel**: dùng C API `ipc_poll` với raw buffer, không dùng `Subscriber<CameraFrame>` template.
