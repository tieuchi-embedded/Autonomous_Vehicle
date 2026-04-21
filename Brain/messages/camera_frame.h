#pragma once
#include "message.h"

typedef struct {
    MessageHeader h;
    uint32_t width;
    uint32_t height;
    uint32_t stride;     // bytes per row
    uint32_t fmt;        // 0=BGR 1=RGB 2=YUV420
    uint32_t data_size;  // = stride * height
} CameraFrame;

// SHM slot size = sizeof(CameraFrame) + data_size
// Publisher: ipc_publish_open_shm(TOPIC_CAMERA_FRAME, sizeof(CameraFrame) + stride*height)
