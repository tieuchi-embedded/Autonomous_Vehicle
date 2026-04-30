# /test — IPC transport verification

Tự động build và chạy demo để verify libipc transport (MQ hoặc SHM).

## Usage
```
/test mq    — test POSIX MQ transport với ImuState
/test shm   — test SHM latest-wins transport với CameraFrame
/test all   — chạy cả hai
```

## Quy trình

1. **Build**: `cmake --build Brain/build` — báo lỗi nếu fail, dừng lại
2. **Cleanup**: xóa MQ `/imu_state` và SHM `/camera_frame` còn sót từ lần trước
3. **Chạy pub + sub**: mở 2 process song song, sub trước pub sau
4. **Observe 5 giây**: capture output của cả hai
5. **Verify**: kiểm tra sub nhận đúng data từ pub (seq tăng, value khớp)
6. **Cleanup**: kill cả hai process, xóa MQ/SHM

## Tiêu chí pass

- MQ: sub nhận ít nhất 3 message, seq tăng dần, `az=9.81`
- SHM: sub nhận ít nhất 3 frame, slot xoay vòng 0→1→2, `width=640 height=480`
- Không có output `error` hoặc `failed`

## Implement steps

Khi user gọi `/test`, Claude sẽ:

1. Đọc argument (`mq` / `shm` / `all`)
2. Build: `cd Brain && cmake --build build 2>&1`
3. Cleanup stale resources:
   - MQ: `rm -f /dev/mqueue/imu_state`
   - SHM: `rm -f /dev/shm/camera_frame`
4. Chạy sub trong background, chờ 0.5s, chạy pub trong background
5. `sleep 5` để collect output
6. Kill cả hai, phân tích output, báo PASS/FAIL với lý do cụ thể
7. Cleanup resources

## Lưu ý

- Nếu build fail → dừng, không chạy demo
- Nếu sub output toàn `timeout` → pub chưa chạy hoặc MQ/SHM stale → báo rõ
- Sau test xong luôn cleanup để không ảnh hưởng lần chạy sau
