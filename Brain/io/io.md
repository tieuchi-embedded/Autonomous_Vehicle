# io/

Hardware interface layer — cầu nối giữa phần cứng và IPC bus.

---

## serial_bridge/

Đọc UART từ STM32, parse frame, publish lên topic `SERIAL`.  
Phase 3 sẽ bổ sung downlink (nhận `CONTROL_CMD`, gửi xuống STM32).

### UART protocol (uplink STM32 → Brain)

```
@IMU:yaw,pitch,roll;;\r\n     # angles (degrees)
@RPM:rpm;;\r\n                 # motor RPM
```

### API

#### `protocol.hpp` — parse từng line

```cpp
namespace serial {

struct ImuFrame { float yaw, pitch, roll; };  // degrees
struct RpmFrame { float rpm; };

// Parse @IMU line. Returns nullopt nếu sai format.
std::optional<ImuFrame> parse_imu(std::string_view line);

// Parse @RPM line. Returns nullopt nếu sai format.
std::optional<RpmFrame> parse_rpm(std::string_view line);

// Compute speed (cm/s) từ motor RPM.
// speed = (rpm / 7) * 2π * 6.5 / 60
float rpm_to_speed_cms(float rpm);

}
```

#### `serial_reader.hpp` — UART line reader

```cpp
namespace serial {

class SerialReader {
public:
    SerialReader(const std::string& device, int baud);  // throws on fail
    std::optional<std::string> read_line();             // nullopt = timeout/error
};

}
```

#### `serial_writer.hpp` — downlink (Phase 3, chưa implement)

```cpp
namespace serial {

class SerialWriter {
public:
    explicit SerialWriter(int fd);
    void send_speed(float v);       // #SPED::<v>\r\n
    void send_steer(float angle);   // #STER::<angle>\r\n
    void send_brake(float pct);     // #BRAK::<pct>\r\n
};

}
```

### serial_node

Pub: `SERIAL` (MQ, drop OLD)

```
serial_node [device]
```

- `device`: optional, default `/dev/ttyACM0`
- Mỗi frame nhận được → pub 1 `SerialData` (accumulate — IMU và RPM update field riêng)

---

## Build

```bash
cd Brain
cmake -S . -B build
cmake --build build
```

Binary output: `Brain/build/bin/serial_node`

---

## Chạy

```bash
# Default device /dev/ttyACM0
./build/bin/serial_node

# Chỉ định device
./build/bin/serial_node /dev/ttyUSB0
```

Xem raw output từ STM32 (không qua node):
```bash
stty -F /dev/ttyACM0 115200 raw
cat /dev/ttyACM0
```

Verify data qua IPC:
```bash
# Terminal 1
./build/bin/serial_node /dev/ttyACM0

# Terminal 2
./build/bin/sub_demo   # sub SERIAL, in yaw/pitch/roll/rpm
```
