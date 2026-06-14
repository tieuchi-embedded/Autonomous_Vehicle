# libipc — Hướng dẫn sử dụng

IPC nội bộ cho Brain. Hai transport: **SHM** (shared memory, latest-wins) cho data lớn/liên tục, **MQ** (POSIX message queue) cho message nhỏ. Cả hai đều latest-wins: MQ đầy thì drop message cũ nhất, đẩy message mới vào cuối — không bao giờ block.

---

## Topics đã đăng ký

| TopicId | Transport | Payload |
|---------|-----------|---------|
| `CAMERA_FRAME` (1) | SHM | `CameraFrame` + pixel bytes |
| `EGO_STATE` (2) | SHM | `EgoState` |
| `LANE_STATE` (3) | SHM | `LaneState` |
| `CONTROL_CMD` (4) | MQ | `ControlCmd` |
| `OBJECT_STATE` (5) | MQ | `ObjectState` |
| `BEHAVIOUR_CMD` (6) | MQ | `BehaviourCmd` |
| `LOCATION` (7) | MQ | `Location` |

---

## C API (`ipc/bus.h`)

```c
#include "ipc/bus.h"

// Publisher
ipc_publisher_t*  ipc_publish(TopicId id, size_t payload_size);
int               ipc_push(ipc_publisher_t* pub, const void* msg, size_t size);
void              ipc_unpublish(ipc_publisher_t* pub);  // tự unlink SHM/MQ

// Subscriber
ipc_subscriber_t* ipc_subscribe(TopicId id);
// return: 0=có data, 1=timeout/no-new-data, -1=lỗi
int               ipc_pull(ipc_subscriber_t* sub, void* buf, size_t size, int timeout_ms);
void              ipc_unsubscribe(ipc_subscriber_t* sub);
```

### Ví dụ C — MQ pub/sub

```c
// Publisher
ControlCmd data = {0};
ipc_publisher_t* pub = ipc_publish(CONTROL_CMD, sizeof(ControlCmd));
data.rpm = 100.0f;
ipc_push(pub, &data, sizeof(data));
ipc_unpublish(pub);

// Subscriber
ControlCmd buf;
ipc_subscriber_t* sub = ipc_subscribe(CONTROL_CMD);
int rc = ipc_pull(sub, &buf, sizeof(buf), 1000);  // timeout 1000ms
if (rc == 0) { /* buf có data */ }
ipc_unsubscribe(sub);
```

### Ví dụ C — SHM pub (CameraFrame)

```c
const size_t pixel_bytes = 320 * 180 * 3;
const size_t payload     = sizeof(CameraFrame) + pixel_bytes;

ipc_publisher_t* pub = ipc_publish(CAMERA_FRAME, payload);

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

ipc_push(pub, buf, payload);
```

### Ví dụ C — SHM sub (CameraFrame)

```c
// Phải chạy pub trước, sub sẽ tự spin-wait cho đến khi pub tạo SHM
ipc_subscriber_t* sub = ipc_subscribe(CAMERA_FRAME);

uint8_t buf[sizeof(CameraFrame) + 320 * 180 * 3];
int rc = ipc_pull(sub, buf, sizeof(buf), 500);
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
int  pub.send(const T& msg);              // -> ipc_push

// Subscriber<T>
ipc::Subscriber<T> sub(TopicId);
bool sub.valid();
std::optional<T> sub.poll(int timeout_ms = 100);  // -> ipc_pull, nullopt = timeout
```

### Ví dụ C++ — SHM (EgoState)

```cpp
#include "ipc/bus.hpp"
#include "messages/ego_state.h"

// Publisher
ipc::Publisher<EgoState> pub(EGO_STATE);
if (!pub.valid()) { /* lỗi */ }

EgoState msg{};
msg.yaw = 45.0f;
msg.rpm = 300.0f;
pub.send(msg);

// Subscriber
ipc::Subscriber<EgoState> sub(EGO_STATE);
auto m = sub.poll(500);  // timeout 500ms
if (m) {
    printf("yaw=%.2f rpm=%.2f\n", m->yaw, m->rpm);
}
```

### Ví dụ C++ — SHM (CameraFrame)

```cpp
// Sub (pub dùng C API như camera_sim_node)
ipc::Subscriber<CameraFrame> sub(CAMERA_FRAME);
auto hdr = sub.poll(500);
// hdr chỉ là struct header, không chứa pixel data
// Để đọc pixel, dùng C API ipc_pull với buffer đủ lớn (xem lane_node.cpp)
```

> **Lưu ý SHM sub pixel data**: `Subscriber<CameraFrame>` chỉ đọc header struct.
> Để đọc cả pixel, dùng C API (`ipc_pull`) với raw buffer như trong `lane_node.cpp`:
> ```cpp
> ipc_subscriber_t* cam_sub = ipc_subscribe(CAMERA_FRAME);
> std::vector<uint8_t> buf(sizeof(CameraFrame) + W * H * 3);
> ipc_pull(cam_sub, buf.data(), buf.size(), 500);
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

### TopicId trong Python

```python
schema.CAMERA_FRAME   # 1
schema.EGO_STATE      # 2
schema.LANE_STATE     # 3
schema.CONTROL_CMD    # 4
schema.OBJECT_STATE   # 5
schema.BEHAVIOUR_CMD  # 6
schema.LOCATION       # 7
```

### Ví dụ Python

```python
import ctypes as ct

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
        print(f"heading={msg.heading_err_rad:.4f} offset={msg.offset_cm:.1f}")
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
| `EgoState` | yaw, pitch, roll (deg), rpm — raw telemetry từ serial |
| `LaneState` | heading_err_rad, offset_cm (+right) |
| `CameraFrame` | width, height, stride, fmt (0=BGR), data_size |
| `ControlCmd` | rpm (-500..+500), steer_deg (-30..+30) |
| `ObjectState` | cls, distance (m), confidence |
| `BehaviourCmd` | target_speed (m/s), target_heading (deg), mode |
| `Location` | x, y (m), heading (rad) |

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
2. **`ipc_unpublish` tự cleanup** — không cần xóa `/dev/mqueue/*` hay `/dev/shm/*` thủ công.
3. **`messages/` là source of truth** — không định nghĩa lại struct ở nơi khác.
4. **CameraFrame payload = header + pixel bytes** — `payload_size = sizeof(CameraFrame) + stride * height`.
5. **SHM sub đọc pixel**: dùng C API `ipc_pull` với raw buffer, không dùng `Subscriber<CameraFrame>` template.
6. **Latest-wins, không block** — MQ đầy thì drop message cũ nhất; không có drop policy cấu hình.
