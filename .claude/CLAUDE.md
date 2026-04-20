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

## Repository Structure
Cần phân tích và nghiên cứu kỹ thêm
```
BFMC/
├── Brain/                  
│   ├── src/
│   ├── include/
│   └── Makefile
├── Embedded Platform/      
│   └── (STM32CubeIDE project)
├── Simulator/              
├── Documentation/          
└── README.md
```

---

## System Architecture

```
Autonomous Driving/
├── Percieption                  
│   ├── Input
│   │   ├── Camera
│   │   ├── IMU
│   │   ├── Encoder
│   │   └── GPS
│   └── Ouput
│       ├── State
│       └── Location
├── Planning/      
│   ├── Input
│   │   ├── State
│   │   └── Location
│   └── Output
│       ├── Trajectory
│       └── Desired Speed
├── Control/              
│   ├── Input
│   │   ├── Trajectory
│   │   └── Desired Speed
│   └── Output
│       ├── Speed
│       └── Steering Angle
└── Actuator/              
    ├── Input
    │   ├── Speed
    │   └── Steering Angle
    └── Output
        ├── PWM Motor
        └── PWM Servo
            
```
---

## Tech stack
- Languages: C (C11), Python 3.12 (hiện tại chưa dùng), Bash
- Build: CMake 3.26 / Make (cần phân tích thêm)
- Target: ARM Cortex-M4, Linux 6.x (host)
- Tool: STM32CubeIDE, build chương trình trực tiếp trên pi 5 hoặc cross compile (cần phân tích thêm)
- Testing: Có xe thật để test
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

- Always work within this repository; primary runtime target is **Raspberry Pi 5 / Ubuntu 24.04**
- Cross-compilation for STM32 targets ARM Cortex-M (specify exact MCU when relevant)
- IPC design is custom — do not suggest ROS or third-party middleware unless asked
- Python modules are **not yet implemented** — do not scaffold them unless explicitly requested
- Simulator module is **undecided** — present trade-offs when asked, do not pick one unilaterally
- Solo project: avoid over-engineering, no abstractions beyond immediate need
