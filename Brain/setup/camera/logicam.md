# Hướng dẫn Test Camera (C++ / OpenCV / Ubuntu)

## 1. Kiểm tra camera

```bash
ls /dev/video*
```

Xác định index của camera cần dùng:
```bash
cat /sys/class/video4linux/video0/name
cat /sys/class/video4linux/video2/name
```

> Cái nào **không phải** "HD Webcam" (camera tích hợp) thì là Logitech.  
> Ví dụ: `UVC Camera (046d:xxxx)` → vendor `046d` là Logitech.

---

## 2. Tạo project

```bash
mkdir cam_test && cd cam_test
```

---

## 3. Tạo `main.cpp`

```cpp
#include <opencv2/opencv.hpp>
using namespace cv;

int main() {
    VideoCapture cap(2);  // đổi index cho đúng camera
    if (!cap.isOpened()) {
        printf("Không mở được camera!\n");
        return -1;
    }

    printf("Resolution: %.0fx%.0f @ %.0f fps\n",
        cap.get(CAP_PROP_FRAME_WIDTH),
        cap.get(CAP_PROP_FRAME_HEIGHT),
        cap.get(CAP_PROP_FPS));

    Mat frame;
    while (true) {
        cap >> frame;
        imshow("Camera Test", frame);
        if (waitKey(1) == 'q') break;
    }
    return 0;
}
```

> **Lưu ý:** Sửa `VideoCapture cap(X)` với `X` là index tìm được ở bước 1.

---

## 4. Tạo `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.10)
project(cam_test)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(cam_test main.cpp)
target_link_libraries(cam_test ${OpenCV_LIBS})
```

---

## 5. Cài dependency

```bash
sudo apt install libopencv-dev cmake build-essential
```

---

## 6. Build

```bash
mkdir build && cd build
cmake ..
make
```

---

## 7. Chạy

```bash
./cam_test
```

Nhấn `q` để thoát.