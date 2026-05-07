# Lane Detection Node

## Build & Run

```bash
# từ thư mục Brain/
cmake --build build --target camera_sim_node lane_node

# Terminal 1 — publish video lên SHM
./build/bin/camera_sim_node record/Full_map.mp4

# Terminal 2 — lane detect (thêm --show để xem debug)
./build/bin/lane_node --show
```

---

## Pipeline

```
CAMERA_FRAME (SHM)
    → IPM warp          (preprocessing.cpp)
    → HLS binary mask   (preprocessing.cpp)
    → Sliding window    (lane_detector.cpp)
    → Linear fit        (lane_detector.cpp)
    → EMA smooth        (lane_tracker.cpp)
    → LANE_STATE (MQ)
```

---

## Thông số chỉnh sửa

### 1. IPM trapezoid — `preprocessing.cpp`

Định nghĩa hình thang src trên ảnh gốc để warp thành bird's-eye view.
Tất cả giá trị là tỷ lệ theo chiều rộng (X) hoặc chiều cao (Y) của ảnh.

```
frame gốc:

    TL_X          TR_X
      ●────────────●    ← TOP_Y
     /              \
    ●────────────────●  ← BOT_Y
  BL_X            BR_X
```

| Tham số      | Mô tả                                  | Giá trị hiện tại |
|--------------|----------------------------------------|-----------------|
| `SRC_TL_X`   | Top-left X (fraction of width)         | 0.20            |
| `SRC_TR_X`   | Top-right X (fraction of width)        | 0.80            |
| `SRC_TOP_Y`  | Cạnh trên Y — cắt vùng xa/horizon     | 0.50            |
| `SRC_BL_X`   | Bottom-left X                          | 0.00            |
| `SRC_BR_X`   | Bottom-right X                         | 1.00            |
| `SRC_BOT_Y`  | Cạnh dưới Y — cắt mũi xe              | 0.80            |

**Nguyên tắc chỉnh:**
- Top hẹp hơn bottom → đúng hình học perspective camera nhìn xuống đường
- TOP_Y càng lớn → cắt bỏ nhiều vùng xa hơn (ít distort hơn)
- BOT_Y càng lớn → lấy thêm vùng gần xe
- Khi warped thẳng (lane song song nhau) → heading_err ≈ 0 trên đường thẳng

### 2. HLS threshold — `preprocessing.cpp`

Lọc vạch kẻ đường trắng trên nền nhựa tối.

| Tham số  | Mô tả                    | Giá trị hiện tại |
|----------|--------------------------|-----------------|
| `L_MIN`  | Ngưỡng dưới kênh L (HLS) | 200             |
| `L_MAX`  | Ngưỡng trên kênh L (HLS) | 255             |

**Chỉnh khi:** vạch lane bị mất (tăng `L_MIN` nếu quá nhiễu, giảm nếu không detect được).

### 3. Sliding window — `lane_detector.cpp`

| Tham số      | Mô tả                                      | Giá trị hiện tại |
|--------------|--------------------------------------------|-----------------|
| `N_WINDOWS`  | Số cửa sổ trượt từ dưới lên               | 9               |
| `MARGIN`     | Bán rộng mỗi cửa sổ (pixel)               | 40              |
| `MIN_PIX`    | Số pixel tối thiểu để cập nhật tâm cửa sổ | 30              |

### 4. Pixel-to-meter — `lane_detector.cpp`

| Tham số       | Mô tả                              | Trạng thái         |
|---------------|------------------------------------|--------------------|
| `PIXEL_TO_M`  | Tỷ lệ pixel → mét (lateral offset) | 0.005 — chưa calibrate thực tế |

Cần đo khoảng cách thực tế giữa 2 lane trên xe thật rồi chia cho số pixel tương ứng trong warped image.

### 5. EMA tracker — `lane_tracker.hpp`

| Tham số  | Mô tả                         | Giá trị hiện tại |
|----------|-------------------------------|-----------------|
| `ALPHA`  | Trọng số frame trước (0→1)    | 0.5             |

ALPHA lớn → mượt hơn nhưng chậm phản ứng. ALPHA nhỏ → nhạy hơn nhưng nhiễu hơn.

---

## Output — `LANE_STATE`

| Field               | Ý nghĩa                                                        |
|---------------------|----------------------------------------------------------------|
| `heading_err_rad`   | Góc lệch hướng xe vs tiếp tuyến lane (rad). 0 = thẳng        |
| `lateral_offset_m`  | Lệch ngang xe vs tâm lane (m). + = xe lệch về phải lane, − = trái |

**Ưu tiên lane:** right lane (xanh lá trên debug). Fallback sang left (cam) khi mất right.

---

## Debug `--show`

| Window   | Nội dung                                              |
|----------|-------------------------------------------------------|
| `frame`  | Ảnh gốc + hình thang IPM (viền xanh)                 |
| `warped` | Bird's-eye view + các đường debug + HUD text          |
| `binary` | Ảnh nhị phân sau HLS threshold                        |

Các đường trên `warped`:
- **Xanh lá** (hoặc **cam** nếu fallback left): lane tham chiếu
- **Trắng**: trục dọc xe (giữa ảnh)
- **Vàng**: đường song song lane tham chiếu, đi qua tâm xe
- **Đỏ** (đáy): segment offset ngang lane ↔ xe

Nhấn `ESC` để thoát.
