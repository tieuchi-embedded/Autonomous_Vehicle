# Python Bindings — libipc

Python wrapper cho `libipc` C API, dùng `ctypes`. Cho phép Python node pub/sub qua cùng MQ/SHM với C/C++ nodes.

---

## Cấu trúc

```
Brain/libipc/bindings/python/
├── ipc_schema.py   # ctypes.Structure mapping từ messages/*.h + TopicId constants
└── ipc.py          # Publisher / Subscriber class wrapping C bus API
```

`ipc.py` load `liblibipc.so` (shared lib build cùng source với `liblibipc.a`).

---

## Build

```bash
cd Brain
cmake -S . -B build
cmake --build build
```

Kiểm tra shared lib:
```bash
ls build/libipc/liblibipc.so
```

---

## API

### Publisher

```python
import sys
sys.path.insert(0, "Brain/libipc/bindings/python")

import ipc_schema as schema
from ipc import Publisher

with Publisher(schema.IMU_STATE, payload_size=0) as pub:
    msg = schema.ImuState()
    msg.roll  = 0.1
    msg.pitch = 0.2
    msg.yaw   = 0.3
    msg.az    = 9.81
    pub.publish(msg)   # auto-fills h.topic, h.seq, h.ts_ns
```

`payload_size` chỉ dùng cho SHM topic với pixel data. Với MQ topic (ImuState, WheelOdom, …) truyền `0` hoặc `sizeof` struct.

### Subscriber

```python
with Subscriber(schema.IMU_STATE, schema.ImuState) as sub:
    msg = sub.poll(timeout_ms=1000)
    if msg is None:
        print("timeout")
    else:
        print(f"seq={msg.h.seq} roll={msg.roll:.2f}")
```

`poll()` trả về:
- `ImuState` instance nếu có data
- `None` nếu timeout
- `RuntimeError` nếu transport error

### Context manager + explicit close

```python
# Dùng with (khuyên dùng)
with Publisher(schema.LANE_STATE, 0) as pub:
    ...

# Hoặc manual close
pub = Publisher(schema.LANE_STATE, 0)
pub.publish(msg)
pub.close()
```

---

## Test

### Option A: Python pub ↔ C sub

Terminal 1 — C sub:
```bash
./Brain/build/demo/sub_demo
```

Terminal 2 — Python pub:
```bash
python3 Brain/demo/Python_demo/pub_demo.py
```

Expected output (C sub):
```
subscribing IMU_STATE via libipc...
sub seq=0 roll=0.00 pitch=0.00 yaw=0.00 az=9.81
sub seq=1 roll=0.01 pitch=0.02 yaw=0.03 az=9.81
...
```

### Option B: C pub ↔ Python sub

Terminal 1 — C pub:
```bash
./Brain/build/demo/pub_demo
```

Terminal 2 — Python sub:
```bash
python3 Brain/demo/Python_demo/sub_demo.py
```

---

## Xử lý lỗi load .so

Nếu Python không tìm thấy `liblibipc.so`:
```bash
export LIBIPC_PATH=/absolute/path/to/Brain/build/libipc/liblibipc.so
python3 Brain/demo/Python_demo/sub_demo.py
```

Search order mặc định của `ipc.py`:
1. `$LIBIPC_PATH` (env var)
2. `Brain/build/libipc/liblibipc.so` (relative từ repo root)
3. System loader (`LD_LIBRARY_PATH`)

---

## Cleanup stale resources

```bash
rm -f /dev/mqueue/imu_state /dev/mqueue/wheel_odom
rm -f /dev/shm/camera_frame
```

---

## Struct mapping

| Python class | C struct | Topic |
|---|---|---|
| `schema.ImuState` | `ImuState` | `IMU_STATE` |
| `schema.WheelOdom` | `WheelOdom` | `WHEEL_ODOM` |
| `schema.EgoState` | `EgoState` | `EGO_STATE` |
| `schema.LaneState` | `LaneState` | `LANE_STATE` |
| `schema.ControlCmd` | `ControlCmd` | `CONTROL_CMD` |
| `schema.CameraFrame` | `CameraFrame` | `CAMERA_FRAME` (SHM) |

Source of truth: `Brain/messages/*.h` — nếu C struct thay đổi, cập nhật `ipc_schema.py` cho khớp.
