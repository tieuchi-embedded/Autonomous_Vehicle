# libipc Demo Guide

Hướng dẫn dùng `libipc` để publish/subscribe giữa các process, sử dụng 2 transport: **POSIX MQ** và **Shared Memory**.

---

## Build

```bash
cd Brain
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Binaries ra tại `Brain/build/bin/`.

---

## Transport 1: POSIX MQ — `ImuState`

Dùng cho message nhỏ, FIFO, có depth. Topic `IMU_STATE` dùng MQ với `depth=10, drop=OLD`.

### Publisher

```c
#include "ipc/bus.h"
#include "messages/imu_state.h"

ipc_publisher_t* pub = ipc_publish_open(IMU_STATE, sizeof(ImuState));

ImuState msg = {
    .h     = { .topic = IMU_STATE, .seq = seq++, .ts_ns = ... },
    .roll  = 0.1f, .pitch = 0.2f, .yaw = 0.3f,
    .az    = 9.81f,
};
ipc_publish(pub, &msg, sizeof(msg));

ipc_publish_close(pub);
```

### Subscriber

```c
#include "ipc/bus.h"
#include "messages/imu_state.h"

ipc_subscriber_t* sub = ipc_subscribe_open(IMU_STATE);

ImuState msg;
int r = ipc_poll(sub, &msg, sizeof(msg), 1000); // timeout_ms=1000
// r=0 → có data, r=1 → timeout, r=-1 → error

ipc_subscribe_close(sub);
```

### Chạy demo

```bash
# Terminal 1
./Brain/build/bin/sub_demo

# Terminal 2
./Brain/build/bin/pub_demo
```

Expected output sub:
```
subscribing IMU_STATE via libipc...
sub seq=0 roll=0.00 pitch=0.00 yaw=0.00 az=9.81
sub seq=1 roll=0.01 pitch=0.02 yaw=0.03 az=9.81
...
timeout    ← khi pub không chạy
```

---

## Transport 2: Shared Memory — `CameraFrame`

Dùng cho data lớn, latest-wins (consumer luôn đọc frame mới nhất). Topic `CAMERA_FRAME` dùng SHM với `n_slots=3`.

### Publisher

```c
#include "ipc/bus.h"
#include "messages/camera_frame.h"

// payload_size = sizeof(CameraFrame) + pixel_data nếu cần
// Demo này chỉ gửi header, không gửi pixel buffer
ipc_publisher_t* pub = ipc_publish_open(CAMERA_FRAME, sizeof(CameraFrame));

CameraFrame msg = {
    .h         = { .topic = CAMERA_FRAME, .seq = seq++, .ts_ns = ... },
    .width     = 640,
    .height    = 480,
    .stride    = 640 * 3,
    .fmt       = 1,       // 0=BGR 1=RGB 2=YUV420
    .data_size = 640 * 3 * 480,
};
ipc_publish(pub, &msg, sizeof(msg));

ipc_publish_close(pub);
```

### Subscriber

```c
#include "ipc/bus.h"
#include "messages/camera_frame.h"

ipc_subscriber_t* sub = ipc_subscribe_open(CAMERA_FRAME);
// Nếu pub chưa chạy, sub tự chờ đến khi SHM được init

CameraFrame msg;
int r = ipc_poll(sub, &msg, sizeof(msg), 2000);
// r=0 → frame mới, r=1 → không có frame mới trong timeout_ms

ipc_subscribe_close(sub);
```

> **Khác MQ**: SHM không FIFO — nếu pub ghi nhanh hơn sub đọc, sub tự skip frame cũ, luôn nhận frame mới nhất.

### Chạy demo

```bash
# Terminal 1 (sub tự chờ pub init)
./Brain/build/bin/shm_sub_demo

# Terminal 2
./Brain/build/bin/shm_pub_demo
```

Expected output sub:
```
subscribing CAMERA_FRAME via libipc SHM...
sub seq=0 640x480 fmt=1 data_size=921600
sub seq=1 640x480 fmt=1 data_size=921600
...
timeout    ← khi pub không chạy
```

---

## Cleanup stale resources

Nếu process crash mà không close, MQ/SHM còn tồn tại trên filesystem:

```bash
# Xóa MQ
rm -f /dev/mqueue/imu_state

# Xóa SHM
rm -f /dev/shm/camera_frame
```

---

## API tóm tắt

| Hàm | Mô tả |
|-----|-------|
| `ipc_publish_open(id, size)` | Mở publisher, tạo MQ hoặc SHM tùy topic |
| `ipc_publish(pub, msg, size)` | Gửi 1 message |
| `ipc_publish_close(pub)` | Đóng và giải phóng |
| `ipc_subscribe_open(id)` | Mở subscriber |
| `ipc_poll(sub, buf, size, timeout_ms)` | Nhận message, block đến timeout |
| `ipc_subscribe_close(sub)` | Đóng và giải phóng |

Transport được chọn tự động dựa vào `TopicDescriptor` trong `libipc/include/ipc/topic.h`.

---

## Thêm topic mới

1. Thêm struct vào `Brain/messages/your_msg.h`
2. Thêm `YOUR_TOPIC` vào `TopicId` enum trong `libipc/include/ipc/topic.h`
3. Thêm descriptor vào bảng trong `libipc/src/topic.c`:
   ```c
   [YOUR_TOPIC] = { "/your_topic", MQ, sizeof(YourMsg), 8, OLD },
   ```
4. Rebuild: `cmake --build build`
