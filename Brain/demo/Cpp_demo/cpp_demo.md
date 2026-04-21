# libipc C++ Demo Guide

Hướng dẫn dùng C++ RAII wrapper (`Publisher<T>` / `Subscriber<T>`) của `libipc`.

---

## Build

```bash
cd Brain
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Binaries ra tại `Brain/build/bin/`, prefix `cpp_`.

---

## Transport 1: POSIX MQ — `ImuState`

### Publisher

```cpp
#include "ipc/bus.hpp"
#include "messages/imu_state.h"

ipc::Publisher<ImuState> pub(IMU_STATE);
if (!pub.valid()) { /* error */ }

ImuState msg = {
    .h     = { .topic = IMU_STATE, .seq = seq++, .ts_ns = ... },
    .roll  = 0.1f, .pitch = 0.2f, .yaw = 0.3f,
    .az    = 9.81f,
};
pub.send(msg);
// destructor tự gọi ipc_publish_close khi ra khỏi scope
```

### Subscriber

```cpp
#include "ipc/bus.hpp"
#include "messages/imu_state.h"

ipc::Subscriber<ImuState> sub(IMU_STATE);
if (!sub.valid()) { /* error */ }

auto msg = sub.poll(2000); // timeout_ms=2000
if (msg)
    printf("seq=%u roll=%.2f\n", msg->h.seq, msg->roll);
else
    printf("timeout\n");
// poll trả std::optional<T> — nullopt nếu timeout hoặc lỗi
```

### Chạy demo

```bash
# Terminal 1
./Brain/build/bin/cpp_sub_demo

# Terminal 2
./Brain/build/bin/cpp_pub_demo
```

Expected output sub:
```
subscribing IMU_STATE via libipc C++ Subscriber...
sub seq=0 roll=0.00 pitch=0.00 yaw=0.00 az=9.81
sub seq=1 roll=0.01 pitch=0.02 yaw=0.03 az=9.81
...
timeout    ← khi pub không chạy
```

---

## Transport 2: Shared Memory — `CameraFrame`

### Publisher

```cpp
#include "ipc/bus.hpp"
#include "messages/camera_frame.h"

// Truyền payload_size tường minh cho SHM
ipc::Publisher<CameraFrame> pub(CAMERA_FRAME, sizeof(CameraFrame));
if (!pub.valid()) { /* error */ }

CameraFrame msg = {
    .h         = { .topic = CAMERA_FRAME, .seq = seq++, .ts_ns = ... },
    .width     = 640, .height = 480,
    .stride    = 640 * 3,
    .fmt       = 1,           // 0=BGR 1=RGB 2=YUV420
    .data_size = 640 * 3 * 480,
};
pub.send(msg);
```

### Subscriber

```cpp
#include "ipc/bus.hpp"
#include "messages/camera_frame.h"

ipc::Subscriber<CameraFrame> sub(CAMERA_FRAME);
// Nếu pub chưa chạy, constructor block cho đến khi SHM được init

auto msg = sub.poll(2000);
if (msg)
    printf("seq=%u %ux%u fmt=%u\n", msg->h.seq, msg->width, msg->height, msg->fmt);
else
    printf("timeout\n");
```

> **Khác MQ**: SHM không FIFO — sub luôn đọc frame mới nhất, tự skip frame cũ nếu pub nhanh hơn.

### Chạy demo

```bash
# Terminal 1
./Brain/build/bin/cpp_shm_sub_demo

# Terminal 2
./Brain/build/bin/cpp_shm_pub_demo
```

Expected output sub:
```
subscribing CAMERA_FRAME via libipc C++ Subscriber (SHM)...
sub seq=0 640x480 fmt=1 data_size=921600
sub seq=1 640x480 fmt=1 data_size=921600
...
timeout    ← khi pub không chạy
```

---

## Cleanup stale resources

```bash
rm -f /dev/mqueue/imu_state
rm -f /dev/shm/camera_frame
```

---

## API tóm tắt

| Class | Constructor | Method | Mô tả |
|-------|-------------|--------|-------|
| `Publisher<T>` | `(TopicId, size_t payload_size = sizeof(T))` | `send(const T&) → int` | Gửi message |
| `Subscriber<T>` | `(TopicId)` | `poll(int timeout_ms) → optional<T>` | Nhận message |

- Cả hai không copy-constructible — dùng move hoặc pointer nếu cần truyền qua function.
- Transport (MQ/SHM) được chọn tự động theo `TopicDescriptor` trong `topic.h`.

---

## Thêm topic mới

1. Thêm struct vào `Brain/messages/your_msg.h`
2. Thêm `YOUR_TOPIC` vào `TopicId` enum trong `libipc/include/ipc/topic.h`
3. Thêm descriptor trong `libipc/src/topic.c`:
   ```c
   [YOUR_TOPIC] = { "/your_topic", MQ, sizeof(YourMsg), 8, OLD },
   ```
4. Dùng ngay trong C++:
   ```cpp
   ipc::Publisher<YourMsg> pub(YOUR_TOPIC);
   ipc::Subscriber<YourMsg> sub(YOUR_TOPIC);
   ```
5. Rebuild: `cmake --build build`
